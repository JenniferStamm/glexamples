#pragma once


#include <reflectionzeug/property/PropertyEnum.h>

enum class TerrainType : unsigned int
{
    Mossy,
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
                    { TerrainType::UserDefined, "User Defined" }
            };
        }
    };

} // namespace reflectionzeug
