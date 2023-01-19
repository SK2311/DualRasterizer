#pragma once
#include "Math.h"
#include "array"
#include <string>
//#include "Texture.h"

namespace dae
{
	struct Vertex_In
	{
		Vector3 position{};
		Vector2 uv{}; //W3
		Vector3 normal{}; //W4
		Vector3 tangent{}; //W4
		ColorRGB color{ colors::White };
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
		ColorRGB color{ colors::White };
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	enum class ShadingMode
	{
		Combined,
		ObservedArea,
		Diffuse,
		Specular
	};
}