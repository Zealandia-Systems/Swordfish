/*
 * Trace.h
 *
 * Created: 23/08/2021 12:36:05 pm
 *  Author: smohekey
 */

#pragma once

#include <experimental/source_location>

#include <marlin/MarlinCore.h>

#include <swordfish/io/Writer.h>

namespace swordfish {
#ifdef DEBUG
	extern io::Writer out;

	struct debug {
		const std::experimental::source_location& _location;

		debug(const std::experimental::source_location& location = std::experimental::source_location::current()) :
				_location(location) {
		}

		template<typename... TArgs>
		void operator()(TArgs&&... args) {
			auto time = micros();

			out << _location.file_name() << ":(" << _location.line() << ") " << time << " [" << _location.function_name() << "] ";

			((out << std::forward<TArgs>(args)), ...);

			out << '\n';

			if (DEBUGGING(SWORDFISH)) {
				SERIAL_ECHO(_location.file_name());
				SERIAL_ECHO(":(");
				SERIAL_ECHO(_location.line());
				SERIAL_ECHO(") [");
				SERIAL_ECHO(_location.function_name());
				SERIAL_ECHO("] ");

				(MYSERIAL0.print(args), ...);

				SERIAL_ECHO("\n");
			}
		}
	};
#else
	struct debug {
		debug() {
		}

		template<typename... TArgs>
		void operator()([[maybe_unused]] TArgs&&... args) {
		}
	};
#endif
} // namespace swordfish