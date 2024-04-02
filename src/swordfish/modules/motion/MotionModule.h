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

#include "CoordinateSystem.h"
#include "CoordinateSystemTable.h"
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
		float32_t feedRate = NaN;
		utils::Flags<AxisSelector> relativeAxes = AxisSelector::None;
		float32_t accel_mm_s2 = NaN;
	};

	class Limits;

	class MotionModule : public swordfish::Module {
	private:
		static core::ValueField<int16_t> __activeCoordinateSystemField;
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

		Eigen::Vector3f _offset;
		Eigen::Vector3f _rotation;
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

		void setHomeOffset(Eigen::Vector3f homeOffset);

		Eigen::Vector3f getWorkOffset() {
			return __workOffsetField.get(_pack);
		}

		void setWorkOffset(Eigen::Vector3f workOffset);

		Eigen::Vector3f getToolOffset() {
			return __toolOffsetField.get(_pack);
		}

		void setToolOffset(Eigen::Vector3f toolOffset);

		CoordinateSystemTable& getWorkCoordinateSystems() {
			return _workCoordinateSystems;
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

		FORCE_INLINE float32_t toLogical(AxisEnum axis, float32_t value) {
			return value + _offset(axis);
		}
		FORCE_INLINE float32_t toNative(AxisEnum axis, float32_t value) {
			return value - _offset(axis);
		}

		FORCE_INLINE void toLogical(xy_pos_t& raw) {
			raw.x += _offset(X);
			raw.y += _offset(Y);
		}
		FORCE_INLINE void toLogical(xyz_pos_t& raw) {
			raw.x += _offset(X);
			raw.y += _offset(Y);
			raw.z += _offset(Z);
		}
		FORCE_INLINE Eigen::Vector2f toLogical(Eigen::Vector2f& raw) {
			return { raw(X) += _offset(X), raw(Y) += _offset(Y) };
		}
		FORCE_INLINE Eigen::Vector3f toLogical(Eigen::Vector3f& raw) {
			return { raw(X) += _offset(X), raw(Y) += _offset(Y), raw(Z) += _offset(Z) };
		}

		FORCE_INLINE void toNative(xy_pos_t& raw) {
			raw.x -= _offset(X);
			raw.y -= _offset(Y);
		}
		FORCE_INLINE void toNative(xyz_pos_t& raw) {
			raw.x -= _offset(X);
			raw.y -= _offset(Y);
			raw.z -= _offset(Z);
		}
		FORCE_INLINE Eigen::Vector2f toNative(Eigen::Vector2f& raw) {
			return { raw(X) -= _offset(X), raw(Y) -= _offset(Y) };
		}
		FORCE_INLINE Eigen::Vector3f toNative(Eigen::Vector3f& raw) {
			return { raw(X) -= _offset(X), raw(Y) -= _offset(Y), raw(Z) -= _offset(Z) };
		}

		/*
		FORCE_INLINE float32_t toLogical(AxisEnum axis, float32_t value) {
		  return value + _offset(axis);
		}

		FORCE_INLINE float32_t toNative(AxisEnum axis, float32_t value) {
		  return value - _offset(axis);
		}

		FORCE_INLINE void toLogical(xy_pos_t &raw) {
		  Eigen::Vector4f v = { raw.x, raw.y, 0, 1 };

		  auto r = _logicalTransform * v;

		  raw.x = r(X);
		  raw.y = r(Y);
		}

		FORCE_INLINE void toLogical(xyz_pos_t &raw) {
		  Eigen::Vector4f v = { raw.x, raw.y, raw.z, 1 };

		  auto r = _logicalTransform * v;

		  raw.x = r(X);
		  raw.y = r(Y);
		  raw.z = r(Z);
		}

		FORCE_INLINE Eigen::Vector2f toLogical(Eigen::Vector2f& raw) {
		  Eigen::Vector4f v = { raw(X), raw(Y), 0, 1 };

		  auto r = _logicalTransform * v;

		  return { r(X), r(Y) };
		}

		FORCE_INLINE Eigen::Vector3f toLogical(Eigen::Vector3f& raw) {
		  Eigen::Vector4f v = { raw(X), raw(Y), raw(Z), 1 };

		  auto r = _logicalTransform * v;

		  return { r(X), r(Y), r(Z) };
		}

		FORCE_INLINE void toNative(xy_pos_t &raw) {
		  Eigen::Vector4f v = { raw.x, raw.y, 0, 1 };

		  auto r = _nativeTransform * v;

		  raw.x = r(X);
		  raw.y = r(Y);
		}

		FORCE_INLINE void toNative(xyz_pos_t &raw) {
		  Eigen::Vector4f v = { raw.x, raw.y, raw.z, 1 };

		  auto r = _nativeTransform * v;

		  raw.x = r(X);
		  raw.y = r(Y);
		  raw.z = r(Z);
		}

		FORCE_INLINE Eigen::Vector2f toNative(Eigen::Vector2f& raw) {
		  Eigen::Vector4f v = { raw(X), raw(Y), 0, 1 };

		  auto r = _nativeTransform * v;

		  return { r(X), r(Y) };
		}

		FORCE_INLINE Eigen::Vector3f toNative(Eigen::Vector3f& raw) {
		  Eigen::Vector4f v = { raw(X), raw(Y), raw(Z), 1 };

		  auto r = _nativeTransform * v;

		  return { r(X), r(Y), r(Z) };
		}
		*/

		static MotionModule& getInstance(core::Object* parent = nullptr);
	};
} // namespace swordfish::motion