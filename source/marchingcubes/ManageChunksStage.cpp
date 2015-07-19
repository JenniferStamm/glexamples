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
    auto chunksChanged = false;

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
        chunksChanged = true;
    }

    // Remove unneeded chunks
    std::vector<vec3> chunksToRemove;

    for (auto chunk : chunks.data())
    {
        if (shouldRemoveChunk(chunk.first))
        {
            chunksChanged = true;
            chunksToRemove.push_back(chunk.first);
        }
    }

    for (auto chunkToRemove : chunksToRemove)
    {
        chunks->erase(chunkToRemove);
    }

    // Copy chunk list
    auto localChunksToAdd = chunksToAdd.data();

    // Generate new non-empty chunks
    const unsigned int chunksToGenerate = 3u;

    for (int i = 0; i < chunksToGenerate;)
    {       
        bool newChunkFound = false;
        vec3 newOffset;

        while (!localChunksToAdd.empty())
        {
            newOffset = localChunksToAdd.front();
            localChunksToAdd.pop();
            // Break if it is a new chunk
            if (chunks->find(newOffset) == chunks->end())
            {
                newChunkFound = true;
                break;
            }
        }

        if (!newChunkFound)
            break;

        chunksChanged = true;

        auto newChunk = new Chunk(newOffset);
        m_chunkFactory->generateDensities(newChunk);
        m_chunkFactory->generateList(newChunk);
        if (!newChunk->isEmpty())
            m_chunkFactory->generateMesh(newChunk);

        chunks.data()[newOffset] = newChunk;

        if (!newChunk->isEmpty())
            ++i;
    }

    if (chunksChanged)
        invalidateOutputs();
}

bool ManageChunksStage::shouldRemoveChunk(glm::vec3 chunkPosition) const
{
    float distanceForRemoving = 10.f;

    return distance(chunkPosition, camera.data()->eye()) > distanceForRemoving;
}