#ifndef VERTEX_HPP
#define VERTEX_HPP

//Project includes
#include "Helpers/GeneralHelpers.hpp"

struct VertexInput
{
    //Data-members
    glm::vec3 pos = {};
    glm::vec2 uv = {};
    glm::vec3 normal = {};
    glm::vec3 tangent = {};

    //Constructors
    VertexInput() = default;

    VertexInput(const glm::vec3& position, const glm::vec2& uv, const glm::vec3& normal, const glm::vec3& tangent = glm::vec3(0, 0, 0))
        : pos(position),
          uv(uv),
          normal(normal),
          tangent(tangent)
    {
    }
};

struct VertexOutput
{
    //Data-members
    glm::vec4 pos = {};
    glm::vec3 worldPos = {};
    glm::vec2 uv = {};
    glm::vec3 normal = {};
    glm::vec3 tangent = {};
    glm::vec3 viewDirection = {};
    bool culled = {false};

    //Constructors
    VertexOutput() = default;
};

template<typename Attribute>
Attribute InterpolateInternal(const Attribute& a0, const Attribute& a1, const Attribute& a2, const glm::vec3 packedInvDepth, const TriangleResult& triResult, float interpolatedDepth)
{
    auto [invDepth0, invDepth1, invDepth2] = packedInvDepth;
    const auto [w0, w1, w2] = triResult;
    
    Attribute attribute0 = a0 * invDepth0 * w0;
    Attribute attribute1 = a1 * invDepth1 * w1;
    Attribute attribute2 = a2 * invDepth2 * w2;
    return (attribute0 + attribute1 + attribute2) * interpolatedDepth;
}

inline VertexOutput Interpolate(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, const TriangleResult& triResult, float interpolatedDepth)
{
    VertexOutput vReturn{};
    
    // Packing the depth for convenience
    glm::vec3 packedInvDepth {1.f / v0.pos.w, 1.f / v1.pos.w, 1.f / v2.pos.w};

    vReturn.worldPos = InterpolateInternal(v0.worldPos, v1.worldPos, v2.worldPos, packedInvDepth, triResult, interpolatedDepth);
    vReturn.uv = InterpolateInternal(v0.uv, v1.uv, v2.uv, packedInvDepth, triResult, interpolatedDepth);
    vReturn.normal = InterpolateInternal(v0.normal, v1.normal, v2.normal, packedInvDepth, triResult, interpolatedDepth);
    vReturn.tangent = InterpolateInternal(v0.tangent, v1.tangent, v2.tangent, packedInvDepth, triResult, interpolatedDepth);
    vReturn.viewDirection = InterpolateInternal(v0.viewDirection, v1.viewDirection, v2.viewDirection, packedInvDepth, triResult, interpolatedDepth);

    return vReturn;
}
#endif // !VERTEX_HPP
