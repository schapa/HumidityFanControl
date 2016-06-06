/*
 * Events.h
 *
 *  Created on: Apr 20, 2016
 *      Author: shapa
 */

#ifndef EVENTS_H_
#define EVENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	EVENT_DUMMY,
	EVENT_SYSTICK,
	EVENT_EXTI,
	EVENT_DHT11,
	EVENT_LAST
} EventTypes_t;

typedef enum {
	ES_SYSTICK_TICK,
	ES_SYSTICK_SECOND_ELAPSED
} SystickSubTypes_t;

typedef enum {
	ES_EXTI_DOWN,
	ES_EXTI_UP,
} ExtiSubTypes_t;

typedef struct {
	uint8_t id;
	uint8_t humidity;
	uint8_t temperature;
} EventDataDht11_t;

typedef union {
	SystickSubTypes_t systick;
	ExtiSubTypes_t exti;
} EventSubTypes_t;

typedef union {
	intptr_t intptr;
	EventDataDht11_t dht11;
} EventDataTypes_t;

typedef struct {
	EventTypes_t type;
	EventSubTypes_t subType;
	EventDataTypes_t data;
} Event_t, *Event_p;

#ifdef __cplusplus
}
#endif

#endif /* EVENTS_H_ */
