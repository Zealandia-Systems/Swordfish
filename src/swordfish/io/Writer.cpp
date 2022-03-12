/*
 * Writer.cpp
 *
 * Created: 15/08/2021 9:16:00 pm
 *  Author: smohekey
 */ 

#include <cstdio>
#include <cstring>

#include <charconv>
#include <string>

#include "Writer.h"

namespace swordfish::io {
	char nl = '\n';
	
	Writer& Writer::operator <<(bool value) {
		const char* buffer = value ? "true" : "false";
		const size_t length = value ? 4 : 5;
		
		_stream.write((const void*)buffer, length);
		
		return *this;
	}
	
	Writer& Writer::operator <<(const char value) {
		_stream.write(&value, 1);
		
		return *this;
	}
	
	Writer& Writer::operator<<(int value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
		} else {
			*this << "Error";
		}
			
		return *this;
	}
	
	Writer& Writer::operator<<(int8_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}

	Writer& Writer::operator<<(int16_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}
	
	Writer& Writer::operator<<(int32_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}
	
	Writer& Writer::operator<<(int64_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}
	
	Writer& Writer::operator<<(unsigned int value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}	
	
	Writer& Writer::operator<<(uint8_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}

	Writer& Writer::operator<<(uint16_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}
	
	Writer& Writer::operator<<(uint32_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}
	
	Writer& Writer::operator<<(uint64_t value) {
		char buffer[20];
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, 10);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
			} else {
			*this << "Error";
		}
		
		return *this;
	}
	
	Writer& Writer::operator<<(float32_t value) {
		auto str = std::to_string(value);
		
		_stream.write(str.data(), str.length());
		
		return *this;
	}
	
	Writer& Writer::operator<<(float64_t value) {
		auto str = std::to_string(value);
		
		_stream.write(str.data(), str.length());
		
		return *this;
	}
	
	Writer& Writer::operator<<(const char* value) {
		size_t length = strlen(value);
		
		_stream.write(value, length);
		
		return *this;
	}
	
	Writer& Writer::operator<<(const std::string_view value) {
		_stream.write(value.data(), value.size());
		
		return *this;
	}
	
	Writer& Writer::operator<<(const void* value) {
		char buffer[20];
		
		*this << "0x";
		
		auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), (uint64_t)value, 16);
		
		if(ec == std::errc())	{
			_stream.write(buffer, ptr - buffer);
		} else {
			*this << "Error";
		}
		
		return *this;
	}
}