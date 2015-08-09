#include <gloperate/plugin/plugin_api.h>

#include "MarchingCubes.h"

#include <glexamples-version.h>

GLOPERATE_PLUGIN_LIBRARY

    GLOPERATE_PAINTER_PLUGIN(MarchingCubes
    , "MarchingCubes"
    , "Copy to implement a new example"
    , GLEXAMPLES_AUTHOR_ORGANIZATION
    , "v1.0.0" )

GLOPERATE_PLUGIN_LIBRARY_END
