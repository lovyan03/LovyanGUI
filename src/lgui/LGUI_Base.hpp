#ifndef LGUI_BASE_HPP_
#define LGUI_BASE_HPP_

#include <cstdint>
#include <vector>

#include "LGUI_InputManager.hpp"
#include "lgui_common.hpp"

namespace lgfx
{
  class IFont;
  class bgr888_t;
  class LGFX_Device;
  class LGFX_Sprite;
  class LovyanGFX;
}

typedef lgfx::LovyanGFX LovyanGFX;
typedef lgfx::LGFX_Device LGFX_Device;
typedef lgfx::LGFX_Sprite LGFX_Sprite;

namespace lgui
{
  enum state_t : std::uint8_t
  { state_invisible = 0
  , state_disappear = 1 << 0
  , state_appear    = 1 << 1
  , state_visible   = 1 << 2
  };

  enum redraw_t : std::uint8_t
  { redraw_none   = 0
  , redraw_left   = 1 << 0
  , redraw_top    = 1 << 1
  , redraw_right  = 1 << 2
  , redraw_bottom = 1 << 3
  , redraw_body   = 1 << 4
  , redraw_vscroll= 1 << 5
  , redraw_hscroll= 1 << 6
  , redraw_cursor = 1 << 7
  };

  class LovyanGUI;
  class LGUI_Container;

  class LGUI_Base
  {
  friend LovyanGUI;
  friend LGUI_Container;
  public:

    bool isVisible(void) const { return _state; }
    bool isHiding(void) const { return _state & state_t::state_disappear; }
    void show(void);
    void hide(void);
    void input(input_t code);
    void touch(touch_t touchstate, LovyanUOM* uom);
    void setup(void);
    bool loop(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect);
    state_t getState(void) const { return _state; }
    const Rectangle& getVisibleRect(void) const { return _visible_rect; }
    const Rectangle& getDestRect(void) const { return _relative_dest_rect; }
    const Rectangle& getHideRect(void) const { return _relative_hide_rect; }
    const Rectangle& getTargetRect(void) const { return isHiding() ? _relative_hide_rect : _relative_dest_rect; }
    const Rectangle& getClientRect(void) const { return _visible_client_rect; }
    void setDestRect(const Rectangle& rect) { _relative_dest_rect = rect; update_client_rect_impl(); }
    void setHideRect(const Rectangle& rect) { _relative_hide_rect = rect; update_client_rect_impl(); }
    void setDestRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) { _relative_dest_rect = { x, y, x+w-1, y+h-1 }; update_client_rect_impl(); }
    void setHideRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) { _relative_hide_rect = { x, y, x+w-1, y+h-1 }; update_client_rect_impl(); }
    void setFont(const lgfx::IFont* font) { _font = font; }
    void setFocus(void) { setFocus_impl(); };

    void showMagnifire(void) { showMagnifire_impl(); }
    void showKeyboard(void) { showKeyboard_impl(this); }

    const lgfx::IFont* getFont(void);
    LGUI_Base* getRootControl(void);
    LGUI_Base* getFocusControl(void) { return getFocusControl_impl(); }
    LGUI_Base* getControlAtPos(std::int32_t x, std::int32_t y) { return getControlAtPos_impl(x, y); }

    LGUI_Base(void) {}
    virtual ~LGUI_Base(void) {}

/**
 * @brief 次回の描画時に再描画を行うよう指示する
 */
    void setRedraw(redraw_t flg = static_cast<redraw_t>(~0u));

    bool hasRedraw(redraw_t flg);

    // callback event handlers.

    /// 表示状態が変更された際に起きるイベント
    void (*onChangeState)(LGUI_Base* sender, state_t state) = nullptr;

    /// 入力操作を受け取った時に起きるイベント
    void (*onInput )(LGUI_Base* sender, input_t input) = nullptr;

    /// タッチ操作を受け取った時に起きるイベント
    void (*onTouch)(LGUI_Base* sender, touch_t touchstate, LovyanUOM* uom) = nullptr;

    /// 外枠の太さ
    std::uint8_t frameWidth = 1;

  protected:
    redraw_t _redraw;
    state_t _state = state_t::state_appear;

    LGUI_Container* _parent = nullptr;
    const lgfx::IFont* _font = nullptr;

    Rectangle _visible_rect;
    Rectangle _relative_move_rect;
    Rectangle _relative_dest_rect;
    Rectangle _relative_hide_rect;

    Rectangle _visible_client_rect;

    virtual bool isContainer(void) { return false; }

    void fill_rect(LovyanGUI* gui, Rectangle& rect, bool force, std::uint32_t color);

    virtual void setup_impl(void) {}
    virtual bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) { return true; }

    virtual bool show_impl(void) { return true; }
    virtual bool input_impl(input_t&) { return true; }
    virtual bool touch_impl(touch_t touchstate, LovyanUOM* uom);
    virtual void update_client_rect_impl(void);

    virtual LGUI_Base* getFocusControl_impl(void) { return this; }
    virtual LGUI_Base* getControlAtPos_impl(std::int32_t x, std::int32_t y) { return (isVisible() && _visible_rect.contains(x, y)) ? this : nullptr; }
    virtual void showMagnifire_impl(void);
    virtual void showKeyboard_impl(LGUI_Base* target);
    virtual void setFocus_impl(void);

//    void get_invalidate_rects(std::vector<Rectangle>& rects);
  };
}

#endif
