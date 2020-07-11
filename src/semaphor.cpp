// file: semaphor.cpp

#include "semaphor.h"
#include "kernsem.h"
#include "locks.h"

Semaphore::Semaphore(int init) {
    SOFT_LOCK
    myImpl = new KernelSem(init);
    SOFT_UNLOCK
}

Semaphore::~Semaphore() {
    SOFT_LOCK
    delete myImpl;
    SOFT_UNLOCK
}

/*
 * myImpl == 0 should never happen
 */

int Semaphore::wait(Time maxTimeToWait) {
    return myImpl->wait(maxTimeToWait);
}

void Semaphore::signal() {
    myImpl->signal();
}

int Semaphore::val() const {
    return myImpl->val();
}
