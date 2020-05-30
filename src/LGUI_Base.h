#ifndef _LGUI_BASE_H_
#define _LGUI_BASE_H_

#include <cstdint>
#include <vector>

namespace lgfx
{
  struct IFont;
}

class LovyanGFX;

namespace lgui
{
  enum input_t : uint8_t
  { input_none   = 0
  , input_left   = 0x25
  , input_up     = 0x26
  , input_right  = 0x27
  , input_down   = 0x28
  , input_change = 0x0A
  , input_enter  = 0x0D
  , input_tab    = 0x09
  };

  struct Rectangle
  {
    std::int32_t left   = 0;
    std::int32_t top    = 0;
    std::int32_t right  = 0;
    std::int32_t bottom = 0;

    Rectangle() = default;
    Rectangle(std::int32_t left, std::int32_t top, std::int32_t right, std::int32_t bottom)
     : left   ( left   )
     , top    ( top    )
     , right  ( right  )
     , bottom ( bottom )
     {}
    std::int32_t width(void) const { return right - left + 1; }
    std::int32_t height(void) const { return bottom - top + 1; }
    void width(std::int32_t w) { right = left + w - 1; }
    void height(std::int32_t h) { bottom = top + h - 1; }

    bool equal(const Rectangle& rhs) const
    {
      return left   == rhs.left
          && top    == rhs.top
          && right  == rhs.right
          && bottom == rhs.bottom;
    }

    Rectangle intersect(const Rectangle& rhs) const
    {
      std::int32_t t = rhs.top    > top    ? rhs.top   : top;
      std::int32_t l = rhs.left   > left   ? rhs.left  : left;
      std::int32_t r = rhs.right  < right  ? rhs.right : right;
      std::int32_t b = rhs.bottom < bottom ? rhs.bottom : bottom;
      return Rectangle(l, t, r, b);
    }

    bool intersectsWith(const Rectangle& r) const {
       return (r.left < right) && (left < r.right) && (r.top < bottom) && (top < r.bottom);
    }

    bool moveTo(const Rectangle& dst, int src_weight = 1, int dst_weight = 1);

    void inflate(std::int32_t inc) { left -= inc; top -= inc; right += inc; bottom += inc; };

    bool empty(void) const { return left > right || top > bottom; }
  };

  class LGUI_Base
  {
  public:
    bool isVisible(void) const { return _visible; }
    void setup(LovyanGFX*, int x, int y, int w, int h);
    void show(void);
    void hide(void);
    bool loop(const std::vector<const Rectangle*>* rects = nullptr);
    void input(input_t code) { input_impl(code); }
    const Rectangle& getWindowRect(void) const { return _window_rect; }
    LGUI_Base(void) {}
    virtual ~LGUI_Base(void) {}

    virtual void onShow(void) {};
    virtual void onHiding(void) {};
    virtual void onHide(void) {};

  protected:
    LovyanGFX* _lcd;
    bool _drawall = false;
    bool _visible = true;
    bool _hiding = false;
    std::uint_fast8_t _moveSpeed = 3;
    std::uint32_t _color_depth = 4;
    std::uint32_t _color_text  = 15;
    std::uint32_t _color_back  =  1;
    std::uint32_t _color_frame =  2;
    std::uint32_t _color_focus =  3;
    std::uint32_t _color_mask  =  0;
    std::uint32_t _loop_counter;

    Rectangle _window_rect;
    Rectangle _window_dest;
    Rectangle _window_hide;
    Rectangle _cursor_rect;
    Rectangle _cursor_dest;

    virtual void setup_impl(void) = 0;
    virtual bool loop_impl(bool drawall, const std::vector<const Rectangle*>* rects) = 0;
    virtual void input_impl(input_t) = 0;
  };
}

#endif
