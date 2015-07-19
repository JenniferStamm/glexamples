#pragma once

#include <queue>

#include <glm/vec3.hpp>
#include <vec3_hash.h>

#include <globjects/base/ref_ptr.h>


#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>


namespace gloperate
{
    class AbstractViewportCapability;
    class PerspectiveProjectionCapability;
    class AbstractCameraCapability;
}
class Chunk;


class ManageChunksStage : public gloperate::AbstractStage
{
public:
    ManageChunksStage();

    virtual void initialize() override;

public:
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<std::vector<glm::vec3>> chunksToAdd;

    gloperate::Data<std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>>> chunks;

protected:
    virtual void process() override;

    std::queue<glm::vec3> m_chunkQueue;

};

