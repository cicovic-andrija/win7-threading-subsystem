// file: event.h

#ifndef _EVENT_H_
#define _EVENT_H_

typedef unsigned char IVTNo;
class KernelEv;
class Event;

#define NUMBER_OF_ENTRIES 256

class IVTEntry {
public:
    IVTEntry(IVTNo num, void interrupt (*newRoutine)(...));
    ~IVTEntry();

    static IVTEntry *allEntries[NUMBER_OF_ENTRIES];

    static IVTEntry *getEntry(IVTNo number);
    static void sendSignal(IVTNo number);
    static void callOldInterruptRoutine(IVTNo number);
    static void registerEvent(IVTNo number, Event *event);

private:
    IVTNo entryNumber;
    void interrupt (*oldRoutine)(...);
    Event *event;

};

// PREPAREENTRY macro:
// flag - should the old interrupt routine be called
// num - valid entry number assumed
#define PREPAREENTRY(num, flag) \
void interrupt interruptRoutine##num(...) {\
    if (flag)\
        IVTEntry::callOldInterruptRoutine(num);\
    IVTEntry::sendSignal(num);\
}\
IVTEntry entry##num(num, interruptRoutine##num);
// end

class Event {
public:
    Event(IVTNo ivtNo);
    ~Event();
    void wait();

protected:
    friend void IVTEntry::sendSignal(IVTNo number);
    friend class KernelEv;
    void signal();

private:
    KernelEv *myImpl;

};

#endif  // _EVENT_H_
