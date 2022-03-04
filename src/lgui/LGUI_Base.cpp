#include "LGUI_Base.hpp"
#include "../LovyanGUI.hpp"

namespace lgui
{
  void LGUI_Base::setup(void)
  {
    setup_impl();
  }
/*
  void LGUI_Base::setup(int x, int y, int w, int h)
  {
    _relative_dest_rect.left  = x;
    _relative_dest_rect.top   = y;
    _relative_dest_rect.setWidth(w);
    _relative_dest_rect.setHeight(h);

    _relative_hide_rect.left = _relative_hide_rect.right  = (_relative_dest_rect.left + _relative_dest_rect.right ) >> 1;
    _relative_hide_rect.top  = _relative_hide_rect.bottom = (_relative_dest_rect.top  + _relative_dest_rect.bottom) >> 1;

    _frame_rect_move = _relative_hide_rect;

    setup_impl();
  }
///*/
  void LGUI_Base::fill_rect(LovyanGUI* gui, Rectangle& rect, bool force, std::uint32_t color)
  {
    if (!force && !gui->hasInvalidate(rect)) return;

    auto canvas = gui->getCanvas(rect.width(), rect.height());
    canvas->clear(color);
//canvas->clear(esp_random());
    gui->pushCanvas(rect);
  }


  bool LGUI_Base::loop(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect)
  {
    if (_state == state_t::state_invisible) return false;

    if (_state == state_t::state_appear)
    {
      _state = state_t::state_visible;
      _visible_rect = { 1, 1, 0, 0 };
      _relative_move_rect = _relative_hide_rect;
      if (show_impl() && onChangeState) onChangeState(this, _state);
    }

    auto prev_rect = _visible_rect;
    {
      auto &destrect = getTargetRect();
      if (destrect != _relative_move_rect)
      {
        _relative_move_rect.moveTo(destrect, gui->smoothMove, 16);
      }
    }
    _visible_rect = _relative_move_rect;
    if (_parent)
    {
      std::int32_t offset_x = _parent->_visible_client_rect.left - _parent->_hscroll_move;
      std::int32_t offset_y = _parent->_visible_client_rect.top - _parent->_vscroll_move;
      _visible_rect.offset(offset_x, offset_y);
    }

    if (!prev_rect.intersectsWith(cliprect)
     && !_visible_rect.intersectsWith(cliprect)) {
      return false;
     }

    if (prev_rect == _visible_rect)
    {
      if (isHiding())
      { // フレーム縮小（閉じる）処理
        gui->invalidate(prev_rect);
        _redraw = redraw_t::redraw_none;
        _state = state_t::state_invisible;

        if (onChangeState) onChangeState(this, _state);

        return false;
      }
    }
    else
    if (!prev_rect.intersectsWith(_visible_rect))
    { // 前回表示分と今回表示分が全く重ならない場合
      _redraw = static_cast<redraw_t>( ~0 );
      gui->invalidate(_visible_rect);
      gui->invalidate(prev_rect);
//gfx->drawRect(_visible_rect.left, _visible_rect.top, _visible_rect.width(), _visible_rect.height(), 0xFFFF);
//gfx->drawRect(prev_rect.left, prev_rect.top, prev_rect.width(), prev_rect.height(), 0xFFFF);
    }
    else 
    { // 前回表示分と重なっている場合
      auto now_rect = _visible_rect;
      if (prev_rect.top != now_rect.top)
      {
        _redraw = static_cast<redraw_t>(_redraw | redraw_body | redraw_top | redraw_vscroll);
        if (prev_rect.top > now_rect.top) {
          _redraw = static_cast<redraw_t>(_redraw | redraw_left | redraw_right);
          auto r = now_rect.intersect(cliprect);
          r.bottom = _visible_client_rect.top - 1;
          now_rect.top = prev_rect.top;
          gui->invalidate(r);
        }
        else
        {
          auto r = prev_rect.intersect(cliprect);
          r.bottom = now_rect.top - 1;
          gui->invalidate(r);
        }
      }

      if (prev_rect.bottom != now_rect.bottom)
      {
        _redraw = static_cast<redraw_t>(_redraw | redraw_bottom | redraw_hscroll | redraw_vscroll);
        if (prev_rect.bottom < now_rect.bottom) {
          _redraw = static_cast<redraw_t>(_redraw | redraw_left | redraw_right);
          auto r = now_rect.intersect(cliprect);
          r.top = _visible_client_rect.bottom + 1;
          now_rect.bottom = prev_rect.bottom;
          gui->invalidate(r);
        }
        else
        {
          auto r = prev_rect.intersect(cliprect);
          r.top = now_rect.bottom + 1;
          gui->invalidate(r);
        }
      }

      if (prev_rect.left != now_rect.left)
      {
        _redraw = static_cast<redraw_t>(_redraw | redraw_body | redraw_left | redraw_hscroll);
        if (prev_rect.left > now_rect.left) {
          _redraw = static_cast<redraw_t>(_redraw | redraw_top | redraw_bottom);
          Rectangle r = { now_rect.left
                        , _visible_client_rect.top
                        , _visible_client_rect.left - 1
                        , _visible_client_rect.bottom };
          gui->invalidate(r.intersect(cliprect));
        }
        else
        {
          Rectangle r = {prev_rect.left, now_rect.top, now_rect.left-1, now_rect.bottom };
          gui->invalidate(r.intersect(cliprect));
        }
      }

      if (prev_rect.right != now_rect.right)
      {
        _redraw = static_cast<redraw_t>(_redraw | redraw_right | redraw_hscroll | redraw_vscroll);
        if (prev_rect.right < now_rect.right) {
          _redraw = static_cast<redraw_t>(_redraw | redraw_top | redraw_bottom);
          Rectangle r = { _visible_client_rect.right + 1
                        , _visible_client_rect.top
                        , now_rect.right
                        , _visible_client_rect.bottom };
          gui->invalidate(r.intersect(cliprect));
        }
        else
        {
          Rectangle r = { now_rect.right + 1, now_rect.top, prev_rect.right, now_rect.bottom };
          gui->invalidate(r.intersect(cliprect));
        }
      }
    }

    auto gfx = gui->getGFX();
    {
      auto r = _visible_rect.intersect(cliprect);
      gfx->setClipRect(r.left, r.top, r.width(), r.height());
    }

    if (frameWidth > 0 && (_redraw || gui->hasInvalidate(_visible_rect)))
    { // 枠線の描画
      auto rect = _visible_rect;
      std::uint32_t color = gui->color.frame[has_focus];

      rect.bottom = rect.top + frameWidth - 1;
      fill_rect(gui, rect, _redraw & redraw_top, color);

      rect.bottom = _visible_rect.bottom;
      rect.top = rect.bottom - frameWidth + 1;
      fill_rect(gui, rect, _redraw & redraw_bottom, color);

      rect.top    = _visible_rect.top    + frameWidth;
      rect.bottom = _visible_rect.bottom - frameWidth;

      if (0 < rect.height())
      {
        rect.right = rect.left + frameWidth - 1;
        fill_rect(gui, rect, _redraw & redraw_left, color);
        rect.right = _visible_rect.right;
        rect.left = rect.right - frameWidth + 1;
        fill_rect(gui, rect, _redraw & redraw_right, color);
      }
    }

    update_client_rect_impl();

    auto rect = cliprect.intersect(_visible_client_rect);

    bool res = true;
    if (!rect.empty())
    {
      auto canvas = gui->getCanvas();
      canvas->setTextColor(gui->color.text);
      canvas->setFont(getFont());

      res = loop_impl(gui, has_focus, rect);
    }

    _redraw = redraw_t::redraw_none;

    gfx->clearClipRect();
    return res;
  }

