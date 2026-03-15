#pragma once

#include "jackaudioio.hpp"

class TestJack: public JackCpp::AudioIO {
    int m_in = 0;
	public:
		// Your audio callback. All audio processing goes in this function.
		virtual int audioCallback(jack_nframes_t nframes, 
				// A vector of pointers to each input port.
				audioBufVector inBufs,
				// A vector of pointers to each output port.
				audioBufVector outBufs);
		void set_curent_in(int n);
		TestJack(int num_ports);
};