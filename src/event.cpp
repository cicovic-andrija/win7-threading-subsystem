// file: event.cpp

#include <stdlib.h>
#include <dos.h>
#include "event.h"
#include "kernev.h"
#include "locks.h"
#include "pcb.h"

Event::Event(IVTNo ivtNo) {
    SOFT_LOCK
    myImpl = new KernelEv();
    IVTEntry::registerEvent(ivtNo, this);
    SOFT_UNLOCK
}

Event::~Event() {
    SOFT_LOCK
    delete myImpl;
    SOFT_UNLOCK
}

/*
 * myImpl == 0 should never happen
 */

void Event::wait() {
    myImpl->wait();
}

void Event::signal() {
    myImpl->signal();
}


// IVTEntry class - definitions

IVTEntry *IVTEntry::allEntries[NUMBER_OF_ENTRIES];

IVTEntry::IVTEntry(IVTNo num, void interrupt (*newRoutine)(...)):
    entryNumber(num), event(NULL)
{
    HARD_LOCK
    oldRoutine = getvect(num);
    setvect(num, newRoutine);
    IVTEntry::allEntries[num] = this;
    HARD_UNLOCK
}

IVTEntry::~IVTEntry() {
    HARD_LOCK
    setvect(entryNumber, oldRoutine);
    HARD_UNLOCK
}

IVTEntry *IVTEntry::getEntry(IVTNo number) {
    return IVTEntry::allEntries[number];
}

void IVTEntry::sendSignal(IVTNo number) {
    if (IVTEntry::allEntries[number]->event != NULL)
        IVTEntry::allEntries[number]->event->signal();
}

void IVTEntry::callOldInterruptRoutine(IVTNo number) {
    IVTEntry::allEntries[number]->oldRoutine();
}

void IVTEntry::registerEvent(IVTNo number, Event *event) {
    IVTEntry::allEntries[number]->event = event;
}

