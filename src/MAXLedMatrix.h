#ifndef _MAXLedMatrix_
#define _MAXLedMatrix_

#include <Arduino.h>
#include <stdint.h>

//Define to change the orientation, used for correct working of some chinese LED matrixes
#define ORIENTATION_FLIP

/**
 * MAXLedMatrix.h - Library for controlling MAX7219 dot matrix displays
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

#define ANIMATION_NO_SCROLL 0
#define ANIMATION_SCROLL_LEFT 1
#define ANIMATION_SCROLL_RIGHT 2

#define ANIMATION_NO_BLINK 4
#define ANIMATION_BLINK 5
#define ANIMATION_FADE_IN_OUT 6

//Flags return by animation loop (the number gives the bit position of the flag)
//Flag indicates that an update has occured
#define ANIMATION_EVENT_UPDATE 0
//Flag indicates that a letter has been fully scrolled into the display
#define ANIMATION_LETTER_SCROLL 1
//Flag indicates that the full text in tfx has been scrolled in
#define ANIMATION_EVENT_FULLSCROLL 2

class MAXLedMatrix {
	public:
		/**
		 * @brief Constructor
		 * 
		 * @param dataPin The SPI_MOSI pin of the controller
		 *
		 * @param clkPin The SPI_CLOCK pin of the controller
		 *
		 * @param csPin The chip-select pin, can be any free pin on the controller
		 */
		MAXLedMatrix(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t nDevices = 1);

		/**
		 * @brief Set the brightness of one of the LED matrixes
		 *
		 * @param brightness The brightness level to set, max value is 16
		 */
		void setBrightness(uint8_t value);

		/**
		 * @brief Set a certain text on the display
		 *
		 * @param text The text to be shown on the display
		 */
		void setText(char *text);

		/**
		 * @brief Draw a character to the GFX buffer directly
		 *
		 * @param target Device to draw character on, 0 indexed
		 *
		 * @param character The character to be drawn
		 */
		void staticDrawCharacter(uint8_t target, char character);

		/**
		 * @brief Toggle the state of a certain pixel
		 *
		 * @param x The x position of the pixel, 0 indexed
		 *
		 * @param y The y position of the pixel, 0 indexed
		 */
		void staticTogglePixel(uint8_t x, uint8_t y);

		/**
		 * @brief Set the scroll direction of the text
		 *
		 * @param direction The id of the scroll direction
		 */
		void setTextScrollDirection(uint8_t direction);

		/**
		 * @brief Update the display
		 */
		uint8_t update();
		
	private:
		//Buffer to send SPI packets to IC
		uint8_t SPI_MOSI_pin;
		uint8_t SPI_CLK_pin;
		uint8_t SPI_CS_pin;
		uint8_t n_devices;

		//Video buffer
		uint8_t gfx[64];

		//Text buffer
		uint8_t tfx_bit_offset;
		int8_t tfx_current_character;
		char tfx[32];

		//Current settings
		uint8_t current_brightness;

		//Bytes holding the animations that are enabled
		uint8_t animation_scroll_direction;
		uint8_t animation_blink;
		uint8_t animation_border;

		//Speed in mili seconds for the animations
		uint16_t animation_speed;

		//Hold the time of the last animation step
		uint64_t last_animation_time;

		//Send a packet over the SPI bus
		void send_spi_packets_to_all(uint8_t opcode, uint8_t data);

		//Send the video buffer to the display
		void send_gfx_to_display();

		//Scroll gfx left
		void scroll_left_animation();

		//Scroll gfx right
		void scroll_right_animation();

		//Increase internal TFX counters
		void increase_tfx_counters();

		void antiClockWise();
};



#endif
