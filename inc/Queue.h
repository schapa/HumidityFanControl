/*
 * Queue.h
 *
 *  Created on: Apr 20, 2016
 *      Author: shapa
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "Events.h"

typedef struct Queue EventQueue_t, *EventQueue_p;

struct Queue {
	Event_t event;
	EventQueue_p next;
	EventQueue_p last;
};

EventQueue_p Queue_pushEvent(EventQueue_p pQueue, Event_p pEvent);
EventQueue_p Queue_getEvent(EventQueue_p pQueue, Event_p pEvent);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H_ */
