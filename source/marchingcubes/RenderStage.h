#pragma once

#include <vec3_hash.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>

#include <reflectionzeug/base/FilePath.h>


namespace gloperate
{
    class AbstractCameraCapability;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractVirtualTimeCapability;
    class AdaptiveGrid;
    class PerspectiveProjectionCapability;
    class ResourceManager;
    class TypedRenderTargetCapability;
}

namespace globjects
{
    class Framebuffer; 
    class Program;
    class Renderbuffer;
    class Texture;
}

class Chunk;

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
    gloperate::InputSlot<reflectionzeug::FilePath> groundTextureFilePath;
    gloperate::InputSlot<bool> useGroundTexture;
    gloperate::InputSlot<reflectionzeug::FilePath> striationTextureFilePath;
    gloperate::InputSlot<bool> useStriationTexture;
    
    gloperate::InputSlot<std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>>> chunks;


protected:
    virtual void process() override;

    void drawChunks(
        const glm::vec3 & eye,
        const glm::mat4 & transform);
    void render();

    void setupGrid();
    void setupProjection();
    void setupOpenGLState();
    void setupFbo();
    void setupTextures();
    void setupGroundTexture();
    void setupStriationTexture();
    void setupProgram();
    void setupRendering();

    void resizeFbo(int width, int height);
    void updateTexture();


protected:
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Texture> m_colorTexture;
    globjects::ref_ptr<globjects::Texture> m_depthTexture;

    globjects::ref_ptr<globjects::Texture> m_striationTexture;
    globjects::ref_ptr<globjects::Texture> m_groundTexture;

    globjects::ref_ptr<globjects::Program> m_renderProgram;

    gl::GLint m_transformLocation;
    gl::GLint m_offsetLocation;

	bool m_initialized;

};

