#include "LGUI_TextBox.hpp"
#include "../LovyanGUI.hpp"

namespace lgui
{
  void LGUI_TextBox::setText(const char* src)
  {
    _data = src;
    _cursor_pos = _data.size();
  }

  const char* LGUI_TextBox::getText(void) const
  {
    return _data.c_str();
  }

  void LGUI_TextBox::setup_impl(void)
  {
  }

  bool LGUI_TextBox::loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect)
  {
    bool drawall = _redraw || gui->hasInvalidate(_visible_client_rect);
//drawall |= _invalidated;

    if (_cursor_drawed != (has_focus && (gui->getGFX()->isEPD() || (gui->getMsec() & 0x180))))
    {
      _cursor_drawed = !_cursor_drawed;
      drawall = true;
    }

    if (!drawall) return true;

    auto canvas = gui->getCanvas(_visible_client_rect.width(), _visible_client_rect.height());
    canvas->clear(0xFFFF);
    auto cursor_x = 3;
    if (_cursor_pos) cursor_x = canvas->textWidth(_data.substr(0, _cursor_pos).c_str()) + 3;

    auto ch = canvas->height();

    if (_cursor_drawed)
    {
      canvas->setColor(gui->color.cursor[has_focus]);
      auto fh = canvas->fontHeight();
      canvas->fillRect(cursor_x, (ch - fh) >> 1, 2, fh);
    }
    if (!_data.empty())
    {
      canvas->setTextColor(0);
      canvas->setTextDatum(textdatum_t::middle_left);
      canvas->drawString(_data.c_str(), 3, ch >> 1);
    }
    //gfx->setClipRect(cliprect.left, cliprect.top, cliprect.width(), cliprect.height());
    gui->pushCanvas(_visible_client_rect);
    //gfx->clearClipRect();

    return true;
  }

  bool LGUI_TextBox::input_impl(input_t& code)
  {
    if ((char)code == 0x08)
    { // BackSpace
      if (0 < _cursor_pos) {
        _data = _data.substr(0, _cursor_pos-1) + _data.substr(_cursor_pos);
        --_cursor_pos;
      }
//      if (!_data.empty()) _data.resize(_data.size() - 1);
    }
    else
    if (code == input_t::input_left)
    { // move left
      if (--_cursor_pos < 0) _cursor_pos = 0;
    }
    else
    if (code == input_t::input_right)
    { // move right
      if (++_cursor_pos > _data.size()) _cursor_pos = _data.size();
    }
    else
    {
      if (_cursor_pos < _data.size())
      {
        _data.insert(_cursor_pos, 1, code);
        ++_cursor_pos;
      }
      else
      {
        _data.append(1, code);
        _cursor_pos = _data.size();
      }
    }
    setRedraw();
    return true;
  }

  bool LGUI_TextBox::touch_impl(touch_t touchstate, LovyanUOM* uom)
  {
    if (touchstate & touch_t::mask_moving)
    {
      std::int32_t x, y;
      uom->getTouchXY(&x, &y);
      x -= _visible_client_rect.left;
      y -= _visible_client_rect.top;

      auto gui = static_cast<LovyanGUI*>(getRootControl());
      auto canvas = gui->getCanvas();
      canvas->setFont(getFont());
      _cursor_pos = canvas->textLength(_data.c_str(), x - 3);
      _redraw = static_cast<redraw_t>(_redraw | redraw_t::redraw_body);
    }

    return true;
  }
}

