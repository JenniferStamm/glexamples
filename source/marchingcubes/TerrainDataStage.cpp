#include "TerrainDataStage.h"


TerrainDataStage::TerrainDataStage()
    : AbstractStage("Terrain Data")
{
    addInput("terrainType", terrainType);
    addInput("groundTextureFilePath", groundTextureFilePath);
    addInput("striationTextureFilePath", striationTextureFilePath);

    addOutput("groundTextureFilePath", baseTextureFilePath);
    addOutput("striationTextureFilePath", extraTextureFilePath);
}

void TerrainDataStage::initialize()
{
    changeToMossy();
}

void TerrainDataStage::process()
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
        if (groundTextureFilePath.hasChanged())
        {
            baseTextureFilePath.data() = groundTextureFilePath.data();
        }
        if (striationTextureFilePath.hasChanged())
        {
            extraTextureFilePath.data() = striationTextureFilePath.data();
        }
    }
        


    invalidateOutputs();
}

void TerrainDataStage::changeToMossy()
{
    baseTextureFilePath.data() = "data/marchingcubes/ground.png";
    extraTextureFilePath.data() = "data/marchingcubes/terrain_color.jpg";
}

void TerrainDataStage::changeToUserDefined()
{
    baseTextureFilePath.data() = groundTextureFilePath.data();
    extraTextureFilePath.data() = striationTextureFilePath.data();
}
