#pragma once

#include "jackaudioio.hpp"
#include <vector>

struct JconvolverInstance;

class TestJack : public JackCpp::AudioIO {
    int m_in = 0;
    const int i0, j0;
    
    // Config properties
    std::string wav_prefix;

    std::vector<JconvolverInstance*> convolvers;
    JconvolverInstance* current_convolver = nullptr;

public:
    TestJack(int i, int j, const char* wav);
    ~TestJack();

    virtual int audioCallback(jack_nframes_t nframes,
            audioBufVector inBufs, audioBufVector outBufs) override;

    bool is_special(int s);
    void set_special(int s);
    bool is_current_in(int i, int j);
    void set_current_in(int i, int j);

    int curi = 0, curj = 0;
    int special = -1;
};
