#include "ManageChunksStage.h"

#include <globjects/globjects.h>

#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/navigation/CoordinateProvider.h>

#include "Chunk.h"
#include "ChunkFactory.h"

using namespace gl;
using namespace glm;
using namespace globjects;

ManageChunksStage::ManageChunksStage()
:   AbstractStage("ManageChunks")
, m_chunkFactory()
, m_allChunksGenerated(true)
{
    addInput("camera", camera);
    addInput("coordinateProvider", coordinateProvider);
    addInput("addPosition", addPosition);
    addInput("chunksToAdd", chunksToAdd);
    addInput("rotationVector1", rotationVector1);
    addInput("rotationVector2", rotationVector2);
    addInput("warpFactor", warpFactor);
    addInput("removeFloaters", removeFloaters);
    addInput("freezeChunkLoading", freezeChunkLoading);

    addOutput("chunks", chunks);

    alwaysProcess(true);
}

ManageChunksStage::~ManageChunksStage()
{
    
}

void ManageChunksStage::initialize()
{
    m_chunkFactory = new ChunkFactory();
    
}

void ManageChunksStage::addTerrainAt(glm::vec3 worldPosition)
{
    vec3 chunkOffset = vec3(floor(worldPosition[0]), floor(worldPosition[1]), floor(worldPosition[2]));
    for (int z = -1; z <= 1; ++z)
        for (int y = -1; y <= 1; ++y)
            for (int x = -1; x <= 1; ++x)
            {
                auto chunk = chunks->find(chunkOffset + vec3(x, y, z));
                if (chunk != chunks->end())
                {
                    chunk->second->addTerrainPosition(worldPosition);
                }
            }
}

void ManageChunksStage::removeChunks()
{
    // Remove unneeded chunks
    std::vector<vec3> chunksToRemove;

    for (auto chunk : chunks.data())
    {
        if (shouldRemoveChunk(chunk.first))
        {
            m_chunksChanged = true;
            chunksToRemove.push_back(chunk.first);
        }
    }

    for (auto chunkToRemove : chunksToRemove)
    {
        chunks->erase(chunkToRemove);
    }
}

void ManageChunksStage::process()
{
    auto regenerate = false;
    m_chunksChanged = false;

    if (addPosition.hasChanged())
    {
        auto worldPosition = coordinateProvider.data()->worldCoordinatesAt(addPosition.data());
        addTerrainAt(worldPosition);
        m_chunksChanged = true;
    }

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
        m_chunksChanged = true;
    }

    if (!freezeChunkLoading.data())
        removeChunks();

    if (chunksToAdd.hasChanged())
        m_chunksChanged = true;

    if (!m_allChunksGenerated)
        m_chunksChanged = true;

    if (m_chunksChanged)
    {
        regenerateChunks();
        invalidateOutputs();
    }
}

bool ManageChunksStage::shouldRemoveChunk(glm::vec3 chunkPosition) const
{
    float distanceForRemoving = 10.f;

    return distance(chunkPosition, camera.data()->eye()) > distanceForRemoving;
}

void ManageChunksStage::regenerateChunks()
{
    m_allChunksGenerated = false;

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
            return;

        if (chunk.second->isValid())
            continue;

        auto currentChunk = chunk.second;


        m_chunkFactory->generateDensities(currentChunk);
        m_chunkFactory->generateList(currentChunk);
        if (!currentChunk->isEmpty())
            m_chunkFactory->generateMesh(currentChunk);

        currentChunk->setValid(true);

        if (!currentChunk->isEmpty())
            ++generatedChunks;
    }

    m_allChunksGenerated = true;

}