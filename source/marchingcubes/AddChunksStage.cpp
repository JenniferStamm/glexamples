#include "AddChunksStage.h"

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

AddChunksStage::AddChunksStage()
:   AbstractStage("AddChunks")
{
    addInput("camera", camera);

    addOutput("chunksToAdd", chunksToAdd);
}

void AddChunksStage::initialize()
{
    
}

void AddChunksStage::process()
{
    chunksToAdd->clear();

    float distanceForAdding = 4.f;

    auto offset = ivec3(camera.data()->eye() - distanceForAdding);

    for (int z = 0; z < distanceForAdding * 2; ++z)
    {
        for (int y = 0; y < distanceForAdding * 2; ++y)
        {
            for (int x = 0; x < distanceForAdding * 2; ++x)
            {
                auto newOffset = vec3(x, y, z) + vec3(offset);

                chunksToAdd->push_back(newOffset);
            }
        }
    }

    invalidateOutputs();
}

