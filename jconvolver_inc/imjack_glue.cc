#include "imjack_glue.h"
#include "zita-convolver.h"
#include "audiofile.h"
#include <stdio.h>
#include <string.h>

#define BSIZE  0x4000

struct JconvolverInstance {
    Convproc* conv;
    int ninp;
    int nout;
};

JconvolverInstance* JconvolverGlue::create_instance(const char* wav_path_prefix, int row, int col) {
    // We are no longer using config parser.
    // Instead we configure Convproc manually as requested:
    // /convolver/new 2 2 1024 65536
    // /input/name 1 in-l
    // /input/name 2 in-r
    // /output/name 1 out-l
    // /output/name 2 out-r
    // /impulse/read 1 1 1 0 0 0 1 filter/filter-center1-soft-0-0-l.wav
    // /impulse/read 2 2 1 0 0 0 1 filter/filter-center1-soft-0-0-r.wav

    unsigned int ninp = 2;
    unsigned int nout = 2;
    unsigned int part = 1024; // fragment size
    unsigned int size = 65536; // max convolution length
    float dens = 0.0f;
    unsigned int fragm = 1024;
    // fsamp is not actively passed to configure, zita handles rate via data size
    
    Convproc* conv = new Convproc;
    if (conv->configure(ninp, nout, size, fragm, part, Convproc::MAXPART, dens)) {
        fprintf(stderr, "Failed to configure convolver %d:%d\n", row, col);
        delete conv;
        return nullptr;
    }

    // Helper lambda to load and assign wav file to convolver instance
    auto load_wav = [&](int ip1, int op1, const char* suffix) -> bool {
        char path[1024];
        snprintf(path, sizeof(path), "%s-%d-%d-%s", wav_path_prefix, row, col, suffix);

        Audiofile audio;
        if (audio.open_read(path)) {
            fprintf(stderr, "Unable to open '%s'\n", path);
            return false;
        }

        int nchan = audio.chan();
        int nfram = audio.size();
        
        int ichan = 1; // Always take first channel of the file (from original config: read X X X X X X 1 ...)
        if (nchan < 1) {
            audio.close();
            return false;
        }

        float gain = 1.0f;
        unsigned int delay = 0;
        unsigned int length = nfram;
        if (length > size) length = size;

        float* buff;
        try {
            buff = new float[BSIZE * nchan];
        } catch (...) {
            audio.close();
            return false;
        }

        while (length > 0) {
            int to_read = (length > BSIZE) ? BSIZE : length;
            int read_frames = audio.read(buff, to_read);
            if (read_frames <= 0) break; // EOF or Error

            float* p = buff + (ichan - 1);
            for (int i = 0; i < read_frames; i++) {
                p[i * nchan] *= gain;
            }

            if (conv->impdata_create(ip1 - 1, op1 - 1, nchan, p, delay, delay + read_frames)) {
                audio.close();
                delete[] buff;
                return false;
            }
            delay += read_frames;
            length -= read_frames;
        }
        
        audio.close();
        delete[] buff;
        printf("Loaded WAV: %s\n", path);
        return true;
    };

    bool success_l = load_wav(1, 1, "l.wav");
    bool success_r = load_wav(2, 2, "r.wav");

    if (!success_l || !success_r) {
        fprintf(stderr, "Warning: failed to load one or more wav files for %d:%d\n", row, col);
    }

    JconvolverInstance* instance = new JconvolverInstance;
    instance->conv = conv;
    instance->ninp = ninp;
    instance->nout = nout;
    
    // Check if we need to call configure/start_process here
    instance->conv->start_process(0, 0);
    
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
