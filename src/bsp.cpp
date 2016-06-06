/*
 * bsp.cpp
 *
 *  Created on: May 30, 2016
 *      Author: shapa
 */

#include "bsp.hpp"
#include "system.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_tim.h"
#include "diag/Trace.h"

static const uint8_t s_bitsCount = 42;

static struct dht11 {
	uint8_t startTime;
	uint8_t buffer[s_bitsCount];
	uint8_t bufferCnt;
} s_dht11_outer, s_dht11_inner;

static void initRCC(void);
static void initGPIO_Power(void);
static void initGPIO_DHT11(void);
static void initGPIO_SPI(void);

static void init_DHT11(void);
static void init_SPI(void);
static void init_TIM(void);
static void dht11_process(dht11 *item);

BoardSupportPackage &BSP = BoardSupportPackage::getInstance();

bool BoardSupportPackage::init() {
	bool result = false;
	SysTick_Config(SystemCoreClock/BSP_TICKS_PER_SECOND);
	initRCC();
	initGPIO_Power();
	init_DHT11();
	init_SPI();
	init_TIM();
 	screenInit();

	result = true;
	return result;
}

void BoardSupportPackage::pushEvent(Event_p pEvent) {
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	eventQueue = Queue_pushEvent(eventQueue, pEvent);
	if (!primask) {
		__enable_irq();
	}
}

void BoardSupportPackage::pendEvent(Event_p pEvent) {
	while (!eventQueue);
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	eventQueue = Queue_getEvent(eventQueue, pEvent);
	if (!primask) {
		__enable_irq();
	}
}

void BoardSupportPackage::startSensorAck(void) {
	NVIC_DisableIRQ(EXTI2_3_IRQn);
	GPIO_ResetBits(GPIOA, GPIO_Pin_3);
	s_dht11_outer.bufferCnt = 0;
	s_dht11_inner.bufferCnt = 0;
	System_delayMsDummy(20);
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
	NVIC_EnableIRQ(EXTI2_3_IRQn);
}

void BoardSupportPackage::shutdown(void) {
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

void BoardSupportPackage::lcd_reset(bool val) {
	if (val)
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	else
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
}

void BoardSupportPackage::lcd_cs(bool val) {
	if (val)
		GPIO_ResetBits(GPIOA, GPIO_Pin_10);
	else
		GPIO_SetBits(GPIOA, GPIO_Pin_10);
}

void BoardSupportPackage::lcd_cmd(uint8_t val) {
	GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	SPI_SendData8(SPI1, val);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
}

void BoardSupportPackage::lcd_data(uint8_t val) {
	GPIO_SetBits(GPIOA, GPIO_Pin_9);
	SPI_SendData8(SPI1, val);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
}

extern "C" void EXTI2_3_IRQHandler(void) {
	uint32_t count = TIM_GetCounter(TIM14);

	if (EXTI_GetITStatus(EXTI_Line2)) {
		EXTI_ClearITPendingBit(EXTI_Line2);
		if (GPIOA->IDR & GPIO_Pin_2) {
			s_dht11_outer.startTime = count;
		} else {
			if (count < s_dht11_inner.startTime)
				s_dht11_outer.buffer[s_dht11_outer.bufferCnt++] = count + (255 - s_dht11_outer.startTime);
			else
				s_dht11_outer.buffer[s_dht11_outer.bufferCnt++] = count - s_dht11_outer.startTime;
		}
		if (s_dht11_outer.bufferCnt == s_bitsCount) {
			dht11_process(&s_dht11_outer);
		}
	}

	if (EXTI_GetITStatus(EXTI_Line3)) {
		EXTI_ClearITPendingBit(EXTI_Line3);
		if (GPIOA->IDR & GPIO_Pin_3) {
			s_dht11_inner.startTime = count;
		} else {
			if (count < s_dht11_inner.startTime)
				s_dht11_inner.buffer[s_dht11_inner.bufferCnt++] = count + (255 - s_dht11_inner.startTime);
			else
				s_dht11_inner.buffer[s_dht11_inner.bufferCnt++] = count - s_dht11_inner.startTime;
		}
		if (s_dht11_inner.bufferCnt == s_bitsCount) {
			dht11_process(&s_dht11_inner);
			s_dht11_inner.bufferCnt = 0;
		}
	}
}

static void initRCC(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
}

static void initGPIO_Power(void) {
	GPIO_InitTypeDef iface = {
			GPIO_Pin_1,
			GPIO_Mode_OUT,
			GPIO_Speed_Level_1,
			GPIO_OType_PP,
			GPIO_PuPd_NOPULL
	};
	GPIO_Init(GPIOA, &iface);
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
}
static void initGPIO_DHT11(void) {
	GPIO_InitTypeDef iface = {
			GPIO_Pin_2 | GPIO_Pin_3,
			GPIO_Mode_OUT,
			GPIO_Speed_Level_1,
			GPIO_OType_OD,
			GPIO_PuPd_UP
	};
	GPIO_Init(GPIOA, &iface);
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
}

static void initGPIO_SPI(void) {
//	PA10 = CS
//	PA9 = D/C#
//	PB1 = Reset
//	PA7 = MOSI
//	PA5 = SCK
	GPIO_InitTypeDef iface = {
			GPIO_Pin_5 | GPIO_Pin_7,
			GPIO_Mode_AF,
			GPIO_Speed_Level_3,
			GPIO_OType_PP,
			GPIO_PuPd_NOPULL
	};
	GPIO_Init(GPIOA, &iface);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0); // SCK
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0); // MOSI

	iface.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	iface.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &iface);

	iface.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &iface);

	GPIO_ResetBits(GPIOA, GPIO_Pin_9);
}

