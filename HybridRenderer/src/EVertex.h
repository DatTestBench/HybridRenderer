#ifndef ELITE_VERTEX
#define ELITE_VERTEX

//Project includes
#include "EMath.h"
#include "ERGBColor.h"
namespace Elite
{
	struct VertexInput
	{
		//Data-members
		FPoint3 pos = {};
		FVector2 uv = {};
		FVector3 normal = {};
		FVector3 tangent = {};

		//Constructors
		VertexInput() = default;
		VertexInput(const FPoint3& position, const FVector2& uv, const FVector3& normal, const FVector3& tangent = FVector3(0, 0, 0))
			: pos{ position }
			, uv{ uv }
			, normal{ normal }
			, tangent{ tangent }
		{  }
	};

	struct VertexOutput
	{
		//Data-members
		FPoint4 pos = {};
		FPoint3 worldPos = {};
		FVector2 uv = {};
		FVector3 normal = {};
		FVector3 tangent = {};
		FVector3 viewDirection = {};
		bool culled = { false };

		//Constructors
		VertexOutput() = default;
		/*Removed in favor of manually assigning all values if needed
		VertexOutput(const VertexInput& input) //Constructor fills in some values, but not all, use at own risk!
			: pos{ input.pos }
			, worldPos{ input.pos }
			, uv{ input.uv }
			, normal{ input.normal }
			, tangent{ input.tangent }
			, viewDirection{  }
			, culled{ false }
		{  }*/


	};

	static VertexOutput Interpolate(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, const float w0, const float w1, const float w2, const float lerpDepth)
	{
		VertexOutput vReturn{};

		vReturn.worldPos = FPoint3(FVector3((v0.worldPos / v0.pos.w) * w0 + FVector3(v1.worldPos / v1.pos.w) * w1 + FVector3(v2.worldPos / v2.pos.w) * w2) * lerpDepth);
		vReturn.uv = ((v0.uv / v0.pos.w) * w0 + (v1.uv / v1.pos.w) * w1 + (v2.uv / v2.pos.w) * w2) * lerpDepth;
		vReturn.normal = ((v0.normal / v0.pos.w) * w0 + (v1.normal / v1.pos.w) * w1 + (v2.normal / v2.pos.w) * w2) * lerpDepth;
		vReturn.tangent = ((v0.tangent / v0.pos.w) * w0 + (v1.tangent / v1.pos.w) * w1 + (v2.tangent / v2.pos.w) * w2) * lerpDepth;
		vReturn.viewDirection = ((v0.viewDirection / v0.pos.w) * w0 + (v1.viewDirection / v1.pos.w) * w1 + (v2.viewDirection / v2.pos.w) * w2) * lerpDepth;

		return vReturn;
	}
}



#endif // !ELITE_VERTEX