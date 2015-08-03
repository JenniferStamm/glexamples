#include "RenderStage.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/boolean.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Framebuffer.h>
#include <globjects/globjects.h>
#include <globjects/Program.h>
#include <globjects/Renderbuffer.h>
#include <globjects/Texture.h>

#include <gloperate/base/RenderTargetType.h>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/TypedRenderTargetCapability.h>
#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/resources/ResourceManager.h>

#include <loggingzeug/logging.h>

#include "Chunk.h"

using namespace gl;
using namespace glm;
using namespace globjects;

RenderStage::RenderStage()
:   AbstractStage("Render")
, m_initialized(false)
{
    addInput("viewport", viewport);
    addInput("camera", camera);
    addInput("projection", projection);
    addInput("resourceManager", resourceManager);
    addInput("targetFBO", targetFBO);
    addInput("renderTargets", renderTargets);

    addInput("showWireframe", showWireframe);
    addInput("useMipMap", useMipMap);
    addInput("useOcclusion", useOcclusion);
    addInput("groundTextureFilePath", groundTextureFilePath);
    addInput("useGroundTexture", useGroundTexture);
    addInput("useShadow", useShadow);
    addInput("striationTextureFilePath", striationTextureFilePath);
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
    setupProgram();
    setupRendering();

    renderTargets.data()->setRenderTarget(gloperate::RenderTargetType::Depth, m_fbo,
        gl::GL_DEPTH_STENCIL_ATTACHMENT, gl::GL_DEPTH_COMPONENT);
}

void RenderStage::process()
{
    auto rerender = false;

    if (groundTextureFilePath.hasChanged())
    {
        setupGroundTexture();
        rerender = true;
    }

    if (striationTextureFilePath.hasChanged())
    {
        setupStriationTexture();
        rerender = true;
    }

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

    if (showWireframe.hasChanged())
    {
        glPolygonMode(GL_FRONT_AND_BACK, showWireframe.data() ? GL_LINE : GL_FILL);
        rerender = true;
    }

    if (useMipMap.hasChanged())
    {
        updateTexture();
        rerender = true;
    }

    if (useShadow.hasChanged())
    {
        rerender = true;
    }

    if (useOcclusion.hasChanged())
    {
        rerender = true;
    }

    if (useGroundTexture.hasChanged())
    {
        rerender = true;
    }

    if (useStriationTexture.hasChanged())
    {
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

void RenderStage::drawChunks(const glm::vec3 & eye, const glm::mat4 & transform)
{
    m_groundTexture->bindActive(GL_TEXTURE0);
    m_striationTexture->bindActive(GL_TEXTURE1);

    m_renderProgram->use();
    m_renderProgram->setUniform(m_transformLocation, transform);
    m_renderProgram->setUniform("useShadow", useShadow.data());
    m_renderProgram->setUniform("useOcclusion", useOcclusion.data());
    m_renderProgram->setUniform("useGroundTexture", useGroundTexture.data());
    m_renderProgram->setUniform("useStriationTexture", useStriationTexture.data());


    for (auto chunk : chunks.data())
    {
        m_renderProgram->setUniform(m_offsetLocation, chunk.first);
        chunk.second->draw();
    }

    m_renderProgram->release();
    m_groundTexture->unbind();
    m_striationTexture->unbind();
}

void RenderStage::render()
{
    m_fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto transform = projection.data()->projection() * camera.data()->view();
    const auto eye = camera.data()->eye();

    drawChunks(eye, transform);
	
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
        setupGroundTexture();
        setupStriationTexture();
    }

}

void RenderStage::setupGroundTexture()
{
    m_groundTexture = resourceManager.data()->load<Texture>(groundTextureFilePath->toString());
    if (!m_groundTexture)
    {
        loggingzeug::critical() << "Could not load Ground Texture";
    }

    m_groundTexture->setName("GroundTexture");
    m_groundTexture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_groundTexture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void RenderStage::setupStriationTexture()
{
    m_striationTexture = resourceManager.data()->load<Texture>(striationTextureFilePath->toString());
    if (!m_striationTexture)
    {
        loggingzeug::critical() << "Could not load Striation Texture";
    }

    m_striationTexture->setName("StriationTexture");
    m_striationTexture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_striationTexture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void RenderStage::setupProgram()
{
    m_renderProgram = new Program{};
    m_renderProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/marchingcubes.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/marchingcubes/marchingcubes.frag")
        );

    m_transformLocation = m_renderProgram->getUniformLocation("transform");
    m_offsetLocation = m_renderProgram->getUniformLocation("a_offset");
}

void RenderStage::setupRendering()
{
    m_renderProgram->setUniform("ground", 0);
    m_renderProgram->setUniform("striation", 1);
}

void RenderStage::resizeFbo(int width, int height)
{
    m_colorTexture->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_depthTexture->image2D(0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);

    m_fbo->printStatus(true);
}

void RenderStage::updateTexture()
{
    if (useMipMap.data())
    {
        m_groundTexture->generateMipmap();
        m_groundTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        m_striationTexture->generateMipmap();
        m_striationTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        m_groundTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        m_striationTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    m_groundTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_striationTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
