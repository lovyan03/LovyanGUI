#ifndef LGUI_TEXTBOX_HPP_
#define LGUI_TEXTBOX_HPP_

#include "LGUI_Base.hpp"
#include <string>

namespace lgui
{
  class LGUI_TextBox : public LGUI_Base
  {
  public:
    void setText(const char* text);
    const char* getText(void) const;

  protected:

    void setup_impl(void) override;
    bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) override;
    bool input_impl(input_t&) override;
    bool touch_impl(touch_t touchstate, LovyanUOM* uom) override;

  private:
    std::string _data;
    std::int32_t _cursor_pos = 0;
    bool _cursor_drawed;
  };
}

#endif
