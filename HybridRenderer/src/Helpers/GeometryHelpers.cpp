#include "pch.h"
#include "Helpers/GeometryHelpers.hpp"

#include "GeneralHelpers.hpp"
#include "Vertex.hpp"

void bgh::CalculateWeightArea(const glm::vec2& pixelPoint, const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, TriangleResult& triResult) noexcept
{
    //Is point in triangle
    const auto edgeA = glm::vec2(v1.pos) - glm::vec2(v0.pos);
    auto pointToSide = pixelPoint - glm::vec2(v0.pos);
    const auto signedAreaTriV2 = bme::Cross2D(pointToSide, edgeA);

    const auto edgeB = glm::vec2(v2.pos) - glm::vec2(v1.pos);
    pointToSide = pixelPoint - glm::vec2(v1.pos);
    const auto signedAreaTriV0 = bme::Cross2D(pointToSide, edgeB);

    const auto edgeC = glm::vec2(v0.pos) - glm::vec2(v2.pos);
    pointToSide = pixelPoint - glm::vec2(v2.pos);
    const auto signedAreaTriV1 = bme::Cross2D(pointToSide, edgeC);

    triResult.weight0 = signedAreaTriV0;
    triResult.weight1 = signedAreaTriV1;
    triResult.weight2 = signedAreaTriV2;
    triResult.Normalize();
}

bool bgh::IsPointInTriangle(const TriangleResult& triResult) noexcept
{
    if (triResult.weight0 < 0 ||
        triResult.weight1 < 0 ||
        triResult.weight2 < 0)
            return false;
    return true;
}