#ifndef GENERAL_HELPERS_HPP
#define GENERAL_HELPERS_HPP
#include "magic_enum.hpp"


// Structs
struct BoundingBox
{
    BoundingBox() = default;
    explicit BoundingBox(const glm::vec2& min, const glm::vec2& max)
        : minPoint(min)
        , maxPoint(max)
    {}
    
    glm::vec2 minPoint;
    glm::vec2 maxPoint;
};

struct TriangleResult
{
    TriangleResult() = default;
    explicit TriangleResult(const float interpDepth, const float lerpDepth, const float w0, const float w1, const float w2)
        : interpolatedDepth(interpDepth)
        , linearInterpolatedDepth(lerpDepth)
        , weight0(w0)
        , weight1(w1)
        , weight2(w2)
    {}
    
    float interpolatedDepth;
    float linearInterpolatedDepth;
    float weight0;
    float weight1;
    float weight2;

    static const TriangleResult Failed;
};
const inline TriangleResult TriangleResult::Failed(0.f, 0.f, 0.f, 0.f, 0.f);

#define ENUM_TO_C_STR(value) std::string(magic_enum::enum_name(value)).c_str()
#define TO_C_STR(value) std::to_string(value).c_str()
#define C_STR_FROM_VIEW(value) std::string(value).c_str()

// Wrappers for enums




#endif // !GENERAL_HELPERS_HPP