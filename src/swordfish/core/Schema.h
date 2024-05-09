/*
 * Field.h
 *
 * Created: 31/08/2021 8:30:37 am
 *  Author: smohekey
 */

#pragma once

#include <cstring>
#include <cstdlib>

#include <charconv>
#include <functional>
#include <type_traits>

#include <swordfish/debug.h>
#include <swordfish/io/Writer.h>
#include <swordfish/utils/TypeInfo.h>

#include "Object.h"
#include "Pack.h"
#include "FormatException.h"
#include "DuplicateIndexException.h"

#include <marlin/gcode/parser.h>

namespace swordfish::core {
	namespace _ {
		template<typename T>
		struct __attribute__((packed)) AlignmentSafe {
			T value_;
		};

		template<typename T>
		void convertSet(std::string_view value, std::function<void(T)> set) {
			if constexpr (std::is_integral<T>::value) {
				int64_t result = 0;

				auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);

				debug()("result: ", result);

				if (ec != std::errc()) {
					throw FormatException { "Malformed number" };
				} else {
					set(result);
				}
			} else if constexpr (std::is_same<T, float32_t>::value) {
				char buffer[value.size() + 1];

				memcpy(buffer, value.data(), value.size());

				buffer[value.size()] = 0;

				char* end;

				auto result = strtof(buffer, &end);

				debug()("result: ", result);

				if (buffer == end) {
					throw FormatException { "Malformed number" };
				} else {
					set(result);
				}
			} else {
				static_assert("Not supported.");
			}
		}
	} // namespace _

	enum class FieldType {
		Value,
		Object,
		Transient
	};

	class Field {
	private:
		const char* _name;

	protected:
		Field(const char* name) :
				_name(name) {
		}

	public:
		const char* name() const {
			return _name;
		}

		virtual FieldType type() const = 0;
		virtual void writeJson(io::Writer& out, Object& object, Pack& pack) = 0;
	};

	class ValueFieldBase : public Field {
	protected:
		ValueFieldBase(const char* name) :
				Field(name) {
		}

	public:
		FieldType type() const override {
			return FieldType::Value;
		}

		virtual void init(Pack& pack) = 0;

		virtual void readJson(Pack& pack, std::string_view value) = 0;
	};

	template<
			typename T>
	class ValueField : public ValueFieldBase {
	protected:
		const uint32_t _byteOffset;
		const T _defaultValue;

	public:
		ValueField(const char* name, uint32_t byteOffset, T defaultValue) :
				ValueFieldBase(name), _byteOffset(byteOffset), _defaultValue(defaultValue) {
		}

		uint32_t byteOffset() const {
			return _byteOffset;
		}

		T defaultValue() const {
			return _defaultValue;
		}

		virtual void init(Pack& pack) override {
			set(pack, _defaultValue);
		}

		T get(Pack& pack) {
			if (pack._values.size() < _byteOffset + sizeof(T)) {
				return _defaultValue;
			}

			const auto* ptr = pack._values.data() + _byteOffset;

			const _::AlignmentSafe<T>* safe = reinterpret_cast<const _::AlignmentSafe<T>*>(ptr);

			return safe->value_;
		}

		virtual void set(Pack& pack, const T value) {
			uint32_t size = _byteOffset + sizeof(T);

			while (pack._values.size() < size) {
				pack._values.emplace_back();
			}

			auto* ptr = pack._values.data() + _byteOffset;

			_::AlignmentSafe<T>* safe = reinterpret_cast<_::AlignmentSafe<T>*>(ptr);

			safe->value_ = value;
		}

		/*virtual void set(Pack& pack, std::string_view value) {
		  _::convertSet<T>(value, [&](T value) {
		    set(pack, value);
		  });
		}*/

		virtual void readJson(Pack& pack, std::string_view value) {
			_::convertSet<T>(value, [&](T value) {
				set(pack, value);
			});
		}

		virtual void writeJson(io::Writer& out, [[maybe_unused]] Object& object, Pack& pack) {
			out << get(pack);
		}
	};

	template<typename T>
	class LinearValueField : public ValueField<T> {
	public:
		LinearValueField(const char* name, uint32_t byteOffset, T defaultValue) :
				ValueField<T>::ValueField(name, byteOffset, defaultValue) {
		}

		virtual void readJson(Pack& pack, std::string_view value) {
			_::convertSet<T>(value, [&](T value) {
				this->set(pack, value * parser.linear_unit_factor);
			});
		}

		virtual void writeJson(io::Writer& out, [[maybe_unused]] Object& object, Pack& pack) {
			out << ValueField<T>::get(pack) / parser.linear_unit_factor;
		}
	};

	template<typename T>
	class ValidatedValueField : public ValueField<T> {
	private:
		void (*_validate)(T, T);

	public:
		ValidatedValueField(const char* name, uint32_t byteOffset, T defaultValue, void (*validate)(T, T)) :
				ValueField<T>(name, byteOffset, defaultValue), _validate(validate) {
		}

		virtual void init(Pack& pack) override {
			ValueField<T>::set(pack, ValueField<T>::_defaultValue);
		}

		virtual void set(Pack& pack, const T value) override {
			if (_validate) {
				_validate(this->get(pack), value);
			}

			ValueField<T>::set(pack, value);
		}
	};

	template<>
	class ValueField<bool> : public ValueFieldBase {
	private:
		const uint32_t _bitOffset;
		const bool _defaultValue;

	public:
		ValueField(const char* name, uint32_t bitOffset, bool defaultValue) :
				ValueFieldBase(name), _bitOffset(bitOffset), _defaultValue(defaultValue) {
		}
		uint32_t bitOffset() const {
			return _bitOffset;
		}

		bool defaultValue() const {
			return _defaultValue;
		}

		virtual void init(Pack& pack) override {
			set(pack, _defaultValue);
		}

		bool get(Pack& pack) const {
			uint32_t byteOffset = _bitOffset >> 3;
			uint8_t bit = _bitOffset % 8;

			if (pack._values.size() < byteOffset) {
				return _defaultValue;
			}

			return (pack._values[byteOffset] & (1 << bit));
		}

		void set(Pack& pack, const bool value) const {
			uint32_t byteOffset = _bitOffset >> 3;
			uint32_t size = byteOffset + 1;
			uint8_t bit = _bitOffset % 8;

			while (pack._values.size() < size) {
				pack._values.emplace_back();
			}

			pack._values[byteOffset] = ((pack._values[byteOffset] & ~(1 << bit)) | (value << bit));
		}

		void readJson(Pack& pack, std::string_view value) {
			if (strncmp("true", value.data(), value.size()) == 0) {
				set(pack, true);
			} else {
				set(pack, false);
			}
		}

		void writeJson(io::Writer& out, [[maybe_unused]] Object& object, Pack& pack) {
			out << get(pack);
		}
	};

	class ObjectFieldBase : public Field {
	protected:
		const uint32_t _index;

		ObjectFieldBase(const char* name, const uint32_t index) :
				Field(name), _index(index) {
		}

	public:
		FieldType type() const override {
			return FieldType::Object;
		}

		uint32_t index() const {
			return _index;
		}

		Object& get(Pack& pack) {
			return pack.getChild(_index);
		}

		virtual Object* create(Object* object) = 0;
	};

	template<
			typename T>
	class ObjectField : public ObjectFieldBase {
	private:
		static T* createT(Object* parent) {
			return _::create<T>(parent);
		}

		std::function<T*(Object*)> _creator;

	public:
		ObjectField(const char* name, const uint32_t index, std::function<T*(Object*)> creator = createT) :
				ObjectFieldBase(name, index), _creator(creator) {
		}

		T& get(Pack& pack) {
			return static_cast<T&>(pack.getChild(_index));
		}

		void writeJson([[maybe_unused]] io::Writer& out, [[maybe_unused]] Object& object, Pack& pack) override {
			get(pack).writeJson(out);
		}

		Object* create(Object* parent) override {
			return _creator(parent);
		}
	};

	template<
			typename T>
	class StaticObjectField : public ObjectFieldBase {
		static_assert(!std::is_constructible<T>());

	private:
		T* _object;

	public:
		StaticObjectField(const char* name, const uint32_t index, T* object) :
				ObjectFieldBase(name, index), _object(object) {
		}

		T& get(Pack& pack) {
			auto& child = pack.getChild(_index);

			return static_cast<T&>(child);
		}

		void writeJson([[maybe_unused]] io::Writer& out, [[maybe_unused]] Object& object, Pack& pack) override {
			get(pack).writeJson(out);
		}

		Object* create([[maybe_unused]] Object& parent) override {
			return _object;
		}
	};

	class TransientFieldBase : public Field {
	public:
		TransientFieldBase(const char* name) :
				Field(name) {
		}

		FieldType type() const override {
			return FieldType::Transient;
		}

		virtual void set(Pack& pack, std::string_view value) = 0;
	};

	template<
			typename TObject,
			typename TValue>
	class TransientField : public TransientFieldBase {
	private:
		std::function<TValue(TObject&)> _getter;
		std::function<void(TObject&, TValue)> _setter;

	public:
		TransientField(const char* name, std::function<TValue(TObject&)> getter, std::function<void(TObject&, TValue)> setter) :
				TransientFieldBase(name), _getter(getter), _setter(setter) {
		}

		virtual void set(Pack& pack, std::string_view value) override {
			_::convertSet<TValue>(value, [&](TValue value) {
				auto& typedObject = static_cast<TObject&>(pack.getObject());

				_setter(typedObject, value);
			});
		}

		void writeJson(io::Writer& out, Object& object, [[maybe_unused]] Pack& pack) override {
			auto& typedObject = static_cast<TObject&>(object);

			out << _getter(typedObject);
		}
	};

	template<typename TObject>
	class TransientField<TObject, const char*> : public TransientFieldBase {
	private:
		std::function<const char*(TObject&)> _getter;

	public:
		TransientField(const char* name, std::function<const char*(TObject&)> getter) :
				TransientFieldBase(name), _getter(getter) {
		}

		void writeJson(io::Writer& out, Object& object, [[maybe_unused]] Pack& pack) {
			auto& typedObject = static_cast<TObject&>(object);

			out << '"' << _getter(typedObject) << '"';
		}
	};

	class Schema {
	private:
		const std::string_view _name;
		const Schema* _parent;
		std::vector<std::reference_wrapper<ValueFieldBase>> _valueFields;
		std::vector<std::reference_wrapper<ObjectFieldBase>> _objectFields;
		std::vector<std::reference_wrapper<TransientFieldBase>> _transientFields;

	public:
		Schema(
				const std::string_view& name,
				const Schema* parent,
				std::initializer_list<std::reference_wrapper<ValueFieldBase>> valueFields,
				std::initializer_list<std::reference_wrapper<ObjectFieldBase>> objectFields) :
				_name(name),
				_parent(parent),
				_valueFields(valueFields),
				_objectFields(objectFields) {
		}

		Schema(
				const std::string_view& name,
				const Schema* parent,
				std::initializer_list<std::reference_wrapper<ValueFieldBase>> valueFields,
				std::initializer_list<std::reference_wrapper<ObjectFieldBase>> objectFields,
				std::initializer_list<std::reference_wrapper<TransientFieldBase>> transientFields) :
				_name(name),
				_parent(parent),
				_valueFields(valueFields),
				_objectFields(objectFields),
				_transientFields(transientFields) {
		}

		inline const std::string_view& name() const {
			return _name;
		}

		inline const Schema* parent() const {
			return _parent;
		}

		inline const std::vector<std::reference_wrapper<ValueFieldBase>>& valueFields() const {
			return _valueFields;
		}

		inline const std::vector<std::reference_wrapper<ObjectFieldBase>>& objectFields() const {
			return _objectFields;
		}

		inline const std::vector<std::reference_wrapper<TransientFieldBase>>& transientFields() const {
			return _transientFields;
		}
	};
} // namespace swordfish::core