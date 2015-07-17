#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/primitives/AbstractDrawable.h>

namespace globjects
{
    class Program;
    class Buffer;
    class Texture;
    class TransformFeedback;
    class VertexArray;
}

class Chunk : public gloperate::AbstractDrawable, public globjects::Referenced
{
public:
    Chunk(glm::vec3 offset);
    ~Chunk();

    virtual void draw() override;

    glm::vec3 offset() const { return m_offset; }
    globjects::ref_ptr<globjects::Buffer> densities() const { return m_densities; }
    globjects::ref_ptr<globjects::Buffer> list() const { return m_list; }
    globjects::ref_ptr<globjects::Buffer> vertexPositions() const { return m_vertexPositions; }
    globjects::ref_ptr<globjects::Buffer> vertexNormals() const { return m_vertexNormals; }
    unsigned int triangleCount() const { return m_triangleCount; }
    void setTriangleCount(unsigned int triangleCount);
    bool isEmpty() const { return m_isEmpty; }
    void setupDensityGeneration(unsigned int densitySize);
    void teardownDensityGeneration();
    void setupListGeneration(unsigned int verticesSize);
    void teardownListGeneration();
    void setupMeshGeneration(globjects::VertexArray * meshVao);
    void teardownMeshGeneration();

protected:
    void setupRendering();

    globjects::ref_ptr<globjects::Buffer> m_densities;
    globjects::ref_ptr<globjects::Texture> m_densitiesTexture;

    globjects::ref_ptr<globjects::Buffer> m_list;

    globjects::ref_ptr<globjects::Buffer> m_vertexPositions;
    globjects::ref_ptr<globjects::Buffer> m_vertexNormals;

    glm::vec3 m_offset;

    unsigned int m_triangleCount;

    bool m_isEmpty;
};
