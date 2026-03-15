#include "testjack.hpp"
#include <cstdio>

TestJack::TestJack(int i, int j) :
    JackCpp::AudioIO("imjack-test", 0, 0),
    i0(i), j0(j)
{
    reserveInPorts(i0*j0*2+2);
    reserveOutPorts(2);

    addOutPort("out-1");
    addOutPort("out-2");
    for (int ii=0; ii<i0; ii++) {
        for (int jj=0; jj<j0; jj++) {
            char buf[32];
            sprintf(buf, "in-%d-%d-l", ii, jj);
            addInPort(buf);
            sprintf(buf, "in-%d-%d-r", ii, jj);
            addInPort(buf);
        }
    }
    // nb. left right naming
    addInPort("special-0-1");
    addInPort("special-0-2");
}

int TestJack::audioCallback(jack_nframes_t nframes,
        audioBufVector inBufs, audioBufVector outBufs) {
    for(unsigned int j = 0; j < nframes; j++) {
        if (special != -1) {
            int last = i0*j0*2;
            outBufs[0][j] = inBufs[last+special+0][j];
            outBufs[1][j] = inBufs[last+special+1][j];
        }
        else {
            int cur = curi*i0 + curj;
            outBufs[0][j] = inBufs[cur*2][j];
            outBufs[1][j] = inBufs[cur*2+1][j];
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
}
