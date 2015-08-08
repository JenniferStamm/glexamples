#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>

#include <reflectionzeug/base/FilePath.h>

#include "TerrainData.h"

class TerrainDataStage : public gloperate::AbstractStage
{
public:
    TerrainDataStage();

    virtual void initialize() override;

public:
    gloperate::InputSlot<TerrainType> terrainType;
    gloperate::InputSlot<reflectionzeug::FilePath> userBaseTextureFilePath;
    gloperate::InputSlot<reflectionzeug::FilePath> userExtraTextureFilePath;

    gloperate::Data<reflectionzeug::FilePath> baseTextureFilePath;
    gloperate::Data<reflectionzeug::FilePath> extraTextureFilePath;

protected:
    virtual void process() override;

    void changeToMossy();
    void changeToUserDefined();
};
