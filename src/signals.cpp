// file: signals.cpp

#include <stdlib.h>
#include "signals.h"

SignalRequestList::SignalRequestList(SignalHandler *handlers):
    head(NULL), tail(NULL), handlerRoutines(handlers)
{

}

SignalRequestList::~SignalRequestList() {
    SignalRequest *cur = NULL;
    while (head) {
        cur = head;
        head = head->next;
        delete cur;
    }
}

SignalRequestList::SignalRequest::SignalRequest(SignalId id):
    next(NULL), signalId(id)
{

}

void SignalRequestList::appendRequest(SignalId signalId) {
    SignalRequest *newRequest = new SignalRequest(signalId);
    if (head)
        tail->next = newRequest;
    else
        head = newRequest;
    tail = newRequest;
}

void SignalRequestList::processRequests(unsigned globalMask, unsigned mask) {

    SignalRequest *cur = head;
    SignalRequest *prev = NULL;

    while (cur) {
        if (((1U << cur->signalId) & globalMask) && ((1U << cur->signalId) & mask)) {
            if (handlerRoutines[cur->signalId] != NULL) {
                handlerRoutines[cur->signalId]();
            }
            if (cur == tail) // cur->next == NULL
                tail = prev;
            if (prev == NULL) { // cur == head
                head = head->next;
                delete cur;
                cur = head;
            }
            else {
                prev->next = cur->next;
                delete cur;
                cur = prev->next;
            }

        }
        else {
            prev = cur;
            cur = cur->next;
        }
    }
}
