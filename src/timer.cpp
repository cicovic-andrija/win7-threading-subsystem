// file: timer.cpp

#include <iostream.h>
#include "pcb.h"
#include "schedule.h"
#include "locks.h"
#include "kernsem.h"
void tick();

volatile char contextSwitchOnDemand;
volatile char explicitTimerCall;
volatile unsigned preemptionDisabled;

static volatile int timeCounter = defaultTimeSlice;
static volatile unsigned runIndefinitelly;
static volatile unsigned tsp;
static volatile unsigned tss;
static volatile unsigned tbp;

void interrupt timer(...) {

    if (!explicitTimerCall) {
        asm int 60h;  // call old timer interrupt routine
        tick();
        // asm cli;
        --timeCounter;
        KernelSem::updateWaitList();
    }
    explicitTimerCall = 0;

    if ((!runIndefinitelly && timeCounter == 0) || contextSwitchOnDemand) {

        if (!preemptionDisabled) {
            contextSwitchOnDemand = 0; // reset only here

            asm {
                mov tsp, sp
                mov tss, ss
                mov tbp, bp
            }

            PCB::running->sp = tsp;
            PCB::running->ss = tss;
            PCB::running->bp = tbp;

            if ((PCB::running != PCB::idle) && (PCB::running->state == PCB::READY))
                    Scheduler::put(PCB::running);

            if ((PCB::running = Scheduler::get()) == 0)
                PCB::running = PCB::idle;

            tsp = PCB::running->sp;
            tss = PCB::running->ss;
            tbp = PCB::running->bp;

            timeCounter = PCB::running->quantum;
            runIndefinitelly = !(timeCounter > 0);

            asm {
                mov sp, tsp
                mov ss, tss
                mov bp, tbp
            }

            PCB::resolveSignalRequests();

        }
        else {
            contextSwitchOnDemand = 1;
        }

    }
}
