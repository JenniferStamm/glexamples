#pragma once

#include <unordered_map>

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/primitives/AbstractDrawable.h>

class Chunk;

namespace globjects
{
    class Program;
    class Texture;
}

class ChunkRenderer : public globjects::Referenced
{

public:
	ChunkRenderer();
    ~ChunkRenderer();

    virtual void render(std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>> chunks);

	void setColorTexture(globjects::ref_ptr<globjects::Texture> colorTexture);
	void setGroundTexture(globjects::ref_ptr<globjects::Texture> groundTexture);

    void setTransform(glm::mat4x4 transform);

	void updateTexture(bool useMipMap);

protected:
    void setupProgram();
    void setupRendering();

protected:
    gl::GLint m_transformLocation;
    gl::GLint m_offsetLocation;

    globjects::ref_ptr<globjects::Program> m_renderProgram;

    glm::mat4x4 m_transform;

	globjects::ref_ptr<globjects::Texture> m_colorTexture;
	globjects::ref_ptr<globjects::Texture> m_groundTexture;
};
