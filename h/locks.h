// file: locks.h

#ifndef _LOCKS_H_
#define _LOCKS_H_

#include "timer.h"

#define HARD_LOCK asm pushf;\
             asm cli;

#define HARD_UNLOCK asm popf;

#define SOFT_LOCK ++preemptionDisabled;

#define SOFT_UNLOCK --preemptionDisabled;\
                    if (!preemptionDisabled && contextSwitchOnDemand)\
                        dispatch();\
                    else ;

#endif // _LOCKS_H_
