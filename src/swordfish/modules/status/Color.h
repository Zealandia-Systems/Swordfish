#pragma once

#include <tuple>
#include <swordfish/types.h>

namespace swordfish::status {
	class Hsl;
	class Hsv;

	class Rgb {
	public:
		float32_t r;
		float32_t g;
		float32_t b;

		Hsl to_hsl();
		Hsv to_hsv();

		uint32_t to_packed();

		std::tuple<uint8_t, uint8_t, uint8_t> to_rgb_u8();

		static Rgb from_rgb_u8(uint8_t r, uint8_t g, uint8_t b);
	};

	class Hsl {
	public:
		float32_t h;
		float32_t s;
		float32_t l;

		Rgb to_rgb();
	};

	class Hsv {
	public:
		float32_t h;
		float32_t s;
		float32_t v;

		Rgb to_rgb();
	};
}