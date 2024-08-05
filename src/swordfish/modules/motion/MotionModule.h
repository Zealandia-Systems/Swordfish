/*
 * MotionManager.h
 *
 * Created: 20/08/2021 5:26:40 pm
 *  Author: smohekey
 */

#pragma once

#include <limits>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <swordfish/macros.h>
#include <swordfish/types.h>
#include <swordfish/Module.h>

#include <swordfish/utils/Flags.h>

#include <swordfish/core/Boundary.h>
#include <swordfish/core/ObjectList.h>
#include <swordfish/core/Pack.h>
#include <swordfish/modules/status/StatusModule.h>

#include "CoordinateSystem.h"
#include "CoordinateSystemTable.h"
#include "Feedrate.h"
#include "Limits.h"
#include "NotHomedException.h"

#include <marlin/core/types.h>

namespace swordfish::estop {
	class EStopModule;
} // namespace swordfish::estop

namespace swordfish::motion {
	enum class AxisSelector {
		None = 0,
		X = 1 << 0,
		Y = 1 << 1,
		Z = 1 << 2,
		All = X | Y | Z,
		__size__ = 3
	};

	struct Movement {
		float32_t x = NaN;
		float32_t y = NaN;
		float32_t z = NaN;
		std::optional<FeedRate> feed_rate = std::nullopt;
		utils::Flags<AxisSelector> relativeAxes = AxisSelector::None;
		float32_t accel_mm_s2 = NaN;
		swordfish::status::MachineState state;
	};

	class Limits;

	class MotionModule : public swordfish::Module {
	private:
		static core::ValueField<int16_t> __activeCoordinateSystemField;
		static core::ValueField<bool> __shouldHomeFourth;
		static core::ObjectField<CoordinateSystemTable> __workCoordinateSystemsField;
		static core::ObjectField<Limits> __limitsField;
		static core::ObjectField<core::LinearVector3> __homeOffsetField;
		static core::ObjectField<core::LinearVector3> __workOffsetField;
		static core::ObjectField<core::LinearVector3> __toolOffsetField;

		MotionModule(core::Object* parent);

		static MotionModule* __instance;

	protected:
		static core::Schema __schema;

		core::Pack _pack;

		CoordinateSystemTable& _workCoordinateSystems;
		Limits& _limits;

		CoordinateSystem _machineCoordinateSystem; // G53
		CoordinateSystem* _activeCoordinateSystem = nullptr;
		CoordinateSystem* _toolChangeCoordinateSystem = nullptr; // G59.8
		CoordinateSystem* _toolCoordinateSystem = nullptr; // G59.9

		swordfish::math::Vector3f32 _offset;
		swordfish::math::Vector3f32 _rotation;
		Eigen::Matrix4f _nativeTransform;
		Eigen::Matrix4f _logicalTransform;

		virtual core::Pack& getPack() override;

		void updateOffset();
		void updateTransforms();

	public:
		virtual ~MotionModule() {
		}

		virtual const char* name() override {
			return "Motion Module";
		}
		virtual void init() override;

		void setLimitsEnabled(bool limitsEnabled) {
			_limits.setEnabled(limitsEnabled);
		}

		bool areLimitsEnabled() {
			return _limits.areEnabled();
		}

		Eigen::Vector3f getHomeOffset() {
			return __homeOffsetField.get(_pack);
		}

		void setHomeOffset(swordfish::math::Vector3f32 homeOffset);

		Eigen::Vector3f getWorkOffset() {
			return __workOffsetField.get(_pack);
		}

		void setWorkOffset(swordfish::math::Vector3f32 workOffset);

		Eigen::Vector3f getToolOffset() {
			return __toolOffsetField.get(_pack);
		}

		void setToolOffset(swordfish::math::Vector3f32 toolOffset);

		CoordinateSystemTable& getWorkCoordinateSystems() {
			return _workCoordinateSystems;
		}

		bool shouldHomeFourth() {
			return __shouldHomeFourth.get(_pack);
		}

		Limits& getLimits() {
			return _limits;
		}

		CoordinateSystem& getMachineCoordiateSystem() {
			return _machineCoordinateSystem;
		}

