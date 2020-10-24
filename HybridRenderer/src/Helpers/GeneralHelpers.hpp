#ifndef GENERAL_HELPERS_HPP
#define GENERAL_HELPERS_HPP
#include "EMath.h"


// Structs
struct BoundingBox
{
    BoundingBox() = default;
    explicit BoundingBox(const Elite::FVector2 min, const Elite::FVector2& max)
        : minPoint(min)
        , maxPoint(max)
    {}
    
    Elite::FVector2 minPoint;
    Elite::FVector2 maxPoint;
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
#endif // !GENERAL_HELPERS_HPP