  void LGUI_Base::show(void)
  {
    if (_state == state_t::state_visible) return;
    if (_state == state_t::state_disappear)
    {
      _state = state_t::state_visible;
    }
    else
    if (_state == state_t::state_invisible)
    {
      _state = state_t::state_appear;
    }
  }

  void LGUI_Base::hide(void)
  {
    if (_state == state_t::state_invisible || _state == state_t::state_disappear) return;

    _state = state_t::state_disappear;

    if (onChangeState) onChangeState(this, _state);
  }

  void LGUI_Base::input(input_t code)
  {
    if (isHiding()) return;
    if (input_impl(code) && onInput)
    {
      onInput(this, code);
    }
  }

  void LGUI_Base::touch(touch_t touchstate, LovyanUOM* uom)
  {
    if (isHiding()) return;
    if (touch_impl(touchstate, uom) && onTouch)
    {
      onTouch(this, touchstate, uom);
    }
  }

  bool LGUI_Base::touch_impl(touch_t touchstate, LovyanUOM* uom)
  {
    if (!_parent) return true;
    return _parent->touch_impl(touchstate, uom);
  }

  void LGUI_Base::update_client_rect_impl(void)
  {
    _visible_client_rect = _visible_rect;
    if (0 < frameWidth)
    {
      _visible_client_rect.inflate( - frameWidth);
    }
  }

  LGUI_Base* LGUI_Base::getRootControl(void)
  {
    if (_parent == nullptr) return this;
    return _parent->getRootControl();
  }

  void LGUI_Base::setFocus_impl(void)
  {
    if (_parent) _parent->setFocus(this);
  }

  void LGUI_Base::showMagnifire_impl(void)
  {
    if (_parent) _parent->showMagnifire();
  }

  void LGUI_Base::showKeyboard_impl(LGUI_Base* target)
  {
    if (_parent) _parent->showKeyboard_impl(target);
  }

  void LGUI_Base::setRedraw(redraw_t flg)
  {
    _redraw = static_cast<redraw_t>(_redraw | flg);
  }

  bool LGUI_Base::hasRedraw(redraw_t flg)
  {
    return _redraw & flg;
  }

  const lgfx::IFont* LGUI_Base::getFont(void)
  {
    if (_font) return _font;
    if (_parent != nullptr) return _parent->getFont(); 
    return &fonts::Font0; 
  }
}
