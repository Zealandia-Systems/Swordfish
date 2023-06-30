/*
 * EmergencyStop.h
 *
 * Created: 16/09/2021 8:35:07 am
 *  Author: smohekey
 */

#pragma once

#include <swordfish/Module.h>
#include <swordfish/core/ISR.h>
#include <swordfish/utils/TypeInfo.h>

#include "EStopException.h"

namespace swordfish::estop {
	class EStopModule : public swordfish::Module {
	private:
		static EStopModule* __instance;
		void handleEStop();

		EStopModule(core::Object* parent);

	protected:
		static core::Schema __schema;

		core::Pack _pack;

		core::Pack& getPack() override {
			return _pack;
		}

	private:
		core::ISR _estopISR;
		bool _triggered;

		bool readPin() {
			return READ(ESTOP_PIN) != ESTOP_ENDSTOP_INVERTING;
		}

	public:
		virtual ~EStopModule() {
		}

		virtual const char* name() override {
			return "Emergency Stop";
		}
		virtual void init() override;

		bool isTriggered() {
			return _triggered;
		}

		bool checkOrClear();
		void throwIfTriggered();

		static EStopModule& getInstance(core::Object* parent = nullptr);
	};
} // namespace swordfish::estop