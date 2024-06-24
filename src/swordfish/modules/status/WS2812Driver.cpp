#include <cstring>
#include <sam.h>

#include "StatusModule.h"
#include "WS2812Driver.h"

namespace swordfish::status {
	static const u8 gamma_table__[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,
    3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,
    6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,
    11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,
    17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
    25,  26,  27,  27,  28,  29,  29,  30,  31,  31,  32,  33,  34,  34,  35,
    36,  37,  38,  38,  39,  40,  41,  42,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,  80,  81,
    82,  84,  85,  86,  88,  89,  90,  92,  93,  94,  96,  97,  99,  100, 102,
    103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 119, 120, 122, 124, 125,
    127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148, 150, 152,
    154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182,
    184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211, 213, 215,
    218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252,
    255};

	WS2812Driver::WS2812Driver(u16 led_count, u8 led_brightness, u16 sweep_time) :
		dma_desc_ { nullptr, nullptr },
		led_count_(led_count),
		buffer_length_(0),
		frame_period_(0),
		buffers_ { nullptr, nullptr },
		active_buffer_(0),
		active_r_(0),
		active_g_(255),
		active_b_(0),
		brightness_(255),
		counter_(0),
		sweep_(false),
		reverse_(false)
	{
		set_led_count(led_count);
		set_sweep_time(sweep_time);
	}

	void WS2812Driver::set_led_count(u16 led_count) {
		debug()("led_count: ", led_count);

		led_count_ = led_count;
		buffer_length_ = (led_count + 40) * 3 * 4;
		frame_period_ = (u32)(3000000/buffer_length_);

		if(buffers_[0]) {
			delete buffers_[0];
		}

		if(buffers_[1]) {
			delete buffers_[1];
		}

		buffers_[0] = new u8[buffer_length_];
		buffers_[1] = new u8[buffer_length_];

		memset(buffers_[0], 0x00, buffer_length_);
		memset(buffers_[1], 0x00, buffer_length_);

		// reconfigure DMAC

		if(dma_desc_[0]) {
			dma_.changeDescriptor(dma_desc_[0], buffers_[0], (void *)&I2S->TXDATA.reg, buffer_length_);
		}

		if(dma_desc_[1]) {
			dma_.changeDescriptor(dma_desc_[1], buffers_[1], (void *)&I2S->TXDATA.reg, buffer_length_);
		}
	}

	void WS2812Driver::set_color(u8 r, u8 g, u8 b) {
		active_r_ = r;
		active_g_ = g;
		active_b_ = b;
	}

	void WS2812Driver::set_sweep(bool sweep) {
		sweep_ = sweep;
	}

	void WS2812Driver::set_brightness(u8 brightness) {
		brightness_ = brightness;
	}

	void WS2812Driver::set_sweep_time(u16 sweep_time) {
		sweep_time_ = (u32)((f32)sweep_time / 3.0f);

		debug()("sweep_time: ", sweep_time_);
	}

	void dma_callback(Adafruit_ZeroDMA *dma) {
		(void) dma;

		auto& driver = StatusModule::getInstance().driver();

		driver.update();
	}

