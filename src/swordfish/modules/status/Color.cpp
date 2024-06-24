#include <algorithm>
#include "Color.h"
#include <cmath>

namespace swordfish::status {
	Hsl Rgb::to_hsl() {
		float32_t r = this->r;
		float32_t g = this->g;
		float32_t b = this->b;

		float32_t min = std::min(r, std::min(g, b));
		float32_t max = std::max(r, std::max(g, b));
		float32_t delta = max - min;

		float32_t h = 0.0;
		float32_t s = 0.0;
		float32_t l = (max + min) / 2;

		if (delta == 0) {
			h = s = 0;
		}
		else {
			if (l < 0.5) {
				s = delta / (max + min) * 100;
			}
			else {
				s = delta / (1 - std::abs(2 * l - 1)) * 100;
			}

			if (r == max) {
				h =  (g - b) / delta;
			}
			else if (g == max) {
				h = (b - r) / delta + 2;
			}
			else if (b == max) {
				h = (r - g) / delta + 4;
			}
			h = fmod(60 * h + 360, 360);
		}
		l *= 100;

		return Hsl { h, s, l };
	}

	Hsv Rgb::to_hsv() {
		float32_t r = this->r;
		float32_t g = this->g;
		float32_t b = this->b;

		float32_t min = std::min(r, std::min(g, b));
		float32_t max = std::max(r, std::max(g, b));
		float32_t delta = max - min;

		float32_t h = 0.0;
		float32_t s = (max > 1e-3) ? (delta / max) : 0;
		float32_t v = max;

		if (delta == 0) {
			h = 0;
		}	else {
			if (r == max) {
				h = (g - b) / delta;
			}
			else if (g == max) {
				h = 2 + (b - r) / delta;
			}
			else if (b == max) {
				h = 4 + (r - g) / delta;
			}

			h *= 60;
			h = std::fmod(h + 360, 360);
		}

		return Hsv {
			h, s, v
		};
	}

	uint32_t Rgb::to_packed() {
		const auto [r, g, b] = to_rgb_u8();

		return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
	}

	std::tuple<uint8_t, uint8_t, uint8_t> Rgb::to_rgb_u8() {
		return {
			(uint8_t)(std::clamp(r, 0.0f, 1.0f) * 255.0),
			(uint8_t)(std::clamp(g, 0.0f, 1.0f) * 255.0),
			(uint8_t)(std::clamp(b, 0.0f, 1.0f) * 255.0)
		};
	}

	Rgb Rgb::from_rgb_u8(uint8_t r, uint8_t g, uint8_t b) {
		return Rgb {
			r: r / 255.0f,
			g: g / 255.0f,
			b: b / 255.0f
		};
	}

	float32_t hue_to_rgb(float32_t v1, float32_t v2, float32_t vh) {
		if (vh < 0) vh += 1;
		if (vh > 1) vh -= 1;

		if (6 * vh < 1) {
			return v1 + (v2 - v1) * 6 * vh;
		}

		if (2 * vh < 1) {
			return v2;
		}

		if (3 * vh < 2) {
			return v1 + (v2 - v1)*(2.0 / 3.0 - vh) * 6;
		}

		return v1;
	}

	Rgb Hsl::to_rgb() {
		float32_t h = this->h / 360;
		float32_t s = this->s / 100;
		float32_t l = this->l / 100;

		float32_t r = 0;
		float32_t g = 0;
		float32_t b = 0;

		if (s == 0) {
			r = g = b = l;
		} else {
			float32_t temp1, temp2;

			temp2 = (l < 0.5) ? (l*(1 + s)) : (l + s - (s*l));
			temp1 = 2 * l - temp2;

			r = hue_to_rgb(temp1, temp2, h + 1.0 / 3.0);
			g = hue_to_rgb(temp1, temp2, h);
			b = hue_to_rgb(temp1, temp2, h - 1.0 / 3.0);
		}

		return Rgb { r, g, b };
	}

	Rgb Hsv::to_rgb() {
		int range = (int)floor(this->h / 60);
		float32_t c = this->v * this->s;
		float32_t x = c * (1 - std::abs(std::fmod(this->h / 60, 2) - 1));
		float32_t m = this->v - c;

		float32_t r, g, b;

		switch (range) {
			case 0: {
				r = (c + m);
				g = (x + m);
				b = m;

				break;
			}

			case 1: {
				r = (x + m);
				g = (c + m);
				b = m;

				break;
			}

			case 2: {
				r = m;
				g = (c + m);
				b = (x + m);

				break;
			}

			case 3: {
				r = m;
				g = (x + m);
				b = (c + m);

				break;
			}

			case 4: {
				r = (x + m);
				g = m;
				b = (c + m);

				break;
			}

			default: {
				r = (c + m);
				g = m;
				b = (x + m);

				break;
			}
		}

		return Rgb { r, g, b };
	}
}