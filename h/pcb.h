// file: pcb.h

#ifndef _PCB_H_
#define _PCB_H_

#include "thread.h"
#include "signals.h"
#define CPU_FLAGS_INIT_VALUE 0x0200

class PCB {
public:
    // type definitions
    enum State { NEW, READY, BLOCKED, PAUSED, TERMINATED };

    /*
     * this class is NOT thread-safe - use it in thread-safe sections
     */
    class List {
    public:
        List();
        ~List();
        void insert(PCB *pcb);
        void remove(PCB *target);
        PCB *get();
    private:
        friend class PCB;
        struct Node {
            PCB  *pcb;
            Node *next;
        };
        Node *head;
        Node *tail;
    };

    // static members
    static PCB *running;
    static PCB *idle;
    static List globalRegister;
    static Thread *getThreadById(ID targetId);
    static PCB *getPCB(ID targetId);
    static void sleep(Time sleepingTime);

    // methods
    PCB();
    PCB(Thread *thread, StackSize stackSize, Time timeSlice);
    ~PCB();
    ID getId() const;
    void start();
    void join();

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
    static void resolveSignalRequests();
    static void pause();
    static void abort();

private:
    // friend declarations
    friend int main(int, char*[]);
    friend void interrupt timer(...);
    friend class KernelSem;
    friend class KernelEv;

    static ID nextId;
    static void body();

    ID id;                                    // identification number
    volatile State state;                     // PCB's current state
    Thread *myThread;                         // this PCB's thread
    ID parentId;                              // parent's identification number
    Time quantum;                             // time slice
    unsigned *stack;                          // pointer to allocated stack
    unsigned ss;                              // stack segment
    unsigned sp;                              // stack pointer
    unsigned bp;                              // base pointer
    char semReleaseStatus;                    // released from semaphore via signal() call?
    List *blocked;                            // list of PCB's waiting for this one to finish

    SignalRequestList *signalList;                     // list of signal requests
    SignalHandler *handlers;                           // vector of signal handlers
    volatile unsigned unmaskedSignals;                 // bit-vector: masked signals
    volatile unsigned unblockedSignals;                // bit-vector: blocked signals
    static volatile unsigned globallyUnmaskedSignals;  // bit-vector: globally masked signals
    static volatile unsigned globallyUnblockedSignals; // bit-vector: globally blocked signals

};

#endif // _PCB_H_
