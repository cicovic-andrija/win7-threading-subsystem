// file: signals.h

#ifndef _SIGNALS_H_
#define _SIGNALS_H_

#define NUMBER_OF_SIGNALS 16
typedef unsigned SignalId;
typedef void (*SignalHandler)();

/*
 * this class is NOT thread-safe - use it in thread-safe sections
 */

class SignalRequestList {
public:
    SignalRequestList(SignalHandler *handlers);
    ~SignalRequestList();
    void appendRequest(SignalId id);
    void processRequests(unsigned globalMask, unsigned mask);

private:
    struct SignalRequest {
        SignalRequest(SignalId id);
        SignalId signalId;
        SignalRequest *next;
    };

    SignalRequest *head;
    SignalRequest *tail;
    SignalHandler *handlerRoutines;

};

#endif // _SIGNALS_H_
