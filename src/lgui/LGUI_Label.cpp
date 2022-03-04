#include "LGUI_Label.hpp"
#include "../LovyanGUI.hpp"

namespace lgui
{
  void LGUI_Label::setText(const char* src)
  {
    _data = src;
  }

  const char* LGUI_Label::getText(void) const
  {
    return _data.c_str();
  }

  bool LGUI_Label::loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect)
  {
    bool drawall = (_redraw & redraw_body) || gui->hasInvalidate(_visible_client_rect);
    if (!drawall) return true;

    auto canvas = gui->getCanvas(_visible_client_rect.width(), _visible_client_rect.height());
    canvas->clear(0xFFFF);

    //if (onDraw && onDraw(this, canvas))

    if (!_data.empty())
    {
      canvas->setTextColor(0);
      canvas->setTextDatum(textdatum_t::middle_center);
      canvas->drawString(_data.c_str(), canvas->width()>>1, canvas->height()>>1);
    }
    gui->pushCanvas(_visible_client_rect);

    return true;
  }
}

