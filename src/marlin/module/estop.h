/*
 * estop.h
 *
 * Created: 28/06/2021 9:46:17 am
 *  Author: smohekey
 */ 


#ifndef ESTOP_H_
#define ESTOP_H_

#include "../inc/MarlinConfig.h"
#include <stdint.h>

#if HAS_ESTOP

inline bool estop_engaged() {
	return READ(ESTOP_PIN) != ESTOP_ENDSTOP_INVERTING;
}

#endif

#endif /* ESTOP_H_ */