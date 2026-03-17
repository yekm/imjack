#include "testjack.hpp"
#include <cstdio>
#include "imjack_glue.h"

TestJack::TestJack(int i, int j, const char* wav) :
    JackCpp::AudioIO("imjack-jc", 0, 0),
    i0(i), j0(j), wav_prefix(wav)
{
    // Need 2 normal inputs (L/R) + 2 special inputs
    reserveInPorts(4);
    reserveOutPorts(2);

    addOutPort("out-1");
    addOutPort("out-2");
    
    addInPort("in-1");
    addInPort("in-2");
    
    addInPort("special-1");
    addInPort("special-2");
    
    convolvers.resize(i0 * j0, nullptr);
    for (int ii=0; ii<i0; ii++) {
        for (int jj=0; jj<j0; jj++) {
            convolvers[ii*j0 + jj] = JconvolverGlue::create_instance(wav_prefix.c_str(), ii, jj);
        }
    }
    
    if (!convolvers.empty()) {
        current_convolver = convolvers[0];
    }
}

TestJack::~TestJack() {
    for (auto c : convolvers) {
        JconvolverGlue::destroy_instance(c);
    }
}

int TestJack::audioCallback(jack_nframes_t nframes,
        audioBufVector inBufs, audioBufVector outBufs) {
    if (special != -1) {
        // Special mode routing
        for(unsigned int j = 0; j < nframes; j++) {
            outBufs[0][j] = inBufs[2][j];
            outBufs[1][j] = inBufs[3][j];
        }
    }
    else {
        // Convolver routing
        if (current_convolver) {
            const float* in[2] = { inBufs[0], inBufs[1] };
            float* out[2] = { outBufs[0], outBufs[1] };
            JconvolverGlue::process(current_convolver, nframes, in, out);
        } else {
            // Bypass if no convolver
            for(unsigned int j = 0; j < nframes; j++) {
                outBufs[0][j] = inBufs[0][j];
                outBufs[1][j] = inBufs[1][j];
            }
        }
    }
    return 0;
}

bool TestJack::is_special(int s) {
    return special == s;
}

void TestJack::set_special(int s) {
    special = s;
}

bool TestJack::is_current_in(int i, int j) {
    return curi == i && curj == j;
}

void TestJack::set_current_in(int i, int j) {
    curi = i;
    curj = j;
    if (i >= 0 && i < i0 && j >= 0 && j < j0) {
        current_convolver = convolvers[i * j0 + j];
    }
}
