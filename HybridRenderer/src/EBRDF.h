#ifndef ELITE_BRDF
#define ELITE_BRDF

//Project includes
#include "EMath.h"
#include "ERGBColor.h"

namespace Elite
{
	namespace BRDF
	{
		inline RGBColor Phong(const RGBColor& specularColor, const float phongExponent, const FVector3& lightDirection, const FVector3& viewDirection, const FVector3& normal)
		{
			const auto reflect = -lightDirection + 2 * Dot(normal, lightDirection) * normal;
			const auto cosineAngle = Dot(reflect, viewDirection);
			if (cosineAngle > 0.f)
				return specularColor * powf(cosineAngle, phongExponent);
			else
				return RGBColor{ 0.f,0.f,0.f };
		}
	}
}
#endif // !ELITE_BRDF