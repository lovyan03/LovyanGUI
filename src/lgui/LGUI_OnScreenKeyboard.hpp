#ifndef LGUI_ONSCREENKEYBOARD_HPP_
#define LGUI_ONSCREENKEYBOARD_HPP_

#include "LGUI_GridView.hpp"

namespace lgui
{
  class LGUI_OnScreenKeyboard : public LGUI_GridView
  {
  public:
    LGUI_OnScreenKeyboard(void);
    char getKeyCode(void) { char res = _keyCode; _keyCode = 0; return res; }
    void setTarget(LGUI_Base* gui) { _target_gui = gui; }
    LGUI_Base* getTarget(void) { return _target_gui; }

  private:
    LGUI_Base* _target_gui = nullptr;
    int _panel = 0;
    char _keyCode = 0;

    void drawCell_impl(LovyanGFX* canvas, std::int32_t column, std::int32_t row, bool& redraw) override;

    void setup_impl(void) override;
    bool input_impl(input_t& code) override;
    bool touch_impl(touch_t touchstate, LovyanUOM* uom) override;
    bool enterCell_impl(std::int32_t column, std::int32_t row) override;
  };
}

#endif
