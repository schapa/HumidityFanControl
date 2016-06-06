/*
 * systemTimer.c
 *
 *  Created on: April 2, 2016
 *      Author: pavelgamov
 */

#include <stddef.h>
#include "system.h"
#include "bsp.hpp"

static volatile uint32_t s_delayDecrement = 0;
static volatile uint32_t s_uptimeSeconds = 0;
static volatile uint32_t s_uptimeTicks = 0;

extern "C" void SysTick_Handler(void) {
	if (s_delayDecrement)
		s_delayDecrement--;

	if (!(++s_uptimeTicks % BSP_TICKS_PER_SECOND)) {
		s_uptimeSeconds++;
		Event_t seconds;
		seconds.type = EVENT_SYSTICK;
		seconds.subType.systick = ES_SYSTICK_SECOND_ELAPSED;
		seconds.data.intptr = s_uptimeSeconds;
		BSP.pushEvent(&seconds);
	}
}

void System_delayMsDummy(uint32_t delay) {
	s_delayDecrement = delay;
	while (s_delayDecrement);
}

uint32_t System_getUptime(void) {
	return s_uptimeSeconds;
}
uint32_t System_getUptimeMs(void) {
	return s_uptimeTicks%BSP_TICKS_PER_SECOND;
}
