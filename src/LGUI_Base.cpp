#include <LGUI_Base.h>
#include <LovyanGFX.hpp>

namespace lgui
{
  static int32_t mix(int32_t src, int32_t dst, int32_t src_w, int32_t dst_w)
  {
    src += ((dst - src) * dst_w / (src_w + dst_w));
    if (src < dst) return src + 1;
    if (src > dst) return src - 1;
    return src;
  }

  bool Rectangle::moveTo(const Rectangle& dst, int src_weight, int dst_weight)
  {
    if (src_weight < 1) src_weight = 1;
    if (dst_weight < 1) dst_weight = 1;
    bool res = false;
    if (left   != dst.left  ) { res = true; left   = mix( left  , dst.left  , src_weight, dst_weight); }
    if (top    != dst.top   ) { res = true; top    = mix( top   , dst.top   , src_weight, dst_weight); }
    if (right  != dst.right ) { res = true; right  = mix( right , dst.right , src_weight, dst_weight); }
    if (bottom != dst.bottom) { res = true; bottom = mix( bottom, dst.bottom, src_weight, dst_weight); }
    return res;
  }

  void LGUI_Base::setup(LovyanGFX* lcd, int x, int y, int w, int h)
  {
    _loop_counter = 0;
    _lcd = lcd;
    _window_dest.left  = x;
    _window_dest.top   = y;
    _window_dest.width(w);
    _window_dest.height(h);

    _window_hide.left = _window_hide.right  = (_window_dest.left + _window_dest.right ) >> 1;
    _window_hide.top  = _window_hide.bottom = (_window_dest.top  + _window_dest.bottom) >> 1;

    _window_rect = _window_hide;

    setup_impl();
  }

  bool LGUI_Base::loop(const std::vector<const Rectangle*>* rects)
  {
    ++_loop_counter;
    bool moving = false;
    if (_visible)
    {
      if (_hiding)
      {
        auto rect = _window_rect;
        moving = _window_rect.moveTo(_window_hide, 16, _moveSpeed);
        if (1 == _window_rect.height() || 1 == _window_rect.width())
        {
          moving = false;
        }
        if (_lcd)
        {
          _lcd->setColor(0);
          if (rect.top < _window_rect.top) {
            _lcd->fillRect(rect.left, rect.top, rect.width(), std::min(rect.height(), _window_rect.top - rect.top));
            rect.top = std::min(rect.bottom, _window_rect.top);
          }
          if (rect.bottom > _window_rect.bottom) {
            _lcd->fillRect(rect.left, rect.bottom, rect.width(), std::max(-rect.height(), _window_rect.bottom - rect.bottom));
            rect.bottom = std::max(rect.top, _window_rect.bottom);
          }
          if (0 < rect.height()) {
            if (rect.left < _window_rect.left) {
              _lcd->fillRect(rect.left, rect.top, _window_rect.left - rect.left, rect.height());
            }
            if (rect.right > _window_rect.right) {
              _lcd->fillRect(_window_rect.right + 1, rect.top, rect.right - _window_rect.right, rect.height());
            }
          }
        }
        if (!moving) {
          _visible = false;
          _hiding = false;
          onHide();
        }
      }
      else
      {
        moving = _window_rect.moveTo(_window_dest, 16, _moveSpeed);
      }
    }
    return loop_impl(moving, rects);
  }

  void LGUI_Base::show(void)
  {
    if (_visible) return;
    _visible = true;
    _window_rect = _window_hide;
    _cursor_rect = _window_hide;
    onShow();
  }

  void LGUI_Base::hide(void)
  {
    if (!_visible || _hiding) return;
    _hiding = true;
    onHiding();
  }
}

