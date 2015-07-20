#include "AddChunksStage.h"

#include <globjects/globjects.h>

#include <gloperate/painter/AbstractCameraCapability.h>

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
    std::queue<vec3> empty;
    std::swap(chunksToAdd.data(), empty);

    int directionMax = 8;
    int upMax = 3;
    int rightMax = 4;

    auto startPosition = camera.data()->eye();

    auto direction = normalize(camera.data()->center() - camera.data()->eye());
    auto up = normalize(camera.data()->up());
    auto right = normalize(cross(up, direction));

    for (int z = 0; z < directionMax; ++z)
    {
        for (int y = -upMax; y < upMax; ++y)
        {
            for (int x = -rightMax; x < rightMax; ++x)
            {
                auto newOffset = ivec3(startPosition + right * float(x) + up * float(y) + direction * float(z));

                chunksToAdd->push(vec3(newOffset));
            }
        }
    }

    invalidateOutputs();
}

