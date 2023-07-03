/*
 * NotHomedException.h
 *
 *  Author: smohekey
 */

#pragma once

#include <swordfish/Exception.h>

namespace swordfish::motion {
	class NotHomedException : public Exception {
	protected:
		virtual void writeType(io::Writer& writer) const override;
		virtual void writeMessage(io::Writer& writer) const override;

	public:
		virtual ~NotHomedException() {
		}
	};
} // namespace swordfish::motion