		CoordinateSystem& getToolCoordinateSystem() {
			return *_toolCoordinateSystem;
		}

		CoordinateSystem& getToolChangeCoordinateSystem() {
			return *_toolChangeCoordinateSystem;
		}

		CoordinateSystem& getActiveCoordinateSystem();

		void setActiveCoordinateSystem(CoordinateSystem& coordinateSystem);

		void move(Movement movement);

		void rapidMove(Movement movement);

		void feedMove(Movement movement);

		virtual void read(io::InputStream& stream) override;

		void synchronize();

		FORCE_INLINE float32_t toLogical(Axis axis, f32 value) {
			if (axis.is_linear()) {
				return value + _offset[axis];
			} else {
				return value + _rotation[axis - 3];
			}
		}
		FORCE_INLINE float32_t toNative(Axis axis, f32 value) {
			if (axis.is_linear()) {
				return value - _offset[axis];
			} else {
				return value - _rotation[axis - 3];
			}
		}

		FORCE_INLINE swordfish::math::Vector2f32 toLogical(const swordfish::math::Vector2f32& raw) {
			return {
				raw.x() + _offset.x(),
				raw.y() + _offset.y()
			};
		}

		FORCE_INLINE swordfish::math::Vector3f32 toLogical(const swordfish::math::Vector3f32& raw) {
			return {
				raw.x() + _offset.x(),
				raw.y() + _offset.y(),
				raw.z() + _offset.z()
			};
		}

		FORCE_INLINE swordfish::math::Vector4f32 toLogical(const swordfish::math::Vector4f32& raw) {
			return {
				raw.x() + _offset.x(),
				raw.y() + _offset.y(),
				raw.z() + _offset.z(),
				raw.a() + _rotation.x()
			};
		}

		FORCE_INLINE swordfish::math::Vector5f32 toLogical(const swordfish::math::Vector5f32& raw) {
			return {
				raw.x() + _offset.x(),
				raw.y() + _offset.y(),
				raw.z() + _offset.z(),
				raw.a() + _rotation.x(),
				raw.b() + _rotation.y()
			};
		}

		FORCE_INLINE swordfish::math::Vector6f32 toLogical(const swordfish::math::Vector6f32& raw) {
			return {
				raw.x() + _offset.x(),
				raw.y() + _offset.y(),
				raw.z() + _offset.z(),
				raw.a() + _rotation.x(),
				raw.b() + _rotation.y(),
				raw.c() + _rotation.z()
			};
		}

FORCE_INLINE swordfish::math::Vector2f32 toNative(const swordfish::math::Vector2f32& raw) {
			return {
				raw.x() - _offset.x(),
				raw.y() - _offset.y()
			};
		}

		FORCE_INLINE swordfish::math::Vector3f32 toNative(const swordfish::math::Vector3f32& raw) {
			return {
				raw.x() - _offset.x(),
				raw.y() - _offset.y(),
				raw.z() - _offset.z()
			};
		}

		FORCE_INLINE swordfish::math::Vector4f32 toNative(const swordfish::math::Vector4f32& raw) {
			return {
				raw.x() - _offset.x(),
				raw.y() - _offset.y(),
				raw.z() - _offset.z(),
				raw.a() - _rotation.x()
			};
		}

		FORCE_INLINE swordfish::math::Vector5f32 toNative(const swordfish::math::Vector5f32& raw) {
			return {
				raw.x() - _offset.x(),
				raw.y() - _offset.y(),
				raw.z() - _offset.z(),
				raw.a() - _rotation.x(),
				raw.b() - _rotation.y()
			};
		}

		FORCE_INLINE swordfish::math::Vector6f32 toNative(const swordfish::math::Vector6f32& raw) {
			return {
				raw.x() - _offset.x(),
				raw.y() - _offset.y(),
				raw.z() - _offset.z(),
				raw.a() - _rotation.x(),
				raw.b() - _rotation.y(),
				raw.c() - _rotation.z()
			};
		}

		static MotionModule& getInstance(core::Object* parent = nullptr);
	};
} // namespace swordfish::motion