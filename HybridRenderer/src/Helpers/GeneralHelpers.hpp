﻿#ifndef GENERAL_HELPERS_HPP
#define GENERAL_HELPERS_HPP
//#include "magic_enum.hpp"
#include "MathHelpers.hpp"
#include "Helpers/Concepts.hpp"

// Structs
template <Vector VecType>
struct BoundingBox
{
    BoundingBox() = default;
    
    explicit BoundingBox(const VecType& min, const VecType& max)
        : minPoint(min)
        , maxPoint(max)
    {
        UpdateInternal();
    }
    
    union
    {
        VecType minPoint;
        VecType bottomLeft;
    };

    union
    {
        VecType maxPoint;
        VecType topRight;
    };

    VecType topLeft;
    VecType bottomRight;

    [[nodiscard]] bool Contains(const VecType& testPoint) const
    {
        return bme::IsInBoundVec(testPoint, minPoint, maxPoint);    
    }

    void Expand(const VecType& newBound) noexcept
    {
        for (auto i = 0; i < VecType::length(); ++i)
        {
            minPoint[i] = std::min(minPoint[i], newBound[i]);
            maxPoint[i] = std::max(maxPoint[i], newBound[i]);
            UpdateInternal();
        }
    }

    void AddMargin(const int margin) noexcept
    {
        for (auto i = 0; i < VecType::length(); ++i)
        {
            minPoint[i] = minPoint[i] - margin;
            maxPoint[i] = maxPoint[i] + margin;
            UpdateInternal();
        } 
    }

    void Clamp(const VecType& lower, const VecType& upper) noexcept
    {
       minPoint = glm::clamp(minPoint, lower, upper);
       maxPoint = glm::clamp(maxPoint, lower, upper);
    }
    
private:
    //todo find an elegant solution for topLeft and bottomRight in 3D
    void UpdateInternal() noexcept
    {
        topLeft = {minPoint.x, maxPoint.y};
        bottomRight = {maxPoint.x, minPoint.y};
    }
};

using BoundingBox2D = BoundingBox<glm::vec2>;
using BoundingBox3D = BoundingBox<glm::vec3>;

struct TriangleResult
{
    TriangleResult() = default;
    explicit TriangleResult(const float w0, const float w1, const float w2)
        : weight0(w0)
        , weight1(w1)
        , weight2(w2)
    {}
    
    float weight0;
    float weight1;
    float weight2;

    template <Numeric ScalarType>
    TriangleResult& operator/=(ScalarType scalar)
    {
        weight0 /= scalar;
        weight1 /= scalar;
        weight2 /= scalar;
        return *this;
    }

    void Normalize() noexcept
    {
        const auto total = weight0 + weight1 + weight2;
        weight0 /= total;
        weight1 /= total;
        weight2 /= total;
    }
};

#define ENUM_TO_C_STR(value) std::string(magic_enum::enum_name(value)).c_str()
#define TO_C_STR(value) std::to_string(value).c_str()
#define C_STR_FROM_VIEW(value) std::string(value).c_str()
#define C_STR_FROM_RAW(value) std::string(value).c_str()

#endif // !GENERAL_HELPERS_HPP