	void WS2812Driver::init() {
		dma_.setTrigger(I2S_DMAC_ID_TX_0);
		dma_.setAction(DMA_TRIGGER_ACTON_BEAT);
		dma_.allocate();

		dma_desc_[0] = dma_.addDescriptor(buffers_[0], (void *)&I2S->TXDATA.reg, buffer_length_, DMA_BEAT_SIZE_BYTE, true, false);
		dma_desc_[1] = dma_.addDescriptor(buffers_[1], (void *)&I2S->TXDATA.reg, buffer_length_, DMA_BEAT_SIZE_BYTE, true, false);

		dma_desc_[0]->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;
		dma_desc_[1]->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;

		dma_.loop(true);
		dma_.setCallback(dma_callback);

		// enable I2S SDO on port PA21 (D32)
		PORT->Group[0].PINCFG[21].bit.PMUXEN = 1;
		PORT->Group[0].PMUX[21 >> 1].bit.PMUXO = 0x9;

		// enable I2S FS on port PA20 (D33)
		//PORT->Group[0].PINCFG[20].bit.PMUXEN = 1;
		//PORT->Group[0].PMUX[20 >> 1].bit.PMUXE = 0x9;

		// TEMPORARY
		// enable I2S SCK on port PB12 (D18)
		//PORT->Group[1].PINCFG[12].bit.PMUXEN = 1;
		//PORT->Group[1].PMUX[12 >> 1].bit.PMUXE = 0x9;

		// enable APBD for I2S
		MCLK->APBDMASK.bit.I2S_ = true;

		// connect 12MHz clock generator to I2S
		GCLK->PCHCTRL[I2S_GCLK_ID_0].reg = GCLK_PCHCTRL_GEN(4) | GCLK_PCHCTRL_CHEN;
		while (0 == (GCLK->PCHCTRL[I2S_GCLK_ID_0].reg & GCLK_PCHCTRL_CHEN));

		GCLK->PCHCTRL[I2S_GCLK_ID_1].reg = GCLK_PCHCTRL_GEN(4) | GCLK_PCHCTRL_CHEN;
		while (0 == (GCLK->PCHCTRL[I2S_GCLK_ID_1].reg & GCLK_PCHCTRL_CHEN));

		// reset I2S
		I2S->CTRLA.bit.SWRST = true;
		while (I2S->SYNCBUSY.bit.SWRST);

		// configure I2S clocks
		I2S->CLKCTRL[0].reg = I2S_CLKCTRL_SLOTSIZE_8
			| I2S_CLKCTRL_NBSLOTS(1)
			| I2S_CLKCTRL_MCKEN
			| I2S_CLKCTRL_MCKSEL_GCLK
			| I2S_CLKCTRL_SCKSEL_MCKDIV
			| I2S_CLKCTRL_FSSEL_SCKDIV
			| I2S_CLKCTRL_MCKOUTDIV(0)
			| I2S_CLKCTRL_MCKDIV(3);

		I2S->TXCTRL.reg = I2S_TXCTRL_MONO_STEREO
			| I2S_TXCTRL_DATASIZE_8
			| I2S_TXCTRL_TXSAME_ZERO
			| I2S_TXCTRL_TXDEFAULT_ZERO;

		I2S->CTRLA.bit.ENABLE = true;
		while(I2S->SYNCBUSY.bit.ENABLE);

		I2S->CTRLA.bit.CKEN0 = true;
		while(I2S->SYNCBUSY.bit.CKEN0);

		I2S->CTRLA.bit.CKEN1 = true;
		while(I2S->SYNCBUSY.bit.CKEN1);

		I2S->CTRLA.bit.TXEN = true;
		while(I2S->SYNCBUSY.bit.TXEN);

		dma_.startJob();
	}

	void WS2812Driver::write_u8(u8 value) {
		static constexpr u32 PATTERNS[] = {0b10001000, 0b10001110, 0b11101000, 0b11101110};

		//value = gamma_table__[value];

		for(auto i = 0; i < 4; i ++) {
			auto bits = (value & 0b11000000) >> 6;

			buffers_[active_buffer_][buffer_index_++] = PATTERNS[bits];

			value <<= 2;
		}
	}

	void WS2812Driver::write_color(u8 r, u8 g, u8 b) {
		write_u8(g > brightness_ ? brightness_ : g);
		write_u8(r > brightness_ ? brightness_ : r);
		write_u8(b > brightness_ ? brightness_ : b);
	}

	f32 lerp(f32 start, f32 end, f32 fraction) {
		return (start + (end - start) * fraction);
	}

	f32 ease_in(f32 t) {
		return t * t;
	}

	f32 flip(f32 x) {
		return 1.0f - x;
	}

	f32 ease_out(f32 t) {
		auto v = flip(t);

		return flip(v * v);
	}

	f32 ease_in_out(f32 t) {
		return lerp(ease_in(t), ease_out(t), t);
	}

	f32 ease_out_in(f32 t) {
		return lerp(ease_out(t), ease_in(t), t);
	}

	f32 ease_in_out_exp(f32 t) {
		return t == 0.0f
  		? 0.0f
  		: t == 1.0f
  		? 1.0f
  		: t < 0.5f ? powf(2, 20 * t - 10) / 2
  		: (2 - powf(2, -20 * t + 10)) / 2;
	}

	void WS2812Driver::update() {
		active_buffer_ = 1 - active_buffer_;
		buffer_index_ = 0;

		const u32 tail_length_ = 20;

		if (sweep_) {
			u32 progress = counter_ % sweep_time_;
			f32 global_t = (f32)progress / (f32)sweep_time_;

			if (progress == 0) {
				reverse_ = !reverse_;
			}

			u32 vled_count = led_count_ + tail_length_ + 2;
			i32 head = (i32)((f32)vled_count * global_t);
			i32 tail = head - tail_length_;

			if (reverse_) {
				tail = led_count_ - head;
				head = tail + tail_length_;
			}

			for (i32 i = 0; i < led_count_; i ++) {

				if (tail <= i && i < head) {
					f32 t = std::fmod((f32)((i - tail) % tail_length_) / (float32_t)tail_length_, 1.0f);

					if (reverse_) {
						t = 1.0f - t;
					}

					f32 v = ease_in_out(t);

					write_color(
						(u8)(active_r_ * v),
						(u8)(active_g_ * v),
						(u8)(active_b_ * v)
					);
				} else {
					write_color(0, 0, 0);
				}
			}
		} else {
			for (auto i = 0; i < led_count_; i ++) {
				write_color(active_r_, active_g_, active_b_);
			}
		}

		counter_ += 1; //frame_period_;
	}
}
