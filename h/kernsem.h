// file: kernsem.h

#ifndef _KERNSEM_H_
#define _KERNSEM_H_

#include "semaphor.h"
#include "pcb.h"

class PCB;

class KernelSem {
public:
    // type declarations

    /*
     * this class is NOT thread-safe - use it in thread-safe sections
     */
    class List {
    public:
        List();
        ~List();
        void insert(PCB *pcb, char tempWait);
        PCB *get(char *tempWaitFlag);
        PCB *get(PCB *target);

    private:
        friend class KernelSem;
        struct Node {
            char tempWaiting;
            PCB *pcb;
            Node *next;
        };
        Node *head;
        Node *tail;
    };

    // methods
    KernelSem(int init = 1);
    ~KernelSem();
    int wait(Time maxTimeToWait);
    void signal();
    int val() const;

private:
    friend void interrupt timer(...);

    static void addToWaitList(KernelSem *sem, PCB *pcb, Time timeToWait);
    static void updateWaitList();
    static void removeFromWaitList(PCB *target);
    void block(Time maxBlockingTime);
    void deblock();
    void deblock(PCB *target);

    struct WaitListNode {
        WaitListNode(PCB *pcb, KernelSem *sem);
        PCB *pcb;
        KernelSem *sem;
        Time remainingTime;
        WaitListNode *next;
    };

    // static members
    static WaitListNode *waitListHead;

    // members
    KernelSem::List *blocked;
    volatile int value;
};

#endif // _KERNSEM_H_
