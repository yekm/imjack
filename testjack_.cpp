#include "testjack.hpp"

class TestJack: public JackCpp::AudioIO {
    int m_in = 0;
	public:
		// Your audio callback. All audio processing goes in this function.
		virtual int audioCallback(jack_nframes_t nframes, 
				// A vector of pointers to each input port.
				audioBufVector inBufs,
				// A vector of pointers to each output port.
				audioBufVector outBufs){
			//for(unsigned int i = 0; i < MIN(inBufs.size(), outBufs.size()); i++){
			for(unsigned int j = 0; j < nframes; j++) {
				outBufs[0][j] = inBufs[m_in*2][j];	// A simple example: copy the input to the output.
				outBufs[1][j] = inBufs[m_in*2+1][j];	// A simple example: copy the input to the output.
			}
			//return 0 on success
			return 0;
		}
		void set_curent_in(int n) {
		    m_in = n;
		}
		TestJack(int num_ports) :
			JackCpp::AudioIO("jackcpp-test", 2,2){
				//we have 16 total input and output ports that we could have
				reserveInPorts(num_ports);
				reserveOutPorts(2);

				addOutPort("out_1");
				addOutPort("out_2");
				for (int i=0; i<num_ports; i++) {
				    char buf[32];
					sprintf(buf, "in-%d-1", i);
                    addOutPort(buf);		// add new out port (3) named "blahout1"
                    sprintf(buf, "in-%d-2", i);
                    addInPort(buf);		// add new out port (3) named "blahout1"
				}
		}
};