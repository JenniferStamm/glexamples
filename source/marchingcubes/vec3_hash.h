#pragma once

#include <glm/vec3.hpp>

#include <xhash>

// This code is needed in order to use a glm::vec3 as the key of an unordered_map. The implemenation is based on https://github.com/g-truc/glm/pull/320
// and will become obsolete once a new GLM version (0.9.7) is published. Then, the following line include will suffice:

// #include <glm/gtx/hash.hpp>

namespace glm {
    namespace detail
    {
        GLM_INLINE void hash_combine(size_t &seed, size_t hash)
        {
            hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash;
        }
    }
}
namespace std {
    template <>
    struct hash<glm::vec3>
    {
        GLM_FUNC_DECL size_t operator()(const glm::vec3 &v) const
        {
            size_t seed = 0;
            hash<float> hasher;
            glm::detail::hash_combine(seed, hasher(v.x));
            glm::detail::hash_combine(seed, hasher(v.y));
            glm::detail::hash_combine(seed, hasher(v.z));
            return seed;
        };
    };
}