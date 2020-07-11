// file: kernsem.cpp

#include <stdlib.h>
#include "kernsem.h"
#include "locks.h"
#include "schedule.h"

KernelSem::KernelSem(int init): value(init) {
    // no need to lock here, already locked before entering the constructor
    blocked = new KernelSem::List();
}

KernelSem::~KernelSem() {
    // no need to lock here, already locked before entering the destructor
    delete blocked;
}

int KernelSem::wait(Time maxTimeToWait) {
    HARD_LOCK
    if (--value < 0) {
        block(maxTimeToWait);
        HARD_UNLOCK
        return PCB::running->semReleaseStatus;
    }
    HARD_UNLOCK
    return 1;
}

void KernelSem::signal() {
    HARD_LOCK
    if (value + 1 <= 0)
        deblock();
    else
        ++value;
    HARD_UNLOCK
}

void KernelSem::block(Time maxBlockingTime) { // interrupts must be masked during this operation
    PCB::running->state = PCB::BLOCKED;
    blocked->insert(PCB::running, maxBlockingTime > 0);
    if (maxBlockingTime > 0)
        addToWaitList(this, PCB::running, maxBlockingTime);
    dispatch();
}

void KernelSem::deblock() { // interrupts must be masked during this operation
    char temporaryWaited;
    PCB *pcb = blocked->get(&temporaryWaited);
    if (temporaryWaited)
        removeFromWaitList(pcb);
    ++value;
    pcb->state = PCB::READY;
    pcb->semReleaseStatus = 1;
    Scheduler::put(pcb);
}

/*
 * This method should be called only from the updateWaitList() function.
 * NOTE: This is not the most elegant solution, but it is the most efficient,
 * which was the key factor here since updateWaitList() is called from the
 * timer() interrupt routine.
 */
void KernelSem::deblock(PCB *target) { // interrupts must be masked during this operation
    PCB *pcb = blocked->get(target);
    ++value;
    pcb->state = PCB::READY;
    pcb->semReleaseStatus = 0;
    Scheduler::put(pcb);
}


int KernelSem::val() const {
    return value;
}



KernelSem::WaitListNode *KernelSem::waitListHead = NULL;

KernelSem::WaitListNode::WaitListNode(PCB *newPCB, KernelSem *newSem):
    pcb(newPCB), sem(newSem), next(NULL), remainingTime(0)
{

}

void KernelSem::addToWaitList(KernelSem *sem, PCB *pcb, Time timeToWait) {
    // timeToWait will not be 0

    WaitListNode *newNode = new WaitListNode(pcb, sem);
    WaitListNode *cur = waitListHead;
    WaitListNode *prev = NULL;

    while (cur && cur->remainingTime <= timeToWait) {
        timeToWait -= cur->remainingTime;
        prev = cur;
        cur = cur->next;
    }

    newNode->remainingTime = timeToWait;
    newNode->next = cur;
    if (prev)
        prev->next = newNode;
    else
        waitListHead = newNode;

    if (cur)
        cur->remainingTime -= timeToWait;
}

void KernelSem::updateWaitList() {
    if (!waitListHead)
        return;

    WaitListNode *temp;
    --(waitListHead->remainingTime);
    while (waitListHead && waitListHead->remainingTime == 0) {
        temp = waitListHead;
        waitListHead = waitListHead->next;
        temp->sem->deblock(temp->pcb);
        delete temp;
    }

}

void KernelSem::removeFromWaitList(PCB *target) {
    WaitListNode *cur = waitListHead;
    WaitListNode *prev = NULL;
    for (; cur; prev = cur, cur = cur->next)
        if (cur->pcb == target)
            break;
    if (!cur)
        return;
    if (cur->next)
        cur->next->remainingTime += cur->remainingTime;
    if (prev)
        prev->next = cur->next;
    else
        waitListHead = cur->next;
    delete cur;
}


/*
 * KernelSem::List
 */

KernelSem::List::List(): head(NULL), tail(NULL) {

}

KernelSem::List::~List() {
    Node *cur = NULL;
    while (head) {
        cur = head;
        head = head->next;
        delete cur;
    }
}

void KernelSem::List::insert(PCB *pcb, char tempWait) {
    Node *newNode = new Node();
    newNode->pcb = pcb;
    newNode->tempWaiting = tempWait;
    newNode->next = NULL;
    if (head)
        tail->next = newNode;
    else
        head = newNode;
    tail = newNode;
}

PCB *KernelSem::List::get(char *tempWaitFlag) {
    if (!head)
        return NULL;
    Node *first = head;
    head = head->next;
    if (!head)
        tail = NULL;
    PCB *res = first->pcb;
    *tempWaitFlag = first->tempWaiting;
    delete first;
    return res;
}


PCB *KernelSem::List::get(PCB *target) {
    Node *cur = head;
    Node *prev = NULL;
    for (; cur; prev = cur, cur = cur->next)
        if (cur->pcb == target)
            break;
    if (!cur)
        return NULL;
    if (!prev)
        head = cur->next;
    else
        prev->next = cur->next;
    if (!cur->next)
        tail = prev;
    PCB *res = cur->pcb;
    delete cur;
    return res;
}
