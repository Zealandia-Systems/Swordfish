#pragma once

#include <Eigen/Core>
#include <Eigen/LU>

#include <swordfish/types.h>

namespace swordfish::math {
#define EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, Size, SizeSuffix) \
	/** \ingroup matrixtypedefs */ \
	/** \brief `Size`&times;`Size` matrix of type `Type`. */ \
	typedef Eigen::Matrix<Type, Size, Size> Matrix##SizeSuffix##TypeSuffix; \
	/** \ingroup matrixtypedefs */ \
	/** \brief `Size`&times;`1` vector of type `Type`. */ \
	typedef Eigen::Matrix<Type, Size, 1> Vector##SizeSuffix##TypeSuffix; \
	/** \ingroup matrixtypedefs */ \
	/** \brief `1`&times;`Size` vector of type `Type`. */ \
	typedef Eigen::Matrix<Type, 1, Size> RowVector##SizeSuffix##TypeSuffix;

#define EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, Size) \
	/** \ingroup matrixtypedefs */ \
	/** \brief `Size`&times;`Dynamic` matrix of type `Type`. */ \
	typedef Eigen::Matrix<Type, Size, Eigen::Dynamic> Matrix##Size##X##TypeSuffix; \
	/** \ingroup matrixtypedefs */ \
	/** \brief `Dynamic`&times;`Size` matrix of type `Type`. */ \
	typedef Eigen::Matrix<Type, Eigen::Dynamic, Size> Matrix##X##Size##TypeSuffix;

#define EIGEN_MAKE_TYPEDEFS_ALL_SIZES(Type, TypeSuffix) \
	EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 2, 2) \
	EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 3, 3) \
	EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 4, 4) \
	EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 5, 5) \
	EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, 6, 6) \
	EIGEN_MAKE_TYPEDEFS(Type, TypeSuffix, Eigen::Dynamic, X) \
	EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 2) \
	EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 3) \
	EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 4) \
	EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 5) \
	EIGEN_MAKE_FIXED_TYPEDEFS(Type, TypeSuffix, 6)

	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(i8, i8)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(u8, u8)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(i16, i16)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(u16, u16)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(i32, i32)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(u32, u32)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(f32, f32)
	EIGEN_MAKE_TYPEDEFS_ALL_SIZES(f64, f64)

#undef EIGEN_MAKE_TYPEDEFS_ALL_SIZES
#undef EIGEN_MAKE_TYPEDEFS
#undef EIGEN_MAKE_FIXED_TYPEDEFS
} // namespace swordfish::math