// file: pcb.cpp

#include <stdlib.h>
#include <dos.h>
#include "locks.h"
#include "pcb.h"
#include "schedule.h"
#include "timer.h"
#include "semaphor.h"

ID PCB::nextId;

PCB *PCB::running;

PCB *PCB::idle;

PCB::List PCB::globalRegister;

volatile unsigned PCB::globallyUnmaskedSignals = 0xFFFF;

volatile unsigned PCB::globallyUnblockedSignals = 0xFFFF;

Semaphore sleepSemaphore(0);

PCB::PCB():
    id(PCB::nextId++), stack(NULL), sp(0), ss(0), bp(0),
    myThread(NULL), quantum(defaultTimeSlice), state(PCB::READY), semReleaseStatus(0),
    unmaskedSignals(0xFFFF), unblockedSignals(0xFFFF), parentId(0)
{
    // no need to lock because it's already locked before entering the constructor
    blocked = new PCB::List();

    handlers = new SignalHandler[NUMBER_OF_SIGNALS];
    handlers[0] = PCB::abort;
    for (int i = 1; i < NUMBER_OF_SIGNALS; ++i)
        handlers[i] = NULL;
    signalList = new SignalRequestList(handlers);
    PCB::globalRegister.insert(this);
}

PCB::PCB(Thread *thread, StackSize stackSize, Time timeSlice):
    myThread(thread), quantum(timeSlice), id(PCB::nextId++),
    bp(0), state(PCB::NEW), semReleaseStatus(0)
{
    stackSize /= 2; // divide by 2 because each location on stack is 2 bytes

    // no need to lock because it's already locked before entering the constructor
    stack = new unsigned [stackSize];
    stack[stackSize - 1] = CPU_FLAGS_INIT_VALUE;
    stack[stackSize - 2] = FP_SEG(PCB::body);
    stack[stackSize - 3] = FP_OFF(PCB::body);
    // locations stack[stackSize - 4] -- stack[stackSize - 11] saved for CPU registers
    ss = FP_SEG(stack + (stackSize - 12));
    sp = FP_OFF(stack + (stackSize - 12));
    blocked = new PCB::List();

    parentId = PCB::running->id;
    unmaskedSignals = PCB::running->unmaskedSignals;
    unblockedSignals = PCB::running->unblockedSignals;
    handlers = new SignalHandler[NUMBER_OF_SIGNALS];
    for (int i = 0; i < NUMBER_OF_SIGNALS; ++i)
        handlers[i] = PCB::running->handlers[i];
    signalList = new SignalRequestList(handlers);
    PCB::globalRegister.insert(this);
}

PCB::~PCB() {
    // no need to lock because it's already locked before entering the destructor
    PCB::globalRegister.remove(this);
    delete blocked;
    delete signalList;
    delete[] handlers;
    if (stack)
        delete[] stack;
}

ID PCB::getId() const {
    return id;
}

void PCB::start() {
    SOFT_LOCK
    if (state == PCB::NEW) {
        state = PCB::READY;
        Scheduler::put(this);
    }
    SOFT_UNLOCK
}

void PCB::join() {
    if ((state == PCB::TERMINATED) || (this == PCB::idle))
        return;
    SOFT_LOCK
    PCB::running->state = PCB::BLOCKED;
    blocked->insert(PCB::running);
    SOFT_UNLOCK
    dispatch();
}

void PCB::body() {
    PCB::running->myThread->run();

    SOFT_LOCK

    PCB *temp = NULL;
    while ((temp = PCB::running->blocked->get()) != NULL) {
        temp->state = PCB::READY;
        Scheduler::put(temp);
    }

    PCB::running->state = PCB::TERMINATED;

    if (       (PCB::running->unblockedSignals & 0x0004)
            && (PCB::globallyUnblockedSignals & 0x0004)
            && (PCB::running->unmaskedSignals & 0x0004)
            && (PCB::globallyUnmaskedSignals & 0x0004)
        ) {
        if (PCB::running->handlers[2] != NULL) {
            PCB::running->handlers[2]();
        }
    }


    temp = PCB::getPCB(PCB::running->parentId);
    if (temp != NULL)
        temp->signal(1);

    SOFT_UNLOCK
    dispatch();
}

void PCB::sleep(Time sleepingTime) {
    sleepSemaphore.wait(sleepingTime);
}

