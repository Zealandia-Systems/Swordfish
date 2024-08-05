#pragma once

#include <swordfish/types.h>

namespace swordfish::motion {
	enum class FeedRateType {
		InverseTime,
		UnitsPerSecond,
	};

	struct FeedRate {
	private:
		FeedRateType type_;
		f32 value_;

		constexpr FeedRate(FeedRateType type, f32 value) :
				type_(type), value_(value) {
		}

	public:
		static constexpr FeedRate InverseTime(f32 value) {
			return FeedRate(FeedRateType::InverseTime, value);
		}

		static constexpr FeedRate UnitsPerSecond(f32 value) {
			return FeedRate(FeedRateType::UnitsPerSecond, value);
		}

		const FeedRateType type() const {
			return type_;
		}

		f32& value() {
			return value_;
		}

		const f32 value() const {
			return value_;
		}

		/*FeedRate& operator=(FeedRate& other) {
		  type_ = other.type_;
		  value_ = other.value_;

		  return *this;
		}*/

		FeedRate& operator=(FeedRate other) {
			type_ = other.type_;
			value_ = other.value_;

			return *this;
		}

		FeedRate operator*(f32 value) {
			return FeedRate(type_, value_ * value);
		}

		FeedRate operator/(f32 value) {
			return FeedRate(type_, value_ / value);
		}
	};
} // namespace swordfish::motion
