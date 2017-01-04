#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "x86_desc.h"
#include "pcb.h"

void schedule();
void switch_to_term1();
void switch_to_term2();
void switch_to_term3();

#endif