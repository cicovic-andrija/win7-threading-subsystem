// Operating Systems 1 project
// Author: Andrija Cicovic 0007/2015
// Compiler: Borland C++ Version 3.1

// file: main.cpp

#include "locks.h"
#include "pcb.h"

// User application thread, running the userMain(int, char *[]) function
int userMain(int argc, char *argv[]);
class UserApplication: public Thread {
public:
    UserApplication(int argCounter, char *argVector[],
                    StackSize stackSize = defaultStackSize,
                    Time timeSlice = defaultTimeSlice);
    ~UserApplication();
    int getReturnValue();
protected:
    void run();
private:
    int argc;
    char **argv;
    int returnValue;
};

UserApplication::UserApplication(int argCounter, char *argVector[],
                                 StackSize stackSize, Time timeSlice):
    Thread(stackSize, timeSlice), argc(argCounter), argv(argVector),
    returnValue(0)
{

}

UserApplication::~UserApplication() {
    waitToComplete();
}

int UserApplication::getReturnValue() {
    return returnValue;
}

void UserApplication::run() {
    returnValue = userMain(argc, argv);
}

// Idle thread, for when no other thread is running
class IdleThread: public Thread {
public:
    IdleThread() : Thread(512, 1) { }
protected:
    void run();
};

void IdleThread::run() {
    while (1)
        ;
}

// "Kernel" thread
#include <dos.h>
#include "timer.h"
static void interrupt (*oldTimerInterrupt)(...);

void system_init() {
    HARD_LOCK
    PCB::running = new PCB();
    oldTimerInterrupt = getvect(0x8);
    setvect(0x8, timer);
    setvect(0x60, oldTimerInterrupt);
    HARD_UNLOCK
}

void system_restore() {
    HARD_LOCK
    delete PCB::running;
    setvect(0x8, oldTimerInterrupt);
    HARD_UNLOCK
}

int main(int argc, char *argv[]) {
    system_init();
    UserApplication *userApp = new UserApplication(argc, argv);
    IdleThread *idleThread = new IdleThread();
    PCB::idle = idleThread->myPCB;

    userApp->start();
    userApp->waitToComplete();
    int userAppReturnValue = userApp->getReturnValue();

    SOFT_LOCK
    delete userApp;
    delete idleThread;
    SOFT_UNLOCK
    system_restore();
    return userAppReturnValue;
}
