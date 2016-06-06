/*
 * bsp.hpp
 *
 *  Created on: May 30, 2016
 *      Author: shapa
 */

#ifndef BSP_HPP_
#define BSP_HPP_

#include <stdint.h>
#include <stddef.h>
#include "Queue.h"
#include "screen.hpp"

class BoardSupportPackage : public GxfScreen {
private:
	BoardSupportPackage() : GxfScreen(128, 64), eventQueue(NULL) {};
	BoardSupportPackage(const BoardSupportPackage &bsp) : BoardSupportPackage() { (void)bsp;};
	virtual ~BoardSupportPackage() {};

public:
	static BoardSupportPackage &getInstance() {
		static BoardSupportPackage bsp;
		return bsp;
	}
	bool init();
	void pushEvent(Event_p pEvent);
	void pendEvent(Event_p pEvent);

	void startSensorAck(void);
	void shutdown(void);

	virtual void lcd_reset(bool);
	virtual void lcd_cs(bool);
	virtual void lcd_cmd(uint8_t);
	virtual void lcd_data(uint8_t);
private:
	volatile EventQueue_p eventQueue;
};

extern BoardSupportPackage &BSP;


#endif /* BSP_HPP_ */
