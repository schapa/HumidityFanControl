/*
 * screen.hpp
 *
 *  Created on: Jun 5, 2016
 *      Author: shapa
 */

#ifndef SCREEN_HPP_
#define SCREEN_HPP_

#include <stdint.h>

class GxfScreen {
public:
	GxfScreen (uint16_t width, uint16_t heigth) : width(width), heigth(heigth), bufferSize(width/8*heigth) {
		bytes = new uint8_t[bufferSize];
	};
	virtual ~GxfScreen() { delete bytes; };

	void screenInit();
	void screenClear(void);
	void screenDraw(void);
	void screenDrawPixel(uint16_t x, uint16_t y, bool val);
	void screenDrawText(uint16_t x, uint16_t y, const char *text);
protected:
	virtual void lcd_reset(bool) = 0;
	virtual void lcd_cs(bool) = 0;
	virtual void lcd_cmd(uint8_t) = 0;
	virtual void lcd_data(uint8_t) = 0;
private:
	void sendCommand(uint8_t);
	void sendCommand(uint8_t, uint8_t);
	void sendCommand(uint8_t, uint8_t, uint8_t);

	uint16_t width;
	uint16_t heigth;
	const uint16_t bufferSize;
	uint8_t *bytes;
};




#endif /* SCREEN_HPP_ */
