#ifndef LGUI_SCROLLABLE_HPP_
#define LGUI_SCROLLABLE_HPP_

#include "LGUI_Base.hpp"

namespace lgui
{
  class LGUI_Scrollable : public LGUI_Base
  {
  friend LGUI_Base;
  public:
    bool visibleVScroll = false;
    bool visibleHScroll = false;
    std::int32_t scrollBarWidth = 8;

  protected:
    std::int32_t _vscroll_size = 0;
    std::int32_t _vscroll_move = 0;
    std::int32_t _vscroll_dest = 0;

    std::int32_t _hscroll_size = 0;
    std::int32_t _hscroll_move = 0;
    std::int32_t _hscroll_dest = 0;

    bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) override;
    bool touch_impl(touch_t touchstate, LovyanUOM* uom) override;

    void update_client_rect_impl(void) override;

    virtual void update_scroll_impl(void) {}
  };
}

#endif
