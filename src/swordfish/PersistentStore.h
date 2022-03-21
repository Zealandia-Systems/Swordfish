#pragma once

#include <swordfish/io/Seekable.h>
#include <swordfish/io/InputStream.h>
#include <swordfish/io/OutputStream.h>
#include <swordfish/io/WrappingInputStream.h>
#include <swordfish/io/WrappingOutputStream.h>

#include <marlin/HAL/shared/eeprom_api.h>

namespace swordfish {
	static constexpr uint32_t CONFIG_START = 0x2000;
	static constexpr uint32_t CONFIG_END = 0x7A1200; // 8MB (GD25Q64)

	class PersistentStoreSeekable : public io::Seekable {
	protected:
		offset_t _offset;

	public:
		PersistentStoreSeekable(offset_t offset = 0) : _offset(offset) {

		}

		offset_t seek(const offset_t offset, const io::Origin origin) override {
			switch(origin) {
				case io::Origin::Start: {
					_offset = offset;

					break;
				}

				case io::Origin::Current: {
					_offset += offset;

					break;
				}

				case io::Origin::End: {
					_offset = CONFIG_END - offset;

					break;
				}
			}

			if(_offset > CONFIG_END) {
				_offset = CONFIG_END - 1;
			}

			return _offset;
		}
	};

	class PersistentStoreInputStream : public io::SeekableInputStream, protected PersistentStoreSeekable {
	protected:
		uint16_t _crc;

	public:
		PersistentStoreInputStream(int offset = 0) : PersistentStoreSeekable(offset), _crc(0) {
			persistentStore.access_start();
		}

		~PersistentStoreInputStream() {
			persistentStore.access_finish();
		}

		size_t read(void* buffer, size_t length) override {
			persistentStore.read_data(_offset, (uintptr_t)buffer, length, &_crc);

			return length;
		}

		offset_t seek(const offset_t offset, const io::Origin origin) override {
			return PersistentStoreSeekable::seek(offset, origin);
		}

		uint16_t getCRC() {
			return _crc;
		}

		void resetCRC() {
			_crc = 0;
		}
	};

	class PersistentStoreOutputStream : public io::SeekableOutputStream, protected PersistentStoreSeekable {
	protected:
		uint16_t _crc;

	public:
		PersistentStoreOutputStream(int offset = 0) : PersistentStoreSeekable(offset), _crc(0) {
			persistentStore.access_start();
		}

		~PersistentStoreOutputStream() {
			persistentStore.access_finish();
		}

		size_t write(const void* buffer, size_t length) override {
			persistentStore.write_data(_offset, (uintptr_t)buffer, length, &_crc);

			return length;
		}

		offset_t seek(const offset_t offset, const io::Origin origin) override {
			return PersistentStoreSeekable::seek(offset, origin);
		}

		uint16_t getCRC() {
			return _crc;
		}

		void resetCRC() {
			_crc = 0;
		}
	};
}
