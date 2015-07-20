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
    addInput("removeFloaters", removeFloaters);

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

    if (removeFloaters.hasChanged())
    {
        m_chunkFactory->setRemoveFloaters(removeFloaters.data());
        regenerate = true;
    }

    if (regenerate)
    {
        for (auto chunk : chunks.data())
        {
            chunk.second->setValid(false);
        }
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

    vec3 newOffset;

    while (!localChunksToAdd.empty())
    {
        newOffset = localChunksToAdd.front();
        localChunksToAdd.pop();
        // Break if it is a new chunk
        if (!shouldRemoveChunk(newOffset) && chunks->find(newOffset) == chunks->end())
        {
            auto newChunk = new Chunk(newOffset);
            chunks.data()[newOffset] = newChunk;
        }
    }

    // Generate new non-empty chunks
    const unsigned int chunksToGenerate = 3u;
    unsigned int generatedChunks = 0;
    for (auto chunk : chunks.data())
    {       
        if (generatedChunks >= chunksToGenerate)
            break;

        if (chunk.second->isValid())
            continue;

        chunksChanged = true;

        auto currentChunk = chunk.second;


        m_chunkFactory->generateDensities(currentChunk);
        m_chunkFactory->generateList(currentChunk);
        if (!currentChunk->isEmpty())
            m_chunkFactory->generateMesh(currentChunk);

        currentChunk->setValid(true);

        if (!currentChunk->isEmpty())
            ++generatedChunks;
    }

    if (chunksChanged)
        invalidateOutputs();
}

bool ManageChunksStage::shouldRemoveChunk(glm::vec3 chunkPosition) const
{
    float distanceForRemoving = 10.f;

    return distance(chunkPosition, camera.data()->eye()) > distanceForRemoving;
}