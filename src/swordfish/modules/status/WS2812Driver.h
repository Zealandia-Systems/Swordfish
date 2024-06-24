#pragma once

#include <cstddef>
#include <cstdint>

#include <Adafruit_ZeroDMA.h>

#include <swordfish/types.h>

#include "Color.h"

namespace swordfish::status {
	class WS2812Driver {
	private:
		Adafruit_ZeroDMA dma_;
		DmacDescriptor* dma_desc_[2];
		u16 led_count_;
		usize buffer_length_;
		u32 frame_period_;
		u32 sweep_time_;
		u8* buffers_[2];
		usize active_buffer_;
		usize buffer_index_;
		u8 active_r_;
		u8 active_g_;
		u8 active_b_;
		u8 brightness_;
		u32 counter_;
		bool sweep_;
		bool reverse_;

		void write_u8(u8 value);
		void write_color(u8 r, u8 g, u8 b);

	public:
		WS2812Driver(u16 led_count, u8 led_brightness, u16 sweep_time);

		void set_led_count(u16 led_count);
		void set_sweep_time(u16 sweep_time);
		void set_color(u8 r, u8 g, u8 b);
		void set_sweep(bool sweep);
		void set_brightness(u8 brightness);
		void init();

		void update();
	};
}