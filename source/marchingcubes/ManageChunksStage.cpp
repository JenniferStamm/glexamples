#include "ManageChunksStage.h"

#include <globjects/globjects.h>

#include <gloperate/painter/AbstractCameraCapability.h>

#include "Chunk.h"
#include "ChunkFactory.h"

using namespace gl;
using namespace glm;
using namespace globjects;

ManageChunksStage::ManageChunksStage()
:   AbstractStage("ManageChunks")
, m_chunkFactory()
{
    addInput("camera", camera);
    addInput("chunksToAdd", chunksToAdd);
    addInput("rotationVector1", rotationVector1);
    addInput("rotationVector2", rotationVector2);
    addInput("warpFactor", warpFactor);

    addOutput("chunks", chunks);

    alwaysProcess(true);
}

void ManageChunksStage::initialize()
{
    m_chunkFactory = new ChunkFactory();
}

void ManageChunksStage::process()
{
    auto regenerate = false;

    if (rotationVector1.hasChanged())
    {
        m_chunkFactory->densityGenerationProgram()->setUniform("rotationVector1", rotationVector1.data());
        regenerate = true;
    }

    if (rotationVector2.hasChanged())
    {
        m_chunkFactory->densityGenerationProgram()->setUniform("rotationVector2", rotationVector2.data());
        regenerate = true;
    }

    if (warpFactor.hasChanged())
    {
        m_chunkFactory->densityGenerationProgram()->setUniform("warpFactor", warpFactor.data());
        regenerate = true;
    }

    if (regenerate)
    {
        chunks->clear();
    }

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
        m_chunkFactory->generateDensities(newChunk);
        m_chunkFactory->generateList(newChunk);
        if (!newChunk->isEmpty())
            m_chunkFactory->generateMesh(newChunk);

        chunks.data()[newOffset] = newChunk;

        if (!newChunk->isEmpty())
            ++i;
    }

    invalidateOutputs();
}

