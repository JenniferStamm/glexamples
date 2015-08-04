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
    class AbstractCameraCapability;
    class InputCapability;
    class CoordinateProvider;
}

class Chunk;
class ChunkFactory;

class ManageChunksStage : public gloperate::AbstractStage
{
public:
    ManageChunksStage();

    ~ManageChunksStage();
    
    virtual void initialize() override;

public:
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<gloperate::CoordinateProvider *> coordinateProvider;
    gloperate::InputSlot<glm::ivec2> addPosition;
    gloperate::InputSlot<std::queue<glm::vec3>> chunksToAdd; 
    gloperate::InputSlot<glm::vec3> rotationVector1;
    gloperate::InputSlot<glm::vec3> rotationVector2;
    gloperate::InputSlot<float> warpFactor;
    gloperate::InputSlot<bool> removeFloaters;
    gloperate::InputSlot<bool> freezeChunkLoading;
    gloperate::InputSlot<float> modificationRadius;

    gloperate::Data<std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>>> chunks;

protected:
    void addTerrainAt(glm::vec3 worldPosition);
    void removeChunks();
    virtual void process() override;

    bool shouldRemoveChunk(glm::vec3 chunkPosition) const;
    void regenerateChunks();
    
protected:
    globjects::ref_ptr<ChunkFactory> m_chunkFactory;

    bool m_chunksChanged;
    bool m_allChunksGenerated;

};

