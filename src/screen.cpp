/*
 * screen.cpp
 *
 *  Created on: Jun 5, 2016
 *      Author: shapa
 */

#include "screen.hpp"
#include "system.h"
#include <string.h>
#include <stdio.h>
#include "century_schoolbook_l_bold_24_font.h"

enum SSD1306_cmd {
	SET_LOWER_COLUMN_START_ADDRESS = 0x00, // ~0x0F
	SET_HIGHER_COLUMN_START_ADDRESS = 0x10, // ~0x1F
	SET_MEMORY_ADDRESSING_MODE = 0x20,
	SET_COLUMN_ADDRESS = 0x21,
	SET_PAGE_ADRESS = 0x22,
	SET_DISPLAY_START_LINE = 0x40, // ~0x7F
	SET_CONTRAST_BANK_0 = 0x81,
	SET_SEGMEN_REMAP = 0xA0, // 0xA1
	SET_DISPLAY_ALL_ON = 0xA4, // 0xA5
	SET_NORMAL_DISPLAY = 0xA6,
	SET_INVERSE_DISPLAY = 0xA7,
	SET_MULTIPLEX_RATIO = 0xA8,
	SET_DISPLAY_OFF = 0xAE,
	SET_DISPLAY_ON = 0xAF,
	SET_PAGE_START_ADDRESS = 0xB0, // ~0xB7h
	SET_COM_OUTPUT_SCAN_DIRECTION = 0xC0, // ~0xC8
	SET_DISPLAY_OFFSET = 0xD3,
	SET_DISPLAY_OSCILLATOR = 0xD5,
	SET_PRECHARGE_PERIOD = 0xD9,
	SET_COM_PINS_CONFIG = 0xDA,
	SET_VCOMH_DESELECT_LVL = 0xDB,

	SET_HORIZONTAL_SCROLL = 0x26, // 0x27
	SET_CONTINOUS_VH_SCROLL_SETUP = 0x29, // 0x2A
	SET_DEACTIVATE_SCROLL = 0x2E,
	SET_ACTIVATE_SCROLL = 0x2F,
	SET_VERTICAL_SCROLL_AREA = 0xA3


};

void GxfScreen::screenInit() {

	lcd_cs(false);
	lcd_reset(true);
	System_delayMsDummy(2);
	lcd_reset(false);
	System_delayMsDummy(10);

	sendCommand(SET_DISPLAY_OFF);
	sendCommand(SET_LOWER_COLUMN_START_ADDRESS);
	sendCommand(SET_HIGHER_COLUMN_START_ADDRESS);
	sendCommand(SET_DISPLAY_START_LINE);

	sendCommand(SET_CONTRAST_BANK_0, 0xFF);
	sendCommand(SET_SEGMEN_REMAP | 1);
	sendCommand(SET_NORMAL_DISPLAY);
	sendCommand(SET_DISPLAY_ALL_ON);
	sendCommand(SET_MULTIPLEX_RATIO, 0x3F);
	sendCommand(SET_DISPLAY_OFFSET, 0x00);
	sendCommand(SET_DISPLAY_OSCILLATOR, 0x80);
	sendCommand(SET_PRECHARGE_PERIOD, 0xF1);
	sendCommand(SET_COM_PINS_CONFIG, 0x12);
	sendCommand(SET_VCOMH_DESELECT_LVL, 0x40);
	sendCommand(SET_MEMORY_ADDRESSING_MODE, 0x00);

	sendCommand(SET_SEGMEN_REMAP | 1);
	sendCommand(SET_COM_OUTPUT_SCAN_DIRECTION | 8);

	sendCommand(0x8D, 0x14);

	screenClear();
	screenDraw();
	sendCommand(SET_DISPLAY_ON);
}


void GxfScreen::screenClear(void) {
	memset(bytes, 0, bufferSize);
}

void GxfScreen::screenDraw(void) {

	sendCommand(SET_LOWER_COLUMN_START_ADDRESS | 0x0);
	sendCommand(SET_HIGHER_COLUMN_START_ADDRESS | 0x0);
	sendCommand(SET_DISPLAY_START_LINE | 0x0);
	lcd_cs(true);
	for (uint16_t i = 0; i < bufferSize; i++) {
		lcd_data(bytes[i]);
	}
	lcd_cs(false);
}

void GxfScreen::screenDrawPixel(uint16_t x, uint16_t y, bool val) {
	if ((x >= width) || (y >= heigth))
		return;

	uint8_t mask = 1<<(y & 0x07);
    uint8_t *ptr = &bytes[x + (y/8)*width];

    if (val)
    	*ptr |= mask;
    else
    	*ptr &= ~mask;
}


void GxfScreen::screenDrawText(uint16_t xPos, uint16_t yStart, const char *text) {

	const fontItem_t *font = &century_schoolbook_l_bold_24_font;
	fontLookupItem_p lookup = font->lookup;
	const uint8_t *pixel = font->pixelData;
	while (*text) {
		fontLookupItem_t character = lookup[(size_t)*text];
		const uint8_t charWidth = (character.width/8 + !!(character.width%8));
	    for (uint16_t y = 0; (y < character.heigth) && (y < heigth); ++y) {
	    	uint16_t xStart = character.offset + y * charWidth;
		    for (uint16_t x = 0; (x < character.width) && (x < width); ++x) {
		    	screenDrawPixel(xPos + x, y + yStart + character.top, pixel[xStart + x/8] & 1<<(x%8));
	    	}
	    }
	    xPos += character.advance - character.left;
	    if (xPos > width)
	    	break;
	    text++;
	}
}


void GxfScreen::sendCommand(uint8_t val) {
	lcd_cs(true);
	lcd_cmd(val);
	lcd_cs(false);
}

void GxfScreen::sendCommand(uint8_t val1, uint8_t val2) {
	lcd_cs(true);
	lcd_cmd(val1);
	lcd_cmd(val2);
	lcd_cs(false);
}
void GxfScreen::sendCommand(uint8_t val1, uint8_t val2, uint8_t val3) {
	lcd_cs(true);
	lcd_cmd(val1);
	lcd_cmd(val2);
	lcd_cmd(val3);
	lcd_cs(false);
}
