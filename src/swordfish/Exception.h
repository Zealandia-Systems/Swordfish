/*
 * Exception.h
 *
 * Created: 30/09/2021 1:06:47 pm
 *  Author: smohekey
 */

#pragma once

namespace swordfish {
	namespace io {
		class Writer;
	}

	class Exception {
	protected:
		virtual ~Exception() { }

		virtual void writeType(io::Writer& writer) const = 0;
		virtual void writeMessage(io::Writer& writer) const = 0;
	public:
		virtual void writeJson(io::Writer& writer) const;
	};

}