Thread *PCB::getThreadById(ID targetId) {
    for (List::Node *cur = PCB::globalRegister.head; cur; cur = cur->next)
        if (cur->pcb->id == targetId)
            return cur->pcb->myThread;
    return NULL;
}

PCB *PCB::getPCB(ID targetId) {
    for (List::Node *cur = PCB::globalRegister.head; cur; cur = cur->next)
        if (cur->pcb->id == targetId)
            return cur->pcb;
    return NULL;
}

void dispatch() {
    if (preemptionDisabled)
        return;
    HARD_LOCK
    contextSwitchOnDemand = 1;
    explicitTimerCall = 1;
    timer();
    HARD_UNLOCK
}


/*
 * PCB::List
 */


PCB::List::List(): head(NULL), tail(NULL) {

}

PCB::List::~List() {
    Node *cur = NULL;
    while (head) {
        cur = head;
        head = head->next;
        delete cur;
    }
}

void PCB::List::insert(PCB *pcb) {
    Node *newNode = new Node();
    newNode->pcb = pcb;
    newNode->next = NULL;
    if (head)
        tail->next = newNode;
    else
        head = newNode;
    tail = newNode;
}

void PCB::List::remove(PCB *target) {
    Node *cur = head;
    Node *prev = NULL;
    for (; cur; prev = cur, cur = cur->next)
        if (cur->pcb == target)
            break;
    if (!cur)
        return;
    if (!prev) // if (cur == head)
        head = cur->next;
    else
        prev->next = cur->next;
    if (!cur->next)
        tail = prev;
    delete cur;
}

PCB *PCB::List::get() {
    if (!head)
        return NULL;
    Node *first = head;
    head = head->next;
    if (!head)
        tail = NULL;
    PCB *res = first->pcb;
    delete first;
    return res;
}

/*
 * signal handling
 */

void PCB::signal(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    if (((1U << signal) & PCB::globallyUnmaskedSignals) == 0) {
        SOFT_UNLOCK
        return;
    }
    if (((1U << signal) & unmaskedSignals) == 0) {
        SOFT_UNLOCK
        return;
    }
    if (state == PCB::PAUSED) {
        state = PCB::READY;
        Scheduler::put(this);
    }
    signalList->appendRequest(signal);
    SOFT_UNLOCK
}

void PCB::registerHandler(SignalId signal, SignalHandler handler) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    handlers[signal] = handler;
    SOFT_UNLOCK
}

SignalHandler PCB::getHandler(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return NULL;
    return handlers[signal];
}

void PCB::maskSignal(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    unmaskedSignals &= ~(1U << signal);
    SOFT_UNLOCK
}

void PCB::maskSignalGlobally(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    globallyUnmaskedSignals &= ~(1U << signal);
    SOFT_UNLOCK

}

void PCB::unmaskSignal(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    unmaskedSignals |= (1U << signal);
    SOFT_UNLOCK

}

void PCB::unmaskSignalGlobally(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    globallyUnmaskedSignals |= (1U << signal);
    SOFT_UNLOCK

}

void PCB::blockSignal(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    unblockedSignals &= ~(1U << signal);
    SOFT_UNLOCK

}

void PCB::blockSignalGlobally(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    globallyUnblockedSignals &= ~(1U << signal);
    SOFT_UNLOCK

}

void PCB::unblockSignal(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    unblockedSignals |= (1U << signal);
    SOFT_UNLOCK

}

void PCB::unblockSignalGlobally(SignalId signal) {
    if (!(signal < NUMBER_OF_SIGNALS))
        return;
    SOFT_LOCK
    globallyUnblockedSignals |= (1U << signal);
    SOFT_UNLOCK

}

void PCB::pause() {
    SOFT_LOCK
    PCB::running->state = PCB::PAUSED;
    SOFT_UNLOCK
    dispatch();
}

void PCB::resolveSignalRequests() {
    SOFT_LOCK
    asm sti;

    PCB::running->signalList->processRequests(PCB::globallyUnblockedSignals, PCB::running->unblockedSignals);

    asm cli;
    SOFT_UNLOCK
}


void PCB::abort() {
    PCB *temp = NULL;
    while ((temp = PCB::running->blocked->get()) != NULL) {
        temp->state = PCB::READY;
        Scheduler::put(temp);
    }
    PCB::running->state = PCB::TERMINATED;
    temp = PCB::getPCB(PCB::running->parentId);
    if (temp != NULL)
        temp->signal(1);
    SOFT_UNLOCK // resets preemptionDisabled to 0
    dispatch();
}
