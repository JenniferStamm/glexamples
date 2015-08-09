#include <gloperate/plugin/plugin_api.h>

#include "MarchingCubes.h"

#include <glexamples-version.h>

GLOPERATE_PLUGIN_LIBRARY

    GLOPERATE_PAINTER_PLUGIN(MarchingCubes
    , "MarchingCubes"
    , "Implementation of marching cubes algorithm for 3D terrain generation."
    , GLEXAMPLES_AUTHOR_ORGANIZATION
    , "v1.0.0" )

GLOPERATE_PLUGIN_LIBRARY_END
