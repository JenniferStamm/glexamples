#pragma once

#include <unordered_map>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

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

	void setStriationTexture(globjects::ref_ptr<globjects::Texture> striationTexture);
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

	globjects::ref_ptr<globjects::Texture> m_striationTexture;
	globjects::ref_ptr<globjects::Texture> m_groundTexture;
};
