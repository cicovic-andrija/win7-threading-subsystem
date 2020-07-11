// file: kernev.cpp

#include "kernev.h"
#include "locks.h"
#include "schedule.h"
#include "pcb.h"

KernelEv::KernelEv() {
    owner = PCB::running;
    value = 0;
}

void KernelEv::wait() {
    if (PCB::running != owner) {
        return;
    }

    HARD_LOCK
    if (--value < 0) {
        PCB::running->state = PCB::BLOCKED;
        dispatch();
        HARD_UNLOCK
        return;
    }
    HARD_UNLOCK
}

void KernelEv::signal() {
    HARD_LOCK // not necessary if signal is only called from interrupt routine
    if (value < 0) {
        value = 0;
        owner->state = PCB::READY;
        Scheduler::put(owner);
    }
    else {
        value = 1;
    }
    HARD_UNLOCK // not necessary if signal is only called from interrupt routine
    dispatch();
}


