// file: kernev.h

#ifndef _KERNEV_H_
#define _KERNEV_H_

#include "event.h"

class PCB;

class KernelEv {
public:
    KernelEv();
    void wait();
    void signal();

private:
    PCB *owner;
    volatile char value;

};

#endif // _KERNEV_H_
