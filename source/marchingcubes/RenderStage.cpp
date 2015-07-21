#include "RenderStage.h"

#include <glm/trigonometric.hpp>

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/globjects.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/Renderbuffer.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/TypedRenderTargetCapability.h>
#include <gloperate/base/RenderTargetType.h>
#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/resources/ResourceManager.h>

#include "ChunkRenderer.h"
#include <glbinding/gl/boolean.h>

using namespace gl;
using namespace glm;
using namespace globjects;

RenderStage::RenderStage()
:   AbstractStage("Render")
, m_chunkRenderer()
, m_initialized(false)
{
    addInput("viewport", viewport);
    addInput("camera", camera);
    addInput("projection", projection);
    addInput("resourceManager", resourceManager);
    addInput("targetFBO", targetFBO);
    addInput("renderTargets", renderTargets);

    addInput("useMipMap", useMipMap);
    addInput("useOcclusion", useOcclusion);
    addInput("useGroundTexture", useGroundTexture);
    addInput("useShadow", useShadow);
    addInput("useStriationTexture", useStriationTexture);


    addInput("chunks", chunks);

	alwaysProcess(true);
}

void RenderStage::initialize()
{

#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    setupGrid();
    setupProjection();
    setupOpenGLState();
    setupFbo();
    setupTextures();

	
	
    m_chunkRenderer = new ChunkRenderer();
	m_chunkRenderer->setGroundTexture(m_groundTexture);
	m_chunkRenderer->setStriationTexture(m_striationTexture);
    m_chunkRenderer->setUseShadow(useShadow.data());
    m_chunkRenderer->setUseOcclusion(useOcclusion.data());
    m_chunkRenderer->setUseGroundTexture(useGroundTexture.data());
    m_chunkRenderer->setUseStriationTexture(useStriationTexture.data());


    renderTargets.data()->setRenderTarget(gloperate::RenderTargetType::Depth, m_fbo,
        gl::GL_DEPTH_STENCIL_ATTACHMENT, gl::GL_DEPTH_COMPONENT);
}

void RenderStage::process()
{
    auto rerender = false;

    if (viewport.hasChanged())
    {
        glViewport(
            viewport.data()->x(),
            viewport.data()->y(),
            viewport.data()->width(),
            viewport.data()->height());
        resizeFbo(viewport.data()->width(), viewport.data()->height());

        rerender = true;
    }

    if (camera.hasChanged() || projection.hasChanged() || targetFBO.hasChanged())
    {
        rerender = true;
    }

    if (useMipMap.hasChanged())
    {
        m_chunkRenderer->updateTexture(useMipMap.data());
        rerender = true;
    }

    if (useShadow.hasChanged())
    {
        m_chunkRenderer->setUseShadow(useShadow.data());
        rerender = true;
    }

    if (useOcclusion.hasChanged())
    {
        m_chunkRenderer->setUseOcclusion(useOcclusion.data());
        rerender = true;
    }

    if (useGroundTexture.hasChanged())
    {
        m_chunkRenderer->setUseGroundTexture(useGroundTexture.data());
        rerender = true;
    }

    if (useStriationTexture.hasChanged())
    {
        m_chunkRenderer->setUseStriationTexture(useStriationTexture.data());
        rerender = true;
    }

    if (chunks.hasChanged())
    {
        rerender = true;
    }

    if (rerender)
    {
        render();

        invalidateOutputs();
    }

    //m_fbo->bind();


    std::array<int, 4> sourceRect = { { viewport.data()->x(), viewport.data()->y(), viewport.data()->width(), viewport.data()->height() } };
    std::array<int, 4> destRect = { { viewport.data()->x(), viewport.data()->y(), viewport.data()->width(), viewport.data()->height() } };

    globjects::Framebuffer * destFbo = targetFBO.data()->framebuffer() ? targetFBO.data()->framebuffer() : globjects::Framebuffer::defaultFBO();

        
    m_fbo->blit(gl::GL_COLOR_ATTACHMENT0, sourceRect, destFbo, destFbo->id() == 0 ? gl::GL_BACK_LEFT : gl::GL_COLOR_ATTACHMENT0, destRect, gl::GL_COLOR_BUFFER_BIT, gl::GL_NEAREST);
    m_fbo->blit(gl::GL_DEPTH_STENCIL_ATTACHMENT, sourceRect, destFbo, destFbo->id() == 0 ? gl::GL_BACK_LEFT : gl::GL_DEPTH_STENCIL_ATTACHMENT, destRect, gl::GL_DEPTH_BUFFER_BIT, gl::GL_NEAREST);

    //m_fbo->unbind();

}

void RenderStage::render()
{
    m_fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto transform = projection.data()->projection() * camera.data()->view();
    const auto eye = camera.data()->eye();
	
    m_chunkRenderer->setTransform(transform);

    m_chunkRenderer->render(chunks.data());
	
    m_grid->update(eye, transform);
    m_grid->draw();

    m_fbo->unbind();
}

void RenderStage::setupGrid()
{
    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({ 0.6f, 0.6f, 0.6f });
}

void RenderStage::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 100.f, fovy = 50.f;

    projection.data()->setZNear(zNear);
    projection.data()->setZFar(zFar);
    projection.data()->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void RenderStage::setupOpenGLState()
{
    glClearColor(0.f, 0.f, 0.2f, 1.0f);
    glEnable(GL_CULL_FACE);
}

void RenderStage::setupFbo()
{
    static const auto createTexture = [](const std::string & name)
    {
        auto tex = Texture::createDefault(GL_TEXTURE_2D);
        tex->setName(name);
        return tex;
    };

    m_colorTexture = createTexture("Color Texture");
    m_depthTexture = createTexture("Depth Texture");
    m_fbo = make_ref<globjects::Framebuffer>();
    m_fbo->setName("Render FBO");

    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, m_colorTexture);
    m_fbo->attachTexture(GL_DEPTH_STENCIL_ATTACHMENT, m_depthTexture);

}

void RenderStage::setupTextures()
{
    if (resourceManager.data())
    {
        m_groundTexture = resourceManager.data()->load<Texture>("data/marchingcubes/ground.png");

        m_groundTexture->setName("GroundTexture");
        m_groundTexture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
        m_groundTexture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

        m_striationTexture = resourceManager.data()->load<Texture>("data/marchingcubes/terrain_color.jpg");

        m_striationTexture->setName("StriationTexture");
        m_striationTexture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
        m_striationTexture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        
    }

}

void RenderStage::resizeFbo(int width, int height)
{
    m_colorTexture->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_depthTexture->image2D(0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);

    m_fbo->printStatus(true);
}
