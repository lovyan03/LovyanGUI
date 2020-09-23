#include "LGUI_Scrollable.hpp"
#include "LovyanGUI.hpp"
#include <LovyanGFX.hpp>

namespace lgui
{
  static void draw_scroll(bool vertical, LovyanGUI* gui, std::int32_t pos, std::int32_t size, Rectangle& rect)
  {
    auto canvas = gui->getCanvas(rect.width(), rect.height());
    canvas->clear( gui->color.border );
//canvas->clear(esp_random());
    canvas->setColor(gui->color.inactiveCursor);

    auto viewsize = vertical ? canvas->height() : canvas->width();
    if (size <= 0) size = 1;
    std::int32_t top    = pos * viewsize / size;
    std::int32_t bottom = (pos + viewsize) * viewsize / size;

    if (vertical) canvas->fillRect(1, top, canvas->width()-1, bottom - top);
    else          canvas->fillRect(top, 1, bottom - top, canvas->height()-1);
    gui->pushCanvas(rect);
  }


  bool LGUI_Scrollable::loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect)
  {
    // スクロール処理
    update_scroll_impl();

    bool hscroll_redraw = _hscroll_move != _hscroll_dest;
    if (hscroll_redraw)
    {
      _hscroll_move = Rectangle::mixPos(_hscroll_move, _hscroll_dest, gui->smoothMove, 16);
      _redraw = static_cast<redraw_t>(_redraw | redraw_body);
    }
    hscroll_redraw |= _redraw & redraw_hscroll;

    bool vscroll_redraw = _vscroll_move != _vscroll_dest;
    if (vscroll_redraw)
    {
      _vscroll_move = Rectangle::mixPos(_vscroll_move, _vscroll_dest, gui->smoothMove, 16);
      _redraw = static_cast<redraw_t>(_redraw | redraw_body);
    }
    vscroll_redraw |= _redraw & redraw_vscroll;

    if (visibleHScroll || visibleVScroll)
    {
      if (visibleHScroll && visibleVScroll)
      {
        Rectangle rect = { _visible_client_rect.right + 1, _visible_client_rect.bottom + 1, _visible_client_rect.right + scrollBarWidth, _visible_client_rect.bottom + scrollBarWidth };
        rect.intersect(cliprect);
        fill_rect(gui, rect, _redraw & (redraw_body | redraw_right | redraw_bottom), gui->color.frame[has_focus]);
      }

      if (visibleHScroll) {
        auto rect = _visible_client_rect;
        rect.top  = rect.bottom + 1;
        rect.bottom += scrollBarWidth;

        if (hscroll_redraw || gui->hasInvalidate(rect))
        {
          draw_scroll(false, gui, _hscroll_move, _hscroll_size, rect);
        }
      }
      if (visibleVScroll) {
        auto rect = _visible_client_rect;
        rect.left  = rect.right + 1;
        rect.right += scrollBarWidth;
        if (vscroll_redraw || gui->hasInvalidate(rect))
        {
          draw_scroll(true, gui, _vscroll_move, _vscroll_size, rect);
        }
      }
    }

//    if (vscroll_redraw || hscroll_redraw) invalidate();

    return LGUI_Base::loop_impl(gui, has_focus, cliprect);
  }

  void LGUI_Scrollable::update_client_rect_impl(void)
  {
    LGUI_Base::update_client_rect_impl();

    _visible_client_rect.bottom -= scrollBarWidth * visibleHScroll;
    _visible_client_rect.right -= scrollBarWidth * visibleVScroll;
  }

  bool LGUI_Scrollable::touch_impl(touch_t touchstate, LovyanUOM* uom)
  {
    if (touchstate & touch_t::mask_moving)
    {
      std::int32_t dx, dy;
      uom->getTouchDeltaXY(&dx, &dy);
      if (dx == 0 && dy == 0) return true;

      auto hscroll = std::max<std::int32_t>(std::min(_hscroll_dest - dx, _hscroll_size - _visible_client_rect.width()), 0);
      auto vscroll = std::max<std::int32_t>(std::min(_vscroll_dest - dy, _vscroll_size - _visible_client_rect.height()), 0);
      if (_hscroll_dest == hscroll && _vscroll_dest == vscroll && _parent)
      {
        _parent->touch(touchstate, uom);
        return true;
      }
      if (_hscroll_dest != hscroll)
      {
        _hscroll_dest = hscroll;
        _hscroll_move = hscroll;
        _redraw = static_cast<redraw_t>(_redraw | redraw_hscroll | redraw_body);
      }
      if (_vscroll_dest != vscroll)
      {
        _vscroll_dest = vscroll;
        _vscroll_move = vscroll;
        _redraw = static_cast<redraw_t>(_redraw | redraw_vscroll | redraw_body);
      }
    }
    return true;
  }
}
