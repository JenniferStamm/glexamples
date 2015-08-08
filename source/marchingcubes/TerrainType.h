#pragma once


#include <reflectionzeug/property/PropertyEnum.h>

enum class TerrainType : unsigned int
{
    Mossy,
    Moon,
    UserDefined
};

namespace reflectionzeug
{

    template<>
    struct EnumDefaultStrings<TerrainType>
    {
        std::map<TerrainType, std::string> operator()()
        {
            return{
                    { TerrainType::Mossy, "Mossy" },
                    { TerrainType::Moon, "Moon" },
                    { TerrainType::UserDefined, "User Defined" }
            };
        }
    };

} // namespace reflectionzeug
