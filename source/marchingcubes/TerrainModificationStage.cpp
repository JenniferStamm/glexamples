#include "TerrainModificationStage.h"

using namespace glm;
using namespace globjects;

TerrainModificationStage::TerrainModificationStage()
    : AbstractStage("TerrainModification")
    , m_ctrlPressed(false)
    , m_mouseMoved(false)
    , m_mousePressed(false)
{
    addInput("input", input);
    addOutput("addPosition", addPosition);
}

TerrainModificationStage::~TerrainModificationStage()
{
    if (input.data())
    {
        input.data()->removeMouseHandler(this);
        input.data()->removeKeyboardHandler(this);
    }
}

void TerrainModificationStage::onMouseMove(int x, int y)
{
    m_mouseMoved = true;
}

void TerrainModificationStage::onMousePress(int x, int y, gloperate::MouseButton button)
{
    m_mousePressed = true;
    m_mouseMoved = false;
}

void TerrainModificationStage::onMouseRelease(int x, int y, gloperate::MouseButton button)
{
    if (m_mousePressed && !m_mouseMoved && m_ctrlPressed)
    {
        addPosition = ivec2(x, y);
    }
    m_mousePressed = false;
}

void TerrainModificationStage::onKeyDown(gloperate::Key key)
{
    if (key == gloperate::Key::KeyLeftControl)
        m_ctrlPressed = true;
}

void TerrainModificationStage::onKeyUp(gloperate::Key key)
{
    if (key == gloperate::Key::KeyLeftControl)
        m_ctrlPressed = false;
}

void TerrainModificationStage::initialize()
{
}

void TerrainModificationStage::process()
{
    if (input.hasChanged())
    {
        input.data()->addMouseHandler(this);
        input.data()->addKeyboardHandler(this);
    }
}