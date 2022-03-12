/*
 * NotCopyable.h
 *
 * Created: 17/08/2021 10:22:47 am
 *  Author: smohekey
 */ 

#pragma once

namespace swordfish::utils {
	class NotCopyable {
	protected:
		NotCopyable() = default;
		~NotCopyable() = default;
		
	private:
		NotCopyable(const NotCopyable&) = delete;
		NotCopyable& operator=(const NotCopyable&) = delete;
	};
}
