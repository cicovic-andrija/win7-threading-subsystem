// file: thread.cpp

#include "thread.h"
#include "pcb.h"
#include "locks.h"

Thread::Thread(StackSize stackSize, Time timeSlice) {
    if (stackSize > maxStackSize)
        stackSize = maxStackSize;
    SOFT_LOCK
    myPCB = new PCB(this, stackSize, timeSlice);
    SOFT_UNLOCK
}

Thread::~Thread() {
    waitToComplete();
    SOFT_LOCK
    delete myPCB;
    SOFT_UNLOCK
}

/*
 * myPCB == NULL should never happen
 */

ID Thread::getId() {
    return myPCB->getId();
}

void Thread::start() {
    myPCB->start();
}

void Thread::waitToComplete() {
    myPCB->join();
}

ID Thread::getRunningId() {
    return PCB::running->getId();
}

void Thread::sleep(Time sleepingTime) {
    PCB::sleep(sleepingTime);
}

Thread *Thread::getThreadById(ID targetId) {
    return PCB::getThreadById(targetId);
}


/*
 * signal handling
 */


void Thread::signal(SignalId signal) {
    myPCB->signal(signal);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler) {
    myPCB->registerHandler(signal, handler);
}

SignalHandler Thread::getHandler(SignalId signal) {
    return myPCB->getHandler(signal);
}

void Thread::maskSignal(SignalId signal) {
    myPCB->maskSignal(signal);
}

void Thread::maskSignalGlobally(SignalId signal) {
    PCB::maskSignalGlobally(signal);
}

void Thread::unmaskSignal(SignalId signal) {
    myPCB->unmaskSignal(signal);
}

void Thread::unmaskSignalGlobally(SignalId signal) {
    PCB::unmaskSignalGlobally(signal);
}

void Thread::blockSignal(SignalId signal) {
    myPCB->blockSignal(signal);
}

void Thread::blockSignalGlobally(SignalId signal) {
    PCB::blockSignalGlobally(signal);
}

void Thread::unblockSignal(SignalId signal) {
    myPCB->unblockSignal(signal);
}

void Thread::unblockSignalGlobally(SignalId signal) {
    PCB::unblockSignalGlobally(signal);
}

void Thread::pause() {
    PCB::pause();
}
