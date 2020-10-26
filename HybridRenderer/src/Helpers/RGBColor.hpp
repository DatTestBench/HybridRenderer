#ifndef RGBCOLOR_HPP
#define	RGBCOLOR_HPP

#include "Helpers/MathHelpers.hpp"
using RGBColor = glm::vec3;

inline void MaxToOne(RGBColor& c) noexcept
{
	const auto maxValue = std::max(c.r, std::max(c.g, c.b));
	if (maxValue > 1.f)
		c /= maxValue;
}

[[nodiscard]] inline RGBColor GetRemapped(const RGBColor& c, const float min, const float max) noexcept
{
	return RGBColor(
		bme::Remap(c.r, min, max),
		bme::Remap(c.g, min, max),
		bme::Remap(c.b, min, max));
}

inline void Remap(RGBColor& c, const float min, const float max) noexcept
{
	c.r = bme::Remap(c.r, min, max);
	c.g = bme::Remap(c.g, min, max);
	c.b = bme::Remap(c.b, min, max);
}



#endif // !RGBCOLOR_HPP