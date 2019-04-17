#include "MAXLedMatrix.h"
#include "cp437font.h"

/**
 * MAXLedMatrix.cpp - Library for controlling MAX7219 dot matrix displays
 * This library is written for the ESP platform, 
 * can be ported to Arduino by not using pgm_read_byte functions
 *
 * Created by Robert Hendriks April 17, 2019.
 *
 *
 * Copyright 2019 Robert Hendriks
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
**/



#define TARGET_ALL_DEVICES 0

//Opcodes as stated in the data sheet
#define OP_NOP 0
#define OP_D0 1
#define OP_D1 2
#define OP_D2 3
#define OP_D3 4
#define OP_D4 5
#define OP_D5 6
#define OP_D6 7
#define OP_D7 8
#define OP_DECODE_MODE 9
#define OP_INTENSITY 10
#define OP_SCAN_LIMIT 11
#define OP_SHUTDOWN 12
#define OP_TEST 15

MAXLedMatrix::MAXLedMatrix(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t nDevices)
 : SPI_MOSI_pin(dataPin), SPI_CLK_pin(clkPin), SPI_CS_pin(csPin), n_devices(nDevices),
	animation_speed(40), last_animation_time(0), 
	animation_scroll_direction(ANIMATION_SCROLL_LEFT), tfx_bit_offset(0), tfx_current_character(0),
	current_brightness(1)
{
	//Set pins to output
	pinMode(SPI_MOSI_pin, OUTPUT);
	pinMode(SPI_CLK_pin, OUTPUT);
	pinMode(SPI_CS_pin, OUTPUT);

	//Clear video buffer
	memset(gfx, 0, 64);
	memset(tfx, 0, 32);

	//Configure ICs for operation
	send_spi_packets_to_all(OP_TEST, 0);
	send_spi_packets_to_all(OP_SCAN_LIMIT, 7);
	send_spi_packets_to_all(OP_DECODE_MODE, 0);
	send_spi_packets_to_all(OP_SHUTDOWN, 1);

	send_gfx_to_display();
}

void MAXLedMatrix::setBrightness(uint8_t value){
	send_spi_packets_to_all(OP_INTENSITY, value);
}

void MAXLedMatrix::setText(char *text){
	strncpy(tfx, text, 32);
	tfx_bit_offset = 0;
	tfx_current_character = 0;
}

void MAXLedMatrix::setTextScrollDirection(uint8_t direction){
	animation_scroll_direction = direction;
}


uint8_t MAXLedMatrix::update(){
	uint8_t event = 0;
	if(millis() - last_animation_time < animation_speed){
		return event;
	}

	//Set flag that an update has been ran
	event |= ( 1 << ANIMATION_EVENT_UPDATE);

	//Text scrolling
	if(animation_scroll_direction == ANIMATION_SCROLL_LEFT){
		scroll_left_animation();
	} else if(animation_scroll_direction == ANIMATION_SCROLL_RIGHT){
		scroll_right_animation();
	}

	//Blinking
	if(animation_blink == ANIMATION_BLINK){
		if(current_brightness > 1) current_brightness = 1;
		else current_brightness = 15;
		setBrightness(current_brightness);
	}else if(animation_blink == ANIMATION_FADE_IN_OUT){
		if(current_brightness < 15) current_brightness += 1;
		else current_brightness = 1;
		setBrightness(current_brightness);
	}

	//Update
	send_gfx_to_display();
	last_animation_time = millis();
	return event;
}

void MAXLedMatrix::staticDrawCharacter(uint8_t target, char character){
	if(target > (n_devices - 1)) return;
	for(uint8_t i = 0; i < 8; i++){
		gfx[(target * 8) + i] = pgm_read_byte(cp437_font + (character * 8) + i);
	}
}

void MAXLedMatrix::staticTogglePixel(uint8_t x, uint8_t y){
	if(x > 63 || y > 7) return;
	gfx[x] ^= (1 << y);
}

void MAXLedMatrix::increase_tfx_counters(){
	tfx_bit_offset += 1;
	if(tfx_bit_offset >= 8){
		tfx_bit_offset = 0;
		tfx_current_character += 1;
	}
	if(tfx_current_character >= strlen(tfx)){
		tfx_current_character = 0;
	}
}


void MAXLedMatrix::scroll_right_animation(){
	for(int8_t i = (n_devices * 8) - 2; i >= 0; i--){
		gfx[i + 1] = gfx[i];
	}

	gfx[0] = pgm_read_byte(cp437_font + (tfx[tfx_current_character] * 8) + tfx_bit_offset);
	increase_tfx_counters();
}

void MAXLedMatrix::scroll_left_animation(){
	for(uint8_t i = 1; i < (n_devices * 8); i++){
		gfx[i - 1] = gfx[i];
	}

	gfx[(n_devices * 8) - 1] = pgm_read_byte(cp437_font + (tfx[tfx_current_character] * 8) + tfx_bit_offset);
	increase_tfx_counters();
}


void MAXLedMatrix::send_spi_packets_to_all(uint8_t opcode, uint8_t data){
	//Set pins according to datasheet
	digitalWrite(SPI_CS_pin, LOW);
	digitalWrite(SPI_CLK_pin, LOW);

	//Because of daisy chaining we need to send multiple packets 
	for(int8_t i = n_devices - 1; i >= 0; i--){
		shiftOut(SPI_MOSI_pin, SPI_CLK_pin, MSBFIRST, opcode);
		shiftOut(SPI_MOSI_pin, SPI_CLK_pin, MSBFIRST, data);
	}

	//Turn latch high
	digitalWrite(SPI_CS_pin, HIGH);
}

void MAXLedMatrix::send_gfx_to_display(){
	//Rotate buffer, TODO improve by rotating per segment and based on
//#ifdef ORIENTATION_FLIP 
	uint8_t rgfx[64] = {};
	for(uint8_t display = 0; display < n_devices; display++) {
		for(uint8_t i = 0; i < 8; i++){
			for(uint8_t j = 0; j < 8; j++){
				rgfx[i + (display * 8)] = (((gfx[j + (display * 8)] & (1 << (7-i))) >> (7-i)) << j ) | rgfx[i + (display * 8)];
			}
		}
	}
//#endif
	
	//Send video buffer towards ICs	
	for(uint8_t row = 0; row < 8; row++){
		uint8_t opcode = row + OP_D0;
		digitalWrite(SPI_CS_pin, LOW);
		digitalWrite(SPI_CLK_pin, LOW);
		for(int8_t i = 0; i < n_devices; i++){
			shiftOut(SPI_MOSI_pin, SPI_CLK_pin, MSBFIRST, opcode);
//#ifdef ORIENTATION_FLIP
			shiftOut(SPI_MOSI_pin, SPI_CLK_pin, MSBFIRST, rgfx[(i * 8) + row]);
//#else
//			shiftOut(SPI_MOSI_pin, SPI_CLK_pin, MSBFIRST, gfx[(i * 8) + row]);
//#endif
		}
		digitalWrite(SPI_CS_pin, HIGH);
	}
}
