#pragma once

#include <glm/vec3.hpp>
#include <vec3_hash.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>


namespace gloperate
{
    class AbstractViewportCapability;
    class AbstractVirtualTimeCapability;
    class PerspectiveProjectionCapability;
    class AbstractCameraCapability;
    class AbstractTargetFramebufferCapability;
    class TypedRenderTargetCapability;
    class AdaptiveGrid;
    class ResourceManager;
}

namespace globjects
{
    class Framebuffer;
    class Renderbuffer;
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
    gloperate::InputSlot<gloperate::ResourceManager *> resourceManager;
    gloperate::InputSlot<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
    gloperate::InputSlot<gloperate::TypedRenderTargetCapability *> renderTargets;
    gloperate::InputSlot<bool> showWireframe;
    gloperate::InputSlot<bool> useMipMap;
    gloperate::InputSlot<bool> useShadow;
    gloperate::InputSlot<bool> useOcclusion;
    gloperate::InputSlot<bool> useGroundTexture;
    gloperate::InputSlot<bool> useStriationTexture;
    
    gloperate::InputSlot<std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>>> chunks;


protected:
    virtual void process() override;

    void render();

    void setupGrid();
    void setupProjection();
    void setupOpenGLState();
    void setupFbo();
    void setupTextures();

    void resizeFbo(int width, int height);


protected:
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Texture> m_colorTexture;
    globjects::ref_ptr<globjects::Texture> m_depthTexture;

    globjects::ref_ptr<globjects::Texture> m_striationTexture;
    globjects::ref_ptr<globjects::Texture> m_groundTexture;

    globjects::ref_ptr<ChunkRenderer> m_chunkRenderer;

	bool m_initialized;

};

