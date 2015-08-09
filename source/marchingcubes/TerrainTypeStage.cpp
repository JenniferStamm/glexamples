#include "TerrainTypeStage.h"


TerrainTypeStage::TerrainTypeStage()
    : AbstractStage("Terrain Data")
{
    addInput("terrainType", terrainType);
    addInput("userBaseTextureFilePath", userBaseTextureFilePath);
    addInput("userExtraTextureFilePath", userExtraTextureFilePath);
    addInput("userFragmentShaderFilePath", userFragmentShaderFilePath);
    addInput("userDensityGenererationShaderFilePath", userDensityGenererationShaderFilePath);
    addInput("userRotationVector1", userRotationVector1);
    addInput("userRotationVector2", userRotationVector2);
    addInput("userWarpFactor", userWarpFactor);

    addOutput("groundTextureFilePath", baseTextureFilePath);
    addOutput("striationTextureFilePath", extraTextureFilePath);
    addOutput("fragmentShaderTextureFilePath", fragmentShaderTextureFilePath);
    addOutput("densityGenererationShaderFilePath", densityGenererationShaderFilePath); 
    addOutput("rotationVector1", rotationVector1);
    addOutput("rotationVector2", rotationVector2);
    addOutput("warpFactor", warpFactor);
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
        case TerrainType::Moon:
            changeToMoon();
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
        if (userRotationVector1.hasChanged())
        {
            rotationVector1.data() = userRotationVector1.data();
        }
        if (userRotationVector2.hasChanged())
        {
            rotationVector2.data() = userRotationVector2.data();
        }
        if (userWarpFactor.hasChanged())
        {
            warpFactor.data() = userWarpFactor.data();
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
    rotationVector1.data() = glm::vec3(1, 0.3, 0.5);
    rotationVector2.data() = glm::vec3(0.1, 0.5, 0.3);
    warpFactor.data() = 3.4f;
}

void TerrainTypeStage::changeToMoon()
{
    baseTextureFilePath.data() = "data/marchingcubes/moon.png";
    extraTextureFilePath.data() = "data/marchingcubes/moon.png";
    fragmentShaderTextureFilePath.data() = "data/marchingcubes/moonmarchingcubes.frag";
    densityGenererationShaderFilePath.data() = "data/marchingcubes/moondensitygeneration.vert";
    rotationVector1.data() = glm::vec3(1, 0.3, 0.5);
    rotationVector2.data() = glm::vec3(0.1, 0.1, 0.0);
    warpFactor.data() = 1.4f;
}

void TerrainTypeStage::changeToUserDefined()
{
    baseTextureFilePath.data() = userBaseTextureFilePath.data();
    extraTextureFilePath.data() = userExtraTextureFilePath.data();
    fragmentShaderTextureFilePath.data() = userFragmentShaderFilePath.data();
    densityGenererationShaderFilePath.data() = userDensityGenererationShaderFilePath.data();
    rotationVector1.data() = userRotationVector1.data();
    rotationVector2.data() = userRotationVector2.data();
    warpFactor.data() = userWarpFactor.data();
}
