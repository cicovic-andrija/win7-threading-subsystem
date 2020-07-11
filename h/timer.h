// file: timer.h

#ifndef _TIMER_H_
#define _TIMER_H_

void interrupt timer(...);
extern volatile unsigned preemptionDisabled;
extern volatile char contextSwitchOnDemand;
extern volatile char explicitTimerCall;

#endif // _TIMER_H_
