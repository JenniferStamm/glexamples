#pragma once

#include <queue>

#include <glm/vec3.hpp>
#include <vec3_hash.h>

#include <globjects/base/ref_ptr.h>


#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/input/MouseInputHandler.h>


namespace gloperate
{
    class AbstractCameraCapability;
    class InputCapability;
    class CoordinateProvider;
}

class Chunk;
class ChunkFactory;

class ManageChunksStage : public gloperate::AbstractStage, public gloperate::MouseInputHandler
{
public:
    ManageChunksStage();

    ~ManageChunksStage();
    
    virtual void initialize() override;

public:
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<gloperate::InputCapability *> input;
    gloperate::InputSlot<gloperate::CoordinateProvider *> coordinateProvider;
    gloperate::InputSlot<std::queue<glm::vec3>> chunksToAdd; 
    gloperate::InputSlot<glm::vec3> rotationVector1;
    gloperate::InputSlot<glm::vec3> rotationVector2;
    gloperate::InputSlot<float> warpFactor;
    gloperate::InputSlot<bool> removeFloaters;

    gloperate::Data<std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>>> chunks;

protected:
    void addTerrainAt(glm::vec3 worldPosition);
    virtual void process() override;

public:
    virtual void onMouseMove(int x, int y) override;
    virtual void onMouseRelease(int x, int y, gloperate::MouseButton button) override;
    virtual void onMousePress(int x, int y, gloperate::MouseButton button) override;
protected:
    bool shouldRemoveChunk(glm::vec3 chunkPosition) const;
    
protected:
    globjects::ref_ptr<ChunkFactory> m_chunkFactory;
    std::vector<glm::ivec2> m_mouseClicks;
    bool m_mouseMoved;
    bool m_mousePressed;

};

