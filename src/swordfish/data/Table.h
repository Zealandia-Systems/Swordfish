/*
 * Table.h
 *
 * Created: 15/09/2021 11:48:04 am
 *  Author: smohekey
 */ 

#pragma once

#include <charconv>
#include <string_view>

#include <swordfish/debug.h>
#include <swordfish/io/Writer.h>
#include <swordfish/core/Object.h>
#include <swordfish/core/ObjectList.h>
#include <swordfish/core/FormatException.h>

#include "Record.h"

namespace swordfish::io {
	class Writer;
}

namespace swordfish::data {
	struct Pagination {
		int16_t page;
		int16_t pageLength;
		
		Pagination(int16_t page = -1, int16_t pageLength = -1) : page(page), pageLength(pageLength) {
			
		}
	};
	
	class ITable {
	public:
		virtual ~ITable();
		virtual Record& createChild() = 0;
		virtual Record* getChild(int16_t index) = 0;
		virtual void remove(Record& child) = 0;
		virtual void writeJson(io::Writer& out, Pagination pagination) = 0;
	};
	
	template<typename T>
	class Table : public core::ObjectList<T>, public ITable {
	protected:
		Table(core::Object* parent) : core::ObjectList<T>(parent) { }
			
	public:
		virtual const char* getName() = 0;
		
		virtual ITable* asTable() override { return static_cast<ITable*>(this); }
			
		virtual T* get(int16_t index) {
			for(auto& record : *this) {
				if(record.getIndex() == index) {
					return &record;
				}
			}
			
			return nullptr;
		}
		
		virtual T* getChild(int16_t index) override {
			int16_t i = 0;
			
			for(auto& record : *this) {
				if(i++ == index) {
					return &record;
				}
			}
			
			return nullptr;
		}
		
		virtual void writeJson(io::Writer& out) override {
			const char* separator = "";
			
			out << "{\"table\":{\"name\":\"" << getName() << "\",\"records\":[";
			
			for(auto& record : *this) {
				out << separator;
				
				record.writeJson(out);
				
				separator = ",";
			}
			
			out << "]}}";
		}
		
		virtual void writeJson(io::Writer& out, Pagination pagination) override {
			if(pagination.page == -1 && pagination.pageLength == -1) {
				writeJson(out);
				
				return;
			}

			const auto page = pagination.page == -1 ? 0 : pagination.page;
			const auto pageLength = pagination.pageLength == -1 ? 10 : pagination.pageLength;
			
			auto separator = "";
			auto i = 0;
			auto startOffset = page * pageLength;
			auto stopOffset = startOffset + pageLength;
			
			out << "{\"table\":{\"name\":\"" << getName() << "\",\"page\":" << page << ",\"pageLength\":" << pageLength << ",\"records\":[";
			
			for(auto& record : *this) {
				if(i >= startOffset && i < stopOffset) {
					out << separator;
					
					record.writeJson(out);
					
					separator = ",";
				}
				
				i++;
			}
			
			out << "]}}";
		}
		
		virtual void writeRecordJson(io::Writer& out, T& record) {
			out << "{\"table\":{\"name\":\"" << getName() << "\",\"record\":";
			
			record.writeJson(out);
			
			out << "}}";
		}
	
		virtual void remove(Record& record) override {
			core::ObjectList<T>::remove(static_cast<T&>(record));
		}
		
		virtual Record* getChild(std::string_view name) override {
			debug()(name);
			
			int16_t index = -1;
			
			auto [ptr, ec] = std::from_chars(name.data(), name.data() + name.size(), index);
			
			if(ec != std::errc()) {
				return nullptr;
			}
			
			return get(index);
		}
		
		virtual Record& createChild() override {
			return static_cast<Record&>(core::ObjectList<T>::emplaceBack());
		}
	};
}