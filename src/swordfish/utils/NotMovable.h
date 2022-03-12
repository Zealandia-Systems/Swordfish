/*
 * NotMovable.h
 *
 * Created: 17/08/2021 10:22:06 am
 *  Author: smohekey
 */ 

#pragma once

namespace swordfish::utils {
	class NotMovable {
	public:
		NotMovable() = default;
		~NotMovable() = default;
		
		NotMovable(NotMovable&&) = delete;
		NotMovable& operator=(NotMovable&&) = delete;
	};
}