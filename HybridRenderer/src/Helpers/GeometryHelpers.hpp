#ifndef GEOMETRY_HELPERS_HPP
#define GEOMETRY_HELPERS_HPP
#include <glm/vec2.hpp>

struct TriangleResult;
struct VertexOutput;

namespace bgh
{
    void CalculateWeightArea(const glm::vec2& pixelPoint, const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, TriangleResult& triResult) noexcept;
    [[nodiscard]] auto IsPointInTriangle(const TriangleResult& triResult) noexcept -> bool;
}


#endif // !GEOMETRY_HELPERS_HPP