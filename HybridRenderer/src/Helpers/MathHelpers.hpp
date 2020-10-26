#ifndef MATH_HELPERS_HPP
#define	MATH_HELPERS_HPP
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/constants.hpp>

namespace bme
{

    //Returns the perpendicular vector of projection of v onto t
    template <typename VecType>
    inline VecType Reject(const VecType& v, const VecType& t)
    { return v - glm::proj(v, t); }

    template<typename T>
    inline T Cross2D(const glm::vec<2, T>& v1, const glm::vec<2, T>& v2)
    { return v1.x * v2.y - v1.y * v2.x; }

    template<typename T>
    inline T Remap(T val, T min, T max)
    { return (val - min) / (max - min); }
}

#endif // !MATH_HELPERS_HPP