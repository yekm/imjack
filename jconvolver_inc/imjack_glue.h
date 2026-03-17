#pragma once

// Forward declarations to avoid exposing jconvolver internals to TestJack
struct JconvolverInstance;

class JconvolverGlue {
public:
    static JconvolverInstance* create_instance(const char* wav_path_prefix, int row, int col);
    static void destroy_instance(JconvolverInstance* instance);
    
    // Process audio: in and out are arrays of pointers to float buffers
    static void process(JconvolverInstance* instance, int nframes, const float* const* in, float** out);
};
