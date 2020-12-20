#ifndef MATH_HELPERS_HPP
#define	MATH_HELPERS_HPP
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include "Helpers/Concepts.hpp"

namespace bme
{

#pragma region General

    /**
    * Squares value
    * @param base value to square
    * @return base squared
    * */
    template <Numeric T>
    [[nodiscard]] T Square(T base)
    {
        return glm::pow(base, static_cast<T>(2));
    }

#pragma endregion General

#pragma region Geometric
    //Returns the perpendicular vector of projection of v onto t
    template <typename VecType>
    [[nodiscard]] inline VecType Reject(const VecType& v, const VecType& t)
    { return v - glm::proj(v, t); }

    template<typename T>
    [[nodiscard]] inline T Cross2D(const glm::vec<2, T>& v1, const glm::vec<2, T>& v2)
    { return v1.x * v2.y - v1.y * v2.x; }


    template<Numeric T>
    [[nodiscard]] inline T Remap(T val, T min, T max)
    { return (val - min) / (max - min); }

    //Reflection vector equivalent to HLSL reflect
    template<typename VecType>
    [[nodiscard]] inline VecType ReflectHLSL(const VecType& i, const VecType& n)
    { return i - 2 * glm::dot(i, n) * n; }

    template <typename VecType>
    [[nodiscard]] inline VecType Reflect(const VecType& i, const VecType& n)
    { return -i + 2 * glm::dot(i, n) * n; }


[[nodiscard]] inline glm::mat4 LookAtRH(const glm::vec3& eye, const glm::vec3& f , const glm::vec3& worldUp)
    {
        const auto s = glm::normalize(glm::cross(f, worldUp));
        const auto u = glm::normalize(glm::cross(s, f));
        //vec<3, T, Q> const f(normalize(center - eye));
        //vec<3, T, Q> const s(normalize(cross(f, up)));
        //vec<3, T, Q> const u(cross(s, f));

        glm::mat4 Result(1);
        Result[0][0] = s.x;
        Result[1][0] = s.y;
        Result[2][0] = s.z;
        Result[0][1] = u.x;
        Result[1][1] = u.y;
        Result[2][1] = u.z;
        Result[0][2] =-f.x;
        Result[1][2] =-f.y;
        Result[2][2] =-f.z;
        Result[3][0] =-glm::dot(s, eye);
        Result[3][1] =-glm::dot(u, eye);
        Result[3][2] = glm::dot(f, eye);
        return Result;
    }

[[nodiscard]] inline glm::mat4 LookAtLH(const glm::vec3& eye, const glm::vec3& f , const glm::vec3& worldUp)
    {
        const auto s = glm::normalize(glm::cross(worldUp, f));
        const auto u = glm::normalize(glm::cross(f, s));
        
        //vec<3, T, Q> const f(normalize(center - eye));
        //vec<3, T, Q> const s(normalize(cross(up, f)));
        //vec<3, T, Q> const u(cross(f, s));

        glm::mat4 Result(1);
        Result[0][0] = s.x;
        Result[1][0] = s.y;
        Result[2][0] = s.z;
        Result[0][1] = u.x;
        Result[1][1] = u.y;
        Result[2][1] = u.z;
        Result[0][2] = f.x;
        Result[1][2] = f.y;
        Result[2][2] = f.z;
        Result[3][0] = -glm::dot(s, eye);
        Result[3][1] = -glm::dot(u, eye);
        Result[3][2] = -glm::dot(f, eye);
        return Result;
    }

#pragma endregion Geometric

#pragma region Random


    /**
     * Checks whether val is in ]min, max[
     * @param val value to do bound check on
     * @param min lower bound, exclusive
     * @param max upper board, exclusive
     * @return whether val is in ]min, max[
     * */
    template <Numeric T>
    [[nodiscard]] bool IsInBound(T val, T min, T max)
    {
        return val < max && val > min;
    }

    
    /**
     * Checks whether val is in [min, max]
     * @param val value to do bound check on
     * @param min lower bound, inclusive
     * @param max upper bound, inclusive
     * @return whether val is in [min, max]
     * */
    template <Numeric T>
    [[nodiscard]] bool IsInBoundInclusive(T val, T min, T max)
    {
        return val <= max && val >= min;
    }

    /**
    * Vector override to check whether point is in [min, max]
    * @param point value to do bound check on
    * @param min lower bound, inclusive
    * @param max upper bound, inclusive
    * @return whether point is in [min, max]
    * */
    template <Vector VecType>
    [[nodiscard]] bool IsInBoundVec(const VecType& point, const VecType& min, const VecType& max)
    {
        for (auto i = 0; i < VecType::lenght(); ++i)
        {
            if (point[i] > max[i] || point[i] < min[i])
                return false;
        }
        return true;
    }

    /**
     * Returns a random T in [0, 1[
     * @template T value type (requires Number)
     * @return Random T in [0, 1[
     * */
    template <Numeric T> 
    [[nodiscard]] T Rand() noexcept
    {
        static std::uniform_real_distribution<T> distribution(static_cast<T>(0.0), static_cast<T>(1.0));
        static std::mt19937 generator;
        return distribution(generator);
    }


    /**
     * Returns a random T in [min, max[
     * @template T value type (requires Number)
     * @param min minimum of random range, inclusive
     * @param max maximum of random range, non inclusive
     * @return Random T in [min, max[
     * */
    template <Numeric T>
    [[nodiscard]] T Rand(T min, T max) noexcept
    {
        return min + (max - min) * Rand<T>();
    }

    /**
     * Returns random vec3 where each component is [0, 1[
     * @return random unit vector
     * */
    [[nodiscard]] inline glm::vec3 RandVec3() noexcept
    {
        return glm::vec3(Rand<float>(), Rand<float>(), Rand<float>());
    }

    /**
     * Returns random vec3 where each component is [min, max[
     * @param min minimum of random range, inclusive
     * @param max maximum of random range, non inclusive
     * @return random vector with each component in [min, max[
     * */
    [[nodiscard]] inline glm::vec3 RandVec3(const float min, const float max) noexcept
    {
        return glm::vec3(Rand(min, max), Rand(min, max), Rand(min, max));
    }

#pragma endregion Random


}

// Adding structured bindings for GLM, do note, this will not work in Clang, nor GCC, for those this should be in namespace glm, not namespace std https://stackoverflow.com/questions/60785190/why-can-i-create-user-defined-structured-bindings-for-glmvec-in-msvc-and-icc
namespace std
{

    //template< std::size_t I, auto N, class T, auto Q>
    //constexpr auto& get(glm::vec<N, T, Q>& v) noexcept { return v[I]; }
    //
    //template< std::size_t I, auto N, class T, auto Q>
    //constexpr const auto& get(const glm::vec<N, T, Q>& v) noexcept { return v[I]; }
    
    template< std::size_t I, auto N, class T, auto Q>
    constexpr auto&& get(glm::vec<N, T, Q>&& v) noexcept { return std::move(v[I]); }
    
    template< std::size_t I, auto N, class T, auto Q>
    constexpr const auto&& get(const glm::vec<N, T, Q>&& v) noexcept { return std::move(v[I]); }
    
    template <auto N, class T, auto Q>
    struct tuple_size<glm::vec<N, T, Q>> : std::integral_constant<std::size_t, N> { };
    
    template <std::size_t I, auto N, class T, auto Q>
    struct tuple_element<I, glm::vec<N, T, Q>> {
        using type = decltype(get<I>(declval<glm::vec<N,T,Q>>()));
    };
}

#endif // !MATH_HELPERS_HPP