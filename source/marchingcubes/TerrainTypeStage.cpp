#include "TerrainTypeStage.h"


TerrainTypeStage::TerrainTypeStage()
    : AbstractStage("Terrain Data")
{
    addInput("terrainType", terrainType);
    addInput("userBaseTextureFilePath", userBaseTextureFilePath);
    addInput("userExtraTextureFilePath", userExtraTextureFilePath);

    addOutput("groundTextureFilePath", baseTextureFilePath);
    addOutput("striationTextureFilePath", extraTextureFilePath);
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
    }
        


    invalidateOutputs();
}

void TerrainTypeStage::changeToMossy()
{
    baseTextureFilePath.data() = "data/marchingcubes/ground.png";
    extraTextureFilePath.data() = "data/marchingcubes/terrain_color.jpg";
}

void TerrainTypeStage::changeToUserDefined()
{
    baseTextureFilePath.data() = userBaseTextureFilePath.data();
    extraTextureFilePath.data() = userExtraTextureFilePath.data();
}
