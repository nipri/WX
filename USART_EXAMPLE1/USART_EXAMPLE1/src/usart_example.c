/**
 * \file
 *
 * \brief AVR XMEGA USART example
 *
 * Copyright (C) 2010 - 2011 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/*! \mainpage
 * \section intro Introduction
 * This example demonstrates how to use XMEGA USART module.
 *
 * \section files Main Files
 * - usart_example.c: the example application.
 * - conf_board.h: board configuration
 * - conf_usart_example.h: configuration of the example
 *
 * \section usart_apiinfo drivers/usart API
 * The USART driver API can be found \ref usart_group "here".
 *
 * \section deviceinfo Device Info
 * All AVR XMEGA devices can be used.
 * This example has been tested with the following setup:
 *   - STK600 
 *     USARTC0 should be connected to the RS232 spare port of STK600
 *       Note:
 *       XMEGA-A1 on STK600-RC100X
 *       XMEGA-A3, XMEGA-A3U and XMEGA-C3 on STK600-RC064X.
 *       XMEGA-A4U on STK600-RC044X
 *       XMEGA-E5 on STK600-RC032X
 *   - Xplain evaluation kit
 *     USARTD0 on PORTD is used by default
 *     Change to USARTC0 to use the USB Virtual COM PORT of the Xplain
 *   - XMEGA A1 Xplained evaluation kit
 *     USARTC0 on PORTC is used. It is located on the J4 header
 *   - XMEGA A3BU Xplained evaluation kit
 *     USARTC0, pin 2 and 3 on header J1 is utilized
 *   - XMEGA E5 Xplained evaluation kit
 *     USARTD0 on PORTD pin 6 and 7 is used (Connected to board controller)
 * UART configuration is 9600 baudrate, no parity, data 8 bit.
 *
 * \section exampledescription Description of the example
 * The example waits for a received character on the configured USART and
 * echoes the character back to the same USART.
 *
 * \section compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for AVR.
 * Other compilers may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/avr">Atmel AVR</A>.\n
 */
#include <conf_usart_example.h>
#include <asf.h>
#include <string.h>
#include <stdio.h>
#include <board.h>
#include <sensor.h>
#include <util/delay.h>
#//include <rtc.h>
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t tx_buf[] = "\n\rHello AVR world ! : ";
uint8_t press_err[] = "Problem initializing pressure sensor!\r\n";
uint8_t tx_buf2[128] = "";

uint32_t id;
uint8_t ver;
sensor_t barometer;
sensor_data_t press_data;
sensor_data_t temp_data;

static bool isLED = false;

inline void sendUARTdata(uint8_t data[], uint8_t length);
inline void getTempPress(void);

struct datetime {
	
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t month;
	uint8_t date;
	uint16_t year;
	}datetime;


ISR (RTC_OVF_vect) {
	
	datetime.seconds++;
	
	if (datetime.seconds > 59) {
		datetime.seconds = 0;
		datetime.minutes++;
		
		if (datetime.minutes > 59) {
			datetime.minutes = 0;
			datetime.hours++;
			
			if (datetime.hours > 23 ) {
				datetime.hours = 0;
			}		
		}
	}
	
	getTempPress();
	
	if (isLED == true) {
		isLED = false;
		PORTE.OUTSET = 0x01;
	} else {
		isLED = true;
		PORTE.OUTCLR = 0x01;
	}
}

inline void sendUARTdata(uint8_t data[], uint8_t length) {
	
	uint8_t i;
	
	if (length > 1) {
		
		for (i = 0; i < length; i++) {
			usart_putchar(USART_SERIAL_EXAMPLE, data[i]);
	
		}
	} else {
		usart_putchar(USART_SERIAL_EXAMPLE, data[0]);
		usart_putchar(USART_SERIAL_EXAMPLE, '\r');
		usart_putchar(USART_SERIAL_EXAMPLE, '\n');
	}
	
}

