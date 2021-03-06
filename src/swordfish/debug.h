/*
 * Trace.h
 *
 * Created: 23/08/2021 12:36:05 pm
 *  Author: smohekey
 */ 

#pragma once

#include <experimental/source_location>

#include <swordfish/io/Writer.h>

namespace swordfish {
	#ifdef DEBUG
		extern io::Writer out;
		
		struct debug {
			const std::experimental::source_location& _location;
			
			debug(const std::experimental::source_location& location = std::experimental::source_location::current()) : _location(location) {
				
			}
			
			template<typename... TArgs>
			void operator()(TArgs&&... args) {
				out << _location.file_name() << ":(" << _location.line() << ") [" << _location.function_name() << "] ";
				
				((out << std::forward<TArgs>(args)), ...);
				
				out << '\n';
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
}