#pragma once

#include <gloperate/input/MouseInputHandler.h>
#include <gloperate/input/KeyboardInputHandler.h>
#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <glm/vec2.hpp>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/painter/InputCapability.h>

class TerrainModificationStage : public gloperate::MouseInputHandler, public gloperate::KeyboardInputHandler, public gloperate::AbstractStage
{
public:
    TerrainModificationStage();
    ~TerrainModificationStage();

    gloperate::InputSlot<gloperate::InputCapability *> input;
    gloperate::Data<glm::ivec2> addPosition;
    gloperate::Data<glm::ivec2> removePosition;

    virtual void onMouseMove(int x, int y) override;
    virtual void onMousePress(int x, int y, gloperate::MouseButton button) override;
    virtual void onMouseRelease(int x, int y, gloperate::MouseButton button) override;
    virtual void onKeyDown(gloperate::Key key) override;
    virtual void onKeyUp(gloperate::Key key) override;
    virtual void initialize() override;
protected:
    virtual void process() override;
protected:
    bool m_ctrlPressed;
    gloperate::MouseButton m_mouseButton;
    bool m_mouseMoved;
    bool m_mousePressed;
    
};