inline void getTempPress(void) {
			
	//time = rtc_get_time();
			
	sensor_get_pressure(&barometer, &press_data);
	sensor_get_temperature(&barometer, &temp_data);
			
	memset(tx_buf2, 0, 128);
			
	//sprintf( (char *)tx_buf2, "Squat: %d	Squat1: %d\r\n", RTC.CTRL, RTC.CNTL);
	sprintf( (char *)tx_buf2, "Time: %2d:%2d:%2d	", datetime.hours, datetime.minutes, datetime.seconds);
	sendUARTdata(tx_buf2, sizeof(tx_buf2));
			
	sprintf( (char *)tx_buf2, "Time: %lu	Pressure: %.2f in Hg	Temperature: %.1f C\r\n", temp_data.timestamp, (  ( (double)press_data.pressure.value / 100) / 33.864  ), (double)temp_data.temperature.value / 10 );
	sendUARTdata(tx_buf2, sizeof(tx_buf2));
			
}

int main(void)
{

//	uint8_t received_byte;
//	uint32_t id;
//	uint8_t ver;
//	sensor_t barometer;
//	sensor_data_t press_data;
//	sensor_data_t temp_data;

	/* Initialize the board.
	 * The board-specific conf_board.h file contains the configuration of
	 * the board initialization.
	 */
//	cli();
	board_init();
	pmic_init();
	sysclk_init();
//	sleepmgr_init();
	sensor_platform_init();
	rtc_init();
	PORTE.DIRSET = 0x01;
	

// Init the RTC

	CLK.RTCCTRL = 0x05;

//	while ( !( OSC_STATUS & OSC_RC32KRDY_bm ) ); /* Wait for the int. 32kHz oscillator to stabilize. */
	PMIC_CTRL |= 0x01; // Set Int. priority level to low in PMIC
		
	while( ( RTC_STATUS & 0x01 ) ); // Needed B 4 writing to RTC PER / CNT registers
	RTC.PER = 0x0400;
	RTC.CTRL = 0x01;				
	RTC.INTCTRL = 0x01;	 //Set this to match the interrupt level in PMIC_CTRL	
		

	sensor_attach(&barometer, SENSOR_TYPE_BAROMETER, 0, 0);
	
	sensor_set_state(&barometer, SENSOR_STATE_HIGHEST_POWER);
	
	press_data.scaled = true;
	temp_data.scaled = true;

	// USART options.
	static usart_rs232_options_t USART_SERIAL_OPTIONS = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT
	};

	// Initialize usart driver in RS232 mode
	usart_init_rs232(USART_SERIAL_EXAMPLE, &USART_SERIAL_OPTIONS);

	// Send "message header"
	sendUARTdata(tx_buf, 22);
	
	//	sysclk_rtcsrc_enable(SYSCLK_SRC_RC2MHZ);
//		rtc_init();

	
	if (barometer.err) {
		sendUARTdata(press_err, 39);
	}
	else {
		memset(tx_buf2, 0, 128);
		sensor_device_id(&barometer, &id, &ver);
		sprintf((char*)tx_buf2, "%s\r\n\r\nSensor ID: 0x%02x ver: 0x%02x\r\n%d bit resolution\r\n\r\n", barometer.drv->caps.name, (unsigned)id, (unsigned)ver, barometer.hal->resolution);
		sendUARTdata(tx_buf2, sizeof(tx_buf2));				
	}
	
//	cpu_irq_enable();
	sei();
	
	while (true) {
		
//		_delay_ms(1000);
		
//		time = rtc_get_time();
		
//		sensor_get_pressure(&barometer, &press_data);
//		sensor_get_temperature(&barometer, &temp_data);
		
//		memset(tx_buf2, 0, 128);
		
//		sprintf( (char *)tx_buf2, "Squat: %d	Squat1: %d\r\n", RTC.CTRL, RTC.CNTL);
//		sprintf( (char *)tx_buf2, "Time: %2d:%2d:%2d	", datetime.hours, datetime.minutes, datetime.seconds);
//		sendUARTdata(tx_buf2, sizeof(tx_buf2));
			
//		sprintf( (char *)tx_buf2, "Pressure: %.2f in Hg	Temperature: %.1f C\r\n", (  ( (double)press_data.pressure.value / 100) / 33.864  ), (double)temp_data.temperature.value / 10 );
//		sendUARTdata(tx_buf2, sizeof(tx_buf2));	
		
	}	
}
