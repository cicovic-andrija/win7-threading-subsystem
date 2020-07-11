// file: thread.h

#ifndef _THREAD_H_
#define _THREAD_H_

typedef unsigned long StackSize;
typedef unsigned int Time;
typedef int ID;
typedef void (*SignalHandler)();
typedef unsigned SignalId;
class PCB;

const StackSize defaultStackSize = 4096; // 4096B
const StackSize maxStackSize = 1UL << 16;  // 2^(16)B = 64KB
const Time defaultTimeSlice = 2;

class Thread {
public:
    virtual ~Thread();
    void start();
    void waitToComplete();
    ID getId();

    static ID getRunningId();
    static Thread *getThreadById(ID id);
    static void sleep(Time sleepingTime);

    // methods for signal handling
    void signal(SignalId signal);
    void registerHandler(SignalId signal, SignalHandler handler);
    SignalHandler getHandler(SignalId signal);
    void maskSignal(SignalId signal);
    static void maskSignalGlobally(SignalId signal);
    void unmaskSignal(SignalId signal);
    static void unmaskSignalGlobally(SignalId signal);
    void blockSignal(SignalId signal);
    static void blockSignalGlobally(SignalId signal);
    void unblockSignal(SignalId signal);
    static void unblockSignalGlobally(SignalId signal);
    static void pause();

protected:
    friend class PCB;
    Thread(StackSize stackSize = defaultStackSize, Time timeSlice = defaultTimeSlice);
    virtual void run() { }

private:
    friend int main(int, char*[]);

    PCB *myPCB;

};

void dispatch();

#endif // _THREAD_H_
