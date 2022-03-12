/*
 * Reader.h
 *
 * Created: 5/11/2021 3:50:17 pm
 *  Author: smohekey
 */ 

#pragma once

#include <cstring>
#include <charconv>
#include <functional>
#include <string_view>
#include <variant>

#include <swordfish/debug.h>
#include <swordfish/types.h>
#include <swordfish/core/FormatException.h>

namespace swordfish::json {
	template<typename TObject>
	class Reader {		
	protected:
		virtual TObject* onStartObject([[maybe_unused]] TObject* parent) { return nullptr; }
		virtual void onPropertyName([[maybe_unused]] TObject* object, [[maybe_unused]] std::string_view name) { }
		virtual void onEndObject([[maybe_unused]] TObject* parent, [[maybe_unused]] TObject* object) { }
		virtual TObject* onStartArray([[maybe_unused]] TObject* parent) { return nullptr; }
		virtual void onArrayIndex([[maybe_unused]] TObject* array, [[maybe_unused]] int32_t index) { }
		virtual void onEndArray([[maybe_unused]] TObject* parent, [[maybe_unused]] TObject* array) { }
		virtual void onValue([[maybe_unused]] TObject* parent, [[maybe_unused]] std::string_view value) { }
		
	public:
		void read(std::string_view buffer) {
			int16_t index = -1;
			
			auto next = [&]() -> int16_t {
				if((uint32_t)(++index) == buffer.length()) {
					throw core::FormatException { "unexpected end of input" };
				}
				
				debug()(index, ": ", (char)buffer[index]);
				
				return buffer[index];
			};
			
			auto peek = [&]() -> int16_t {
				uint32_t peekIndex = index + 1;
				
				return peekIndex < buffer.length() ? buffer[peekIndex] : -1;
			};
			
			auto expect = [&](char e) {
				auto c = next();
				
				debug()("Expecting: ", e, " found: ", (char)c);
				
				if(c != e) {
					throw core::FormatException { "unexpected character" };
				}
			};
			
			std::function<void(TObject*)> readValue = nullptr;
			
			auto unescapeString = [&](const std::string_view string, std::function<void(std::string_view)> handleResult) {
				char buf[string.size()];
				
				uint16_t length = 0;
				
				for(auto i = 0u; i < string.size(); i++) {
					if(string[i] != '%') {
						debug()("adding: ", string[i]);
						
						buf[length++] = string[i];
					} else {
						uint8_t c = 0;
						
						auto [ptr, ec] = std::from_chars(string.data() + i + 1, string.data() + i + 3, c, 16);
						
						if(ec != std::errc()) {
							throw core::FormatException { "ill formated string" };
						}
						
						debug()("adding: '", (char)c, "' (", c, ")");
						
						buf[length++] = (char)c;
						
						i += 2;
					}
				}
				
				auto view = std::string_view { buf, length };
					
				handleResult(view);
			};
			
			auto readString = [&](std::function<void(std::string_view)> handleResult) {
				debug()("readString");
				
				expect('"');
				
				auto c = next();
				
				auto start = index;
				
				while(true) {
					if(c == '"') {
						handleResult(buffer.substr(start, index - start));
						
						return;
					}
					
					c = next();
				}
			};
			
			auto readNumber = [&](TObject* parent) {
				auto start = index + 1;
				char c;
				
				debug()("readNumber");
				
				if(peek() == '-') {
					next();
				}
				
				while(isdigit(c = peek())) {
					next();
				}
				
				if(c != '.') {
					onValue(parent, buffer.substr(start, index - start + 1));
					
					return;
				}
				
				next();
				
				while(isdigit(peek())) {
					next();
				}
				
				onValue(parent, buffer.substr(start, index - start + 1));
			};
			
			auto readObject = [&](TObject* parent) {
				debug()("readObject");
				
				expect('{');
				
				auto* object = onStartObject(parent);
				
				while(true) {
					readString([&](std::string_view view) {
						unescapeString(view, [&](std::string_view view) {
							onPropertyName(object, view);
							
							expect(':');
					
							readValue(object);
						});
					});
					
					auto c = next();
					
					if(c == ',') {
						continue;
					} else if(c == '}') {
						onEndObject(parent, object);
						
						return object;
					} else {
						throw core::FormatException { "ill formatted object, expected ','" };
					}
				}
			};
			
			auto readArray = [&](TObject* parent) {
				debug()("readArray");
				
				expect('[');
				
				auto* array = onStartArray(parent);
				
				int32_t index = 0;
				
				while(true) {
					onArrayIndex(array, index++);
					
					readValue(array);
					
					auto c = next();
					
					if(c == ',') {
						continue;
					} else if(c == ']') {
						onEndArray(parent, array);
						
						return array;
					} else {
						throw core::FormatException { "ill formatted array, expected ',' or ']'" };
					}
				}
			};
			
			readValue = [&](TObject* parent) {
				debug()("readValue");
				
				auto c = peek();
				
				switch(c) {
					case '{': {
						readObject(parent);
						
						return;
					}
					
					case '[': {
						readArray(parent);
						
						return;
					}
					
					case '"': {
						readString([&](std::string_view view) {
							unescapeString(view, [&](std::string_view view) {
								onValue(parent, view);
							});
						});
						
						return;
					}
					
					case -1: {
						throw core::FormatException { "Unexpected end of input." };
					}
					
					case 't': {
						debug()("readTrue");
						
						auto start = index + 1;
						
						expect('t');
						expect('r');
						expect('u');
						expect('e');
						
						onValue(parent, buffer.substr(start, 4));
							
						return;
					}
					
					case 'f': {
						debug()("readFalse");
						
						auto start = index + 1;
						
						expect('f');
						expect('a');
						expect('l');
						expect('s');
						expect('e');
						
						onValue(parent, buffer.substr(start, 5));
						
						return;
					}
					
					default: {
						if(isdigit(c) || c == '-') {
							readNumber(parent);
							
							return;
						} else {
							debug()("unexpected: ", (char)c);
							
							throw core::FormatException { "Unexpected character" };
						}
					}
				}
			};
			
			readValue(nullptr);
		}
	};
}