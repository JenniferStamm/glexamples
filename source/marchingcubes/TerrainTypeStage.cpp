#include "TerrainTypeStage.h"


TerrainTypeStage::TerrainTypeStage()
    : AbstractStage("Terrain Data")
{
    addInput("terrainType", terrainType);
    addInput("userBaseTextureFilePath", userBaseTextureFilePath);
    addInput("userExtraTextureFilePath", userExtraTextureFilePath);
    addInput("userFragmentShaderFilePath", userFragmentShaderFilePath);
    addInput("userDensityGenererationShaderFilePath", userDensityGenererationShaderFilePath);

    addOutput("groundTextureFilePath", baseTextureFilePath);
    addOutput("striationTextureFilePath", extraTextureFilePath);
    addOutput("fragmentShaderTextureFilePath", fragmentShaderTextureFilePath);
    addOutput("densityGenererationShaderFilePath", densityGenererationShaderFilePath);
}

void TerrainTypeStage::initialize()
{
    changeToMossy();
}

void TerrainTypeStage::process()
{
    if (terrainType.hasChanged())
    {
        switch (terrainType.data())
        {
        case TerrainType::Mossy:
            changeToMossy();
            break;
        case TerrainType::UserDefined:
            changeToUserDefined();
            break;
        default:
            break;
        }
    }

    if (terrainType.data() == TerrainType::UserDefined)
    {
        if (userBaseTextureFilePath.hasChanged())
        {
            baseTextureFilePath.data() = userBaseTextureFilePath.data();
        }
        if (userExtraTextureFilePath.hasChanged())
        {
            extraTextureFilePath.data() = userExtraTextureFilePath.data();
        }
        if (userFragmentShaderFilePath.hasChanged())
        {
            fragmentShaderTextureFilePath.data() = userFragmentShaderFilePath.data();
        }
        if (userDensityGenererationShaderFilePath.hasChanged())
        {
            densityGenererationShaderFilePath.data() = userDensityGenererationShaderFilePath.data();
        }
    }
        


    invalidateOutputs();
}

void TerrainTypeStage::changeToMossy()
{
    baseTextureFilePath.data() = "data/marchingcubes/ground.png";
    extraTextureFilePath.data() = "data/marchingcubes/terrain_color.jpg";
    fragmentShaderTextureFilePath.data() = "data/marchingcubes/marchingcubes.frag";
    densityGenererationShaderFilePath.data() = "data/marchingcubes/densitygeneration.vert";
}

void TerrainTypeStage::changeToUserDefined()
{
    baseTextureFilePath.data() = userBaseTextureFilePath.data();
    extraTextureFilePath.data() = userExtraTextureFilePath.data();
    fragmentShaderTextureFilePath.data() = userFragmentShaderFilePath.data();
    densityGenererationShaderFilePath.data() = userDensityGenererationShaderFilePath.data();
}
