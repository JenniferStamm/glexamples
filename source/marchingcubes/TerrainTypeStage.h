#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>

#include <reflectionzeug/base/FilePath.h>

#include "TerrainType.h"

class TerrainTypeStage : public gloperate::AbstractStage
{
public:
    TerrainTypeStage();

    virtual void initialize() override;

public:
    gloperate::InputSlot<TerrainType> terrainType;
    gloperate::InputSlot<reflectionzeug::FilePath> userBaseTextureFilePath;
    gloperate::InputSlot<reflectionzeug::FilePath> userExtraTextureFilePath;
    gloperate::InputSlot<reflectionzeug::FilePath> userFragmentShaderFilePath;
    gloperate::InputSlot<reflectionzeug::FilePath> userDensityGenererationShaderFilePath;

    gloperate::Data<reflectionzeug::FilePath> baseTextureFilePath;
    gloperate::Data<reflectionzeug::FilePath> extraTextureFilePath;
    gloperate::Data<reflectionzeug::FilePath> fragmentShaderTextureFilePath;
    gloperate::Data<reflectionzeug::FilePath> densityGenererationShaderFilePath;

protected:
    virtual void process() override;

    void changeToMossy();
    void changeToMoon();
    void changeToUserDefined();
};
