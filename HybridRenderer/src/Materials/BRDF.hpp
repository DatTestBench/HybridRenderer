#ifndef BRDF_HPP
#define BRDF_HPP

//Project includes
#include "Helpers/EMath.h"
#include "Helpers/RGBColor.hpp"

namespace BRDF
{
    inline RGBColor Phong(const RGBColor& specularColor, const float phongExponent, const glm::vec3& lightDirection, const glm::vec3& viewDirection, const glm::vec3& normal)
    {
        const auto reflect = -lightDirection + 2 * glm::dot(normal, lightDirection) * normal;
        const auto cosineAngle = glm::dot(reflect, viewDirection);
        if (cosineAngle > 0.f)
            return specularColor * powf(cosineAngle, phongExponent);

        return RGBColor(0);
    }
}

#endif // !BRDF_HPP
