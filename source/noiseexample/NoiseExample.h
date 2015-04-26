#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/painter/Painter.h>


namespace globjects
{
    class Program;
    class Texture;
}

namespace gloperate
{
    class ScreenAlignedQuad;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}

class NoiseExample : public gloperate::Painter
{
public:
    NoiseExample(gloperate::ResourceManager & resourceManager);
    virtual ~NoiseExample();

    void setupProjection();

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;

    void createAndSetupTexture();

protected:
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
    globjects::ref_ptr<globjects::Texture> m_texture;
    gl::GLint m_transformLocation;
};
