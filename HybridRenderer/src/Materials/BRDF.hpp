#ifndef BRDF_HPP
#define BRDF_HPP

//Project includes
#include "Helpers/EMath.h"
#include "Helpers/RGBColor.hpp"

namespace BRDF
{
    inline Elite::RGBColor Phong(const Elite::RGBColor& specularColor, const float phongExponent, const Elite::FVector3& lightDirection, const Elite::FVector3& viewDirection, const Elite::FVector3& normal)
    {
        const auto reflect = -lightDirection + 2 * Dot(normal, lightDirection) * normal;
        const auto cosineAngle = Dot(reflect, viewDirection);
        if (cosineAngle > 0.f)
            return specularColor * powf(cosineAngle, phongExponent);
        else
            return Elite::RGBColor{0.f, 0.f, 0.f};
    }
}

#endif // !BRDF_HPP
