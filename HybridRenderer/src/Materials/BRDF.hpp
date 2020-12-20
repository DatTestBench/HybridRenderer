#ifndef BRDF_HPP
#define BRDF_HPP

//Project includes
#include "Helpers/RGBColor.hpp"

namespace BRDF
{
    inline RGBColor Phong(const RGBColor& specularColor, const float phongExponent, const glm::vec3& lightDirection, const glm::vec3& viewDirection, const glm::vec3& normal)
    {
        const auto reflect = bme::Reflect(lightDirection, normal);
        const auto cosineAngle = glm::clamp(glm::dot(viewDirection, reflect), 0.f, 1.f);
        return specularColor * powf(cosineAngle, phongExponent);
    }
}

#endif // !BRDF_HPP