static void init_DHT11(void) {
	initGPIO_DHT11();
	NVIC_InitTypeDef interrupt = {
			EXTI2_3_IRQn,
			0,
			ENABLE
	};
	EXTI_InitTypeDef exti = {
			EXTI_Line2 | EXTI_Line3,
			EXTI_Mode_Interrupt,
			EXTI_Trigger_Rising_Falling,
			ENABLE
	};
	NVIC_Init(&interrupt);
	EXTI_Init(&exti);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource3);
}

static void init_SPI(void) {

	SPI_InitTypeDef iface = {
			SPI_Direction_1Line_Tx,
			SPI_Mode_Master,
			SPI_DataSize_8b,
			SPI_CPOL_Low,
			SPI_CPHA_1Edge,
			SPI_NSS_Soft,
			SPI_BaudRatePrescaler_2, //SPI_BaudRatePrescaler_64,
			SPI_FirstBit_MSB,
			0xFEDA
	};

	initGPIO_SPI();
	SPI_Init(SPI1, &iface);
	SPI_TIModeCmd(SPI1, DISABLE);
	SPI_Cmd(SPI1, ENABLE);
}

static void init_TIM(void) {
	TIM_TimeBaseInitTypeDef iface = {
			0x2,
			TIM_CounterMode_Up,
			0xFF,
			TIM_CKD_DIV1,
			0
	};

	TIM_TimeBaseInit(TIM14, &iface);
	TIM_Cmd(TIM14, ENABLE);
}

static void dht11_process(dht11 *item) {
	const uint8_t validBits = s_bitsCount - 2; // Omit first two bits
	uint8_t realData[s_bitsCount/8] = { 0 };
	uint16_t median = 0;
	uint8_t crc = 0;

	for (uint8_t i = 0; i < validBits; i++) {
		median += item->buffer[i + 2];
	}
	median /= s_bitsCount;

	for (uint8_t i = 0; i < validBits; i++) {
		if (item->buffer[i + 2] > median)
			realData[i/8] |= 0x80>>(i%8); // MSB first
	}
	for (uint8_t i = 0; i < 4; i++) {
		crc += realData[i];
	}
	if (crc == realData[4]) {
		Event_t data;
		data.type = EVENT_DHT11;
		data.data.dht11.id = 0;
		data.data.dht11.humidity = realData[0];
		data.data.dht11.temperature = realData[2];
		BSP.pushEvent(&data);
	}
}
