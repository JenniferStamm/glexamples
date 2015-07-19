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
    class AbstractVirtualTimeCapability;
    class PerspectiveProjectionCapability;
    class AbstractCameraCapability;
    class AbstractTargetFramebufferCapability;
    class AdaptiveGrid;
}

namespace globjects
{
    class Framebuffer;
    class Texture;
}

class Chunk;
class ChunkRenderer;


class RenderStage : public gloperate::AbstractStage
{
public:
	RenderStage();

    virtual void initialize() override;

public:
    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<gloperate::PerspectiveProjectionCapability *> projection;
    gloperate::InputSlot<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
    gloperate::InputSlot<bool> useMipMap;
    gloperate::InputSlot<glm::vec3> rotationVector1;
    gloperate::InputSlot<glm::vec3> rotationVector2;
    gloperate::InputSlot<float> warpFactor;
	gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> colorTexture;
	gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> groundTexture;


protected:
    virtual void process() override;

    void render();

    void setupGrid();
    void setupProjection();
    void setupOpenGLState();
    void setupFbo();

    void resizeFbo(int width, int height);


protected:
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Texture> m_colorTexture;
    globjects::ref_ptr<globjects::Texture> m_depthTexture;

    std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>> m_chunks;
    globjects::ref_ptr<ChunkRenderer> m_chunkRenderer;
    std::queue<glm::vec3> m_chunkQueue;

	bool m_initialized;

};

