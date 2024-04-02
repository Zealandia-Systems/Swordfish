/*
 * Vector3.cpp
 *
 * Created: 30/11/2021 1:49:54 pm
 *  Author: smohekey
 */

#include "Vector3.h"

namespace swordfish::core {
	template<>
	ValueField<float32_t> Vector3Base<false>::__xField = { "x", 0, 0.0f };

	template<>
	ValueField<float32_t> Vector3Base<false>::__yField = { "y", 4, 0.0f };

	template<>
	ValueField<float32_t> Vector3Base<false>::__zField = { "z", 8, 0.0f };

	template<>
	Schema Vector3Base<false>::__schema = {
		utils::typeName<Vector3Base<false>>(),
		nullptr,
		{ Vector3Base<false>::__xField,
		  Vector3Base<false>::__yField,
		  Vector3Base<false>::__zField },
		{

			}
	};

	template<>
	LinearValueField<float32_t> Vector3Base<true>::__xField = { "x", 0, 0.0f };

	template<>
	LinearValueField<float32_t> Vector3Base<true>::__yField = { "y", 4, 0.0f };

	template<>
	LinearValueField<float32_t> Vector3Base<true>::__zField = { "z", 8, 0.0f };

	template<>
	Schema Vector3Base<true>::__schema = {
		utils::typeName<Vector3Base<true>>(),
		nullptr,
		{ Vector3Base<true>::__xField,
		  Vector3Base<true>::__yField,
		  Vector3Base<true>::__zField },
		{

			}
	};
} // namespace swordfish::core