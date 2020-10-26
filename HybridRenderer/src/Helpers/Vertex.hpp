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

inline VertexOutput Interpolate(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, const TriangleResult& result)
{
    VertexOutput vReturn{};

    const auto lerpDepth = result.linearInterpolatedDepth;
    const auto w0 = result.weight0;
    const auto w1 = result.weight1;
    const auto w2 = result.weight2;

    vReturn.worldPos = glm::vec3(glm::vec3((v0.worldPos / v0.pos.w) * w0 + glm::vec3(v1.worldPos / v1.pos.w) * w1 + glm::vec3(v2.worldPos / v2.pos.w) * w2) * lerpDepth);
    vReturn.uv = ((v0.uv / v0.pos.w) * w0 + (v1.uv / v1.pos.w) * w1 + (v2.uv / v2.pos.w) * w2) * lerpDepth;
    vReturn.normal = ((v0.normal / v0.pos.w) * w0 + (v1.normal / v1.pos.w) * w1 + (v2.normal / v2.pos.w) * w2) * lerpDepth;
    vReturn.tangent = ((v0.tangent / v0.pos.w) * w0 + (v1.tangent / v1.pos.w) * w1 + (v2.tangent / v2.pos.w) * w2) * lerpDepth;
    vReturn.viewDirection = ((v0.viewDirection / v0.pos.w) * w0 + (v1.viewDirection / v1.pos.w) * w1 + (v2.viewDirection / v2.pos.w) * w2) * lerpDepth;

    return vReturn;
}

inline VertexOutput Interpolate(const VertexOutput& v0, const VertexOutput& v1, const VertexOutput& v2, const float w0, const float w1, const float w2, const float lerpDepth)
{
    VertexOutput vReturn{};

    vReturn.worldPos = glm::vec3(glm::vec3((v0.worldPos / v0.pos.w) * w0 + glm::vec3(v1.worldPos / v1.pos.w) * w1 + glm::vec3(v2.worldPos / v2.pos.w) * w2) * lerpDepth);
    vReturn.uv = ((v0.uv / v0.pos.w) * w0 + (v1.uv / v1.pos.w) * w1 + (v2.uv / v2.pos.w) * w2) * lerpDepth;
    vReturn.normal = ((v0.normal / v0.pos.w) * w0 + (v1.normal / v1.pos.w) * w1 + (v2.normal / v2.pos.w) * w2) * lerpDepth;
    vReturn.tangent = ((v0.tangent / v0.pos.w) * w0 + (v1.tangent / v1.pos.w) * w1 + (v2.tangent / v2.pos.w) * w2) * lerpDepth;
    vReturn.viewDirection = ((v0.viewDirection / v0.pos.w) * w0 + (v1.viewDirection / v1.pos.w) * w1 + (v2.viewDirection / v2.pos.w) * w2) * lerpDepth;

    return vReturn;
}



#endif // !VERTEX_HPP
