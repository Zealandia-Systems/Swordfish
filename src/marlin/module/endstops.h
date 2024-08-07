/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * endstops.h - manages endstops
 */

#include "../inc/MarlinConfig.h"
#include <stdint.h>

#include <swordfish/types.h>

#define X_ENDSTOP			(X_HOME_DIR < 0 ? EndstopValue::X_MIN : EndstopValue::X_MAX)
#define X2_ENDSTOP		(X_HOME_DIR < 0 ? EndstopValue::X2_MIN : EndstopValue::X2_MAX)
#define Y_ENDSTOP			(Y_HOME_DIR < 0 ? EndstopValue::Y_MIN : EndstopValue::Y_MAX)
#define Y2_ENDSTOP		(Y_HOME_DIR < 0 ? EndstopValue::Y2_MIN : EndstopValue::Y2_MAX)
#define Z_ENDSTOP			(Z_HOME_DIR < 0 ? EndstopValue::Z_MIN, EndstopValue::Z_MAX)
#define A_ENDSTOP EndstopValue::A_MAX

#define WORK_ENDSTOP	WORK_PROBE

class Endstops {
  public:
    #if HAS_EXTRA_ENDSTOPS
      typedef uint32_t esbits_t;
      TERN_(X_DUAL_ENDSTOPS, static float x2_endstop_adj);
      TERN_(Y_DUAL_ENDSTOPS, static float y2_endstop_adj);
      TERN_(Z_MULTI_ENDSTOPS, static float z2_endstop_adj);
      #if ENABLED(Z_MULTI_ENDSTOPS) && NUM_Z_STEPPER_DRIVERS >= 3
        static float z3_endstop_adj;
      #endif
      #if ENABLED(Z_MULTI_ENDSTOPS) && NUM_Z_STEPPER_DRIVERS >= 4
        static float z4_endstop_adj;
      #endif
    #else
      typedef uint8_t esbits_t;
    #endif

  private:
    static bool enabled, enabled_globally;
    static esbits_t live_state;
    static volatile uint8_t hit_state;      // Use X_MIN, Y_MIN, Z_MIN, Z_MIN_PROBE, and WORK_PROBE as BIT index

    #if ENDSTOP_NOISE_THRESHOLD
      static esbits_t validated_live_state;
      static uint8_t endstop_poll_count;    // Countdown from threshold for polling
    #endif

  public:
    Endstops() {};

    /**
     * Initialize the endstop pins
     */
    static void init();

    /**
     * Are endstops or the probe set to abort the move?
     */
    FORCE_INLINE static bool abort_enabled() {
      return enabled || TERN0(HAS_BED_PROBE, z_probe_enabled);
    }

    static inline bool global_enabled() { return enabled_globally; }

    /**
     * Periodic call to poll endstops if required. Called from temperature ISR
     */
    static void poll();

    /**
     * Update endstops bits from the pins. Apply filtering to get a verified state.
     * If abort_enabled() and moving towards a triggered switch, abort the current move.
     * Called from ISR contexts.
     */
    static void update();

    /**
     * Get Endstop hit state.
     */
    FORCE_INLINE static uint8_t trigger_state() { return hit_state; }

    /**
     * Get current endstops state
     */
    FORCE_INLINE static esbits_t state() {
      return
        #if ENDSTOP_NOISE_THRESHOLD
          validated_live_state
        #else
          live_state
        #endif
      ;
    }

    static inline bool probe_switch_activated() {
      return (true
        #if ENABLED(PROBE_ACTIVATION_SWITCH)
          && READ(PROBE_ACTIVATION_SWITCH_PIN) == PROBE_ACTIVATION_SWITCH_STATE
        #endif
      );
    }

    /**
     * Report endstop hits to serial. Called from loop().
     */
    static void event_handler();

    /**
     * Report endstop states in response to M119
     */
    static void report_states();

    // Enable / disable endstop checking globally
    static void enable_globally(const bool onoff=true);

    // Enable / disable endstop checking
    static void enable(const bool onoff=true);

    // Disable / Enable endstops based on ENSTOPS_ONLY_FOR_HOMING and global enable
    static void not_homing();

    #if ENABLED(VALIDATE_HOMING_ENDSTOPS)
      // If the last move failed to trigger an endstop, call kill
      static void validate_homing_move();
    #else
      FORCE_INLINE static void validate_homing_move() { hit_on_purpose(); }
    #endif

    // Clear endstops (i.e., they were hit intentionally) to suppress the report
    FORCE_INLINE static void hit_on_purpose() { hit_state = 0; }

    // Enable / disable endstop z-probe checking
    #if HAS_BED_PROBE
      static volatile bool z_probe_enabled;
      static void enable_z_probe(const bool onoff=true);
    #endif

		#if HAS_TOOL_PROBE
			static bool tool_probe_enabled;

			static void enable_tool_probe(const bool enabled) {
				tool_probe_enabled = enabled;
			}
		#endif

		#if HAS_WORK_PROBE
			static xyz_bool_t work_probe_enabled;

			static void enable_work_probe(const Axis axis, const bool enabled = true) {
				work_probe_enabled[axis] = enabled;
			}
		#endif

		static volatile bool a_home_enabled;
		static void enable_a_home(const bool enabled) {
			a_home_enabled = enabled;
		}

    static void resync();

    // Debugging of endstops
    #if ENABLED(PINS_DEBUGGING)
      static bool monitor_flag;
      static void monitor();
      static void run_monitor();
    #endif

    #if ENABLED(SPI_ENDSTOPS)
      typedef struct {
        union {
          bool any;
          struct { bool x:1, y:1, z:1; };
        };
      } tmc_spi_homing_t;
      static tmc_spi_homing_t tmc_spi_homing;
      static void clear_endstop_state();
      static bool tmc_spi_homing_check();
    #endif
};

extern Endstops endstops;

/**
 * A class to save and change the endstop state,
 * then restore it when it goes out of scope.
 */
class TemporaryGlobalEndstopsState {
  bool saved;

  public:
    TemporaryGlobalEndstopsState(const bool enable) : saved(endstops.global_enabled()) {
      endstops.enable_globally(enable);
    }
    ~TemporaryGlobalEndstopsState() { endstops.enable_globally(saved); }
};
