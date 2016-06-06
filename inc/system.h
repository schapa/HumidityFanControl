/*
 * systemStatus.h
 *
 *  Created on: Jan 9, 2016
 *      Author: pavelgamov
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define BSP_TICKS_PER_SECOND 1000

uint32_t System_getUptime(void);
uint32_t System_getUptimeMs(void);

void System_delayMsDummy(uint32_t delay);

#ifdef __cplusplus
}
#endif


#endif /* SYSTEM_H_ */
