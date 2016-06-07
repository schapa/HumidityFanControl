
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "bsp.hpp"
#include "Events.h"
#include "system.h"

int main(int argc, char* argv[]) {

	(void)argc;
	(void)argv;
	const char *greetengs = "Booting...";
	char buff[128];
	const uint8_t shutdownTout = 30;
	uint8_t humidity = 0;
	uint8_t temperature = 0;

	BSP.init();
	BSP.screenDrawText(8, 16, greetengs);
	BSP.screenDraw();

	while (true) {
		Event_t event;
		BSP.pendEvent(&event);
		switch (event.type) {
			case EVENT_SYSTICK:
				if ((event.subType.systick == ES_SYSTICK_SECOND_ELAPSED) /*&& (event.data.intptr % 2)*/) {
					BSP.startSensorAck();
					if (event.data.intptr > shutdownTout)
						BSP.shutdown();
				}
				break;
			case EVENT_DHT11:
				humidity = event.data.dht11.humidity;
				temperature = event.data.dht11.temperature;
				break;
			default:
				break;
		}

		BSP.screenClear();
		snprintf(buff, 128, "Shutd %02ld", shutdownTout - System_getUptime());
		BSP.screenDrawText(0, 0, buff);
		snprintf(buff, 128, "%02d %% %02d C", humidity, temperature);
		BSP.screenDrawText(0, 32, buff);
		BSP.screenDraw();
	}
	return 0;
}
