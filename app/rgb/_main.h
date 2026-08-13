#pragma once

#include <Adafruit_NeoPixel.h>
#include "../../pins.h"

#define NUM_LEDS 1
Adafruit_NeoPixel leds(NUM_LEDS, RGB_DATA_PIN, NEO_GRB + NEO_KHZ800);

class LED_RGB
{
public:
	void setup()
	{
		leds.begin();
		leds.clear();
		leds.show();
	}

	void loop(bool emg_active, bool lock1_open, bool lock2_open)
	{
		uint8_t target_r = 0;
		uint8_t target_g = 0;
		uint8_t target_b = 0;

		if (emg_active)
		{
			// Prioridade maxima: EMG ativo em GND.
			target_r = 255;
			target_g = 0;
			target_b = 0;
		}
		else if (lock1_open && lock2_open)
		{
			target_r = 255;
			target_g = 80;
			target_b = 0;
		}
		else if (lock1_open || lock2_open)
		{
			target_r = 255;
			target_g = 255;
			target_b = 0;
		}
		else
		{
			// Roxo com prioridade apenas sobre o estado verde.
			target_r = 180;
			target_g = 0;
			target_b = 255;
		}

		if (target_r == last_r && target_g == last_g && target_b == last_b)
			return;

		last_r = target_r;
		last_g = target_g;
		last_b = target_b;

		leds.setPixelColor(0, leds.Color(target_r, target_g, target_b));
		leds.show();
	}

private:
	uint8_t last_r = 255;
	uint8_t last_g = 255;
	uint8_t last_b = 255;
};
