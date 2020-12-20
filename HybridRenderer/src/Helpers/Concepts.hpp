#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP

#include <type_traits>
#include <concepts>

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Note that glm::vec3 should be casted to glm::vec3 before use in functions that use this
template <typename VecType>
concept Vector = std::is_same_v<VecType, glm::vec3> || std::is_same_v<VecType, glm::vec2>;

#endif // !CONCEPTS_HPP