#include "ManageChunksStage.h"

#include <glm/trigonometric.hpp>

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/globjects.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/Camera.h>
#include <gloperate/primitives/AdaptiveGrid.h>

#include "Chunk.h"
#include "ChunkRenderer.h"

using namespace gl;
using namespace glm;
using namespace globjects;

ManageChunksStage::ManageChunksStage()
:   AbstractStage("ManageChunks")
{
    addInput("camera", camera);
    addInput("chunksToAdd", chunksToAdd);

    addOutput("chunks", chunks);
}

void ManageChunksStage::initialize()
{
    
}

void ManageChunksStage::process()
{
    // Remove unneeded chunks

    float distanceForRemoving = 7.f;

    std::vector<vec3> chunksToRemove;

    for (auto chunk : chunks.data())
    {
        auto currentOffset = chunk.first;
        if (distance(currentOffset, camera.data()->eye()) > distanceForRemoving)
        {
            chunksToRemove.push_back(chunk.first);
        }
    }

    for (auto chunkToRemove : chunksToRemove)
    {
        chunks->erase(chunkToRemove);
    }

    // Add to queue
    for (auto chunkToAdd : chunksToAdd.data())
    {
        m_chunkQueue.push(chunkToAdd);
    }


    // Generate new non-empty chunks
    const unsigned int chunksToGenerate = 3u;

    for (int i = 0; i < chunksToGenerate;)
    {
        if (m_chunkQueue.empty())
            break;
        glm::vec3 newOffset = m_chunkQueue.front();
        m_chunkQueue.pop();
        while (chunks->find(newOffset) != chunks->end())
        {
            if (m_chunkQueue.empty())
                break;
            newOffset = m_chunkQueue.front();
            m_chunkQueue.pop();
        }

        // Don't add chunk if it was already generated
        if (chunks->find(newOffset) != chunks->end())
            continue;

        auto newChunk = new Chunk(newOffset);
        m_chunkRenderer->generateDensities(newChunk);
        m_chunkRenderer->generateList(newChunk);
        if (!newChunk->isEmpty())
            m_chunkRenderer->generateMesh(newChunk);

        m_chunks[newOffset] = newChunk;

        if (!newChunk->isEmpty())
            ++i;
    }

    invalidateOutputs();
}

