#include "AddChunksStage.h"

#include <globjects/globjects.h>

#include <gloperate/primitives/AxisAlignedBoundingBox.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>

using namespace gl;
using namespace glm;
using namespace globjects;

AddChunksStage::AddChunksStage()
:   AbstractStage("AddChunks")
{
    addInput("camera", camera);
    addInput("projection", projection);
    addInput("freezeChunkLoading", freezeChunkLoading);

    addOutput("chunksToAdd", chunksToAdd);
}

void AddChunksStage::initialize()
{
    
}

void AddChunksStage::process()
{
    if (freezeChunkLoading.data())
        return;

    // Clear queue
    std::queue<vec3> empty;
    std::swap(chunksToAdd.data(), empty);

    const auto viewProjectionInverted = camera.data()->viewInverted() * projection.data()->projectionInverted();
    const auto viewProjection = projection.data()->projection() * camera.data()->view();

    // Generate a bounding box for the frustum in world coordinates

    const auto vertices = { vec3(-1, -1, -1), vec3(-1, -1, 1), vec3(-1, 1, -1), vec3(-1, 1, 1), vec3(1, -1, -1), vec3(1, -1, 1), vec3(1, 1, -1), vec3(1, 1, 1) };
    auto frustumBoundingBox = gloperate::AxisAlignedBoundingBox();

    for (const auto vertex : vertices)
    {
        auto result = viewProjectionInverted * vec4(vertex, 1);
        frustumBoundingBox.extend(vec3(result) / result.w);
    }

    // Frustum with padding in camera/projection coordinates
    const auto frustumBoundingBoxInCamera = gloperate::AxisAlignedBoundingBox(vec3(-1.3f, -1.3f, -1.3f), vec3(1.3f, 1.3f, 1.3f));

    vec3 position, realPosition;
    vec4 positionInCamera;

    // Add all chunks within frustum
    for (float z = frustumBoundingBox.llf().z; z <= frustumBoundingBox.urb().z + 1; z += 1.f)
    {
        for (float y = frustumBoundingBox.llf().y; y <= frustumBoundingBox.urb().y + 1; y += 1.f)
        {
            for (float x = frustumBoundingBox.llf().x; x <= frustumBoundingBox.urb().x + 1; x += 1.f)
            {
                position = vec3(x, y, z);
                positionInCamera = viewProjection * vec4(position, 1.f);
                realPosition = vec3(positionInCamera) / positionInCamera.w;

                // Only add those chunks that are within the real frustum, not only the outer bounding box
                if (frustumBoundingBoxInCamera.inside(realPosition))
                {
                    chunksToAdd->push(vec3(floor(position.x), floor(position.y), floor(position.z)));
                }
            }
        }
    }

    invalidateOutputs();
}

