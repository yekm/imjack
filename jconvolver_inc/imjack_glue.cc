#include "imjack_glue.h"
#include "config.h"
#include "zita-convolver.h"
#include <stdio.h>
#include <string.h>

// Globals defined in config.cc
extern const char* g_wav_path_prefix;
extern int g_row;
extern int g_col;

// Globals needed by config.cc
Convproc *convproc = 0;
char jackname [NAMELEN] = "imjack";
char jackserv [NAMELEN] = "";
unsigned int latency = 0;
unsigned int options = 0;
unsigned int fsamp = 48000;
unsigned int fragm = 1024;
unsigned int ninp = 0;
unsigned int nout = 0;
unsigned int size = 0;

// Dummy Jclient for config parser
class Jclient {
public:
    Jclient(const char*, const char*, Convproc*) {}
    unsigned int fsamp() const { return ::fsamp; }
    unsigned int fragm() const { return ::fragm; }
    void add_input_port(const char*, const char* = 0) {}
    void add_output_port(const char*, const char* = 0) {}
};
Jclient *jclient = 0;

// Need to implement convnew which config.cc calls when it hits /convolver/new
// We intercept this to setup Convproc but avoid jclient start
static char *inp_name [Convproc::MAXINP];
static char *out_name [Convproc::MAXOUT];
static char *inp_conn [Convproc::MAXINP];
static char *out_conn [Convproc::MAXOUT];

int convnew (const char *line, int lnum)
{
    unsigned int part;
    float        dens;
    int          r;

    convproc = new Convproc;
    jclient = new Jclient (jackname, jackserv, convproc);
    fsamp = jclient->fsamp ();
    fragm = jclient->fragm ();

    memset (inp_name, 0, Convproc::MAXINP * sizeof (char *));
    memset (inp_conn, 0, Convproc::MAXINP * sizeof (char *));
    memset (out_name, 0, Convproc::MAXOUT * sizeof (char *));
    memset (out_conn, 0, Convproc::MAXOUT * sizeof (char *));

    r = sscanf (line, "%u %u %u %u %f", &ninp, &nout, &part, &size, &dens);
    if (r < 4) return ERR_PARAM;
    if (r < 5) dens = 0;

    if ((ninp == 0) || (ninp > Convproc::MAXINP)) return ERR_OTHER;
    if ((nout == 0) || (nout > Convproc::MAXOUT)) return ERR_OTHER;
    if  ((part & (part -1)) || (part < Convproc::MINPART) || (part > Convproc::MAXPART)) return ERR_OTHER;
    
    if (part > Convproc::MAXDIVIS * fragm) part = Convproc::MAXDIVIS * fragm; 
    if (part < fragm) part = fragm; 
    if (size > MAXSIZE) return ERR_OTHER;
    if ((dens < 0.0f) || (dens > 1.0f)) return ERR_OTHER;

    convproc->set_options (options);
    if (convproc->configure (ninp, nout, size, fragm, part, Convproc::MAXPART, dens)) return ERR_OTHER;

    return 0;
}

int inpname (const char *line) { return 0; }
int outname (const char *line) { return 0; }
void makeports (void) {}

struct JconvolverInstance {
    Convproc* conv;
    int ninp;
    int nout;
};

JconvolverInstance* JconvolverGlue::create_instance(const char* config_path, const char* wav_path_prefix, int row, int col) {
    g_wav_path_prefix = wav_path_prefix;
    g_row = row;
    g_col = col;
    
    // Set some defaults just in case
    convproc = nullptr;
    fsamp = 48000;
    fragm = 1024;
    
    if (config(config_path)) {
        fprintf(stderr, "Failed to load config %s\n", config_path);
        if (convproc) {
            delete convproc;
            convproc = nullptr;
        }
        return nullptr;
    }
    
    if (!convproc) return nullptr;
    
    JconvolverInstance* instance = new JconvolverInstance;
    instance->conv = convproc;
    instance->ninp = ninp;
    instance->nout = nout;
    
    // We don't start it immediately, TestJack might need to handle threading/priority
    instance->conv->start_process(0, 0); // Need to check how jack cpp manages thread prio
    
    return instance;
}

void JconvolverGlue::destroy_instance(JconvolverInstance* instance) {
    if (instance) {
        if (instance->conv) {
            instance->conv->stop_process();
            instance->conv->cleanup();
            delete instance->conv;
        }
        delete instance;
    }
}

void JconvolverGlue::process(JconvolverInstance* instance, int nframes, const float* const* in, float** out) {
    if (!instance || !instance->conv) return;
    
    // Fill inputs
    for (int i = 0; i < instance->ninp; ++i) {
        float* q = instance->conv->inpdata(i);
        const float* p = in[i];
        for (int j = 0; j < nframes; ++j) {
            q[j] = p[j];
        }
    }
    
    instance->conv->process();
    
    // Extract outputs
    for (int i = 0; i < instance->nout; ++i) {
        const float* p = instance->conv->outdata(i);
        float* q = out[i];
        for (int j = 0; j < nframes; ++j) {
            q[j] = p[j];
        }
    }
}
