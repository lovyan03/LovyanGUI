#ifndef LOVYANGUI_HPP_
#define LOVYANGUI_HPP_

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "lgui/LGUI_InputManager.hpp"
#include "lgui/LGUI_Label.hpp"
#include "lgui/LGUI_TextBox.hpp"
#include "lgui/LGUI_OnScreenKeyboard.hpp"
#include "lgui/LGUI_TreeView.hpp"
#include "lgui/LGUI_Container.hpp"

namespace lgui
{
  class LovyanGUI : public LGUI_Container
  {
  friend LGUI_Base;
  friend LGUI_Container;
  public:
    struct colors
    {
      /// text color 文字の色
      std::uint32_t text   = 0xFFFFFFU;

      /// item border color アイテムの境界線の色
      std::uint32_t border = 0x000000U;

      /// back color (fixed area) 固定領域背景色
      std::uint32_t fixed  = 0x666666U;

      /// back color (editable area) 編集エリアの背景色
      std::uint32_t back   = 0x222222U;

      /// not display color. 描画しないカラーコード (描画時にマスク色として使用される)
      std::uint32_t mask   = 0x550055U;

      union
      {
        /// frame outline colors[inactive, active]
        std::uint32_t frame[2] = { 0x777755U, 0x9999FFU };
        struct
        {
          /// inactive frame color 非アクティブフレーム色
          std::uint32_t inactiveFrame;

          /// active frame color アクティブフレーム色
          std::uint32_t activeFrame;
        };
      };

      union
      {
        std::uint32_t cursor[2] = { 0x555555U, 0x7777FFU };
        struct
        {
          std::uint32_t inactiveCursor;
          std::uint32_t activeCursor;
        };
      };

      union
      {
        std::uint32_t subCursor[2] = { 0x4F4F55U, 0x4F4FAAU };
        struct
        {
          std::uint32_t inactiveSubCursor;
          std::uint32_t activeSubCursor;
        };
      };
    };

    colors color;

    /// 枠の移動速度 (0が最速)
    std::uint_fast8_t smoothMove = 32;

    std::uint_fast8_t colorDepth = 16;

    //void addControl(LGUI_Base*);
    //void removeControl(LGUI_Base*);
    //void setFocusControl(LGUI_Base*);

    void input(input_t);

    bool loop(void);
    void setup(LGFX_Device*);
    void setup(LGFX_Device*, LGFX_Sprite*);
    void setup(LGFX_Device*, LGUI_InputManager*);
    void setup(LGFX_Device*, LGUI_InputManager*, LGFX_Sprite*);

    LGFX_Device* getGFX(void) { return _gfx; }
    LGFX_Sprite* getCanvas(void) { return _canvas; }
    LGFX_Sprite* getCanvas(std::int32_t width, std::int32_t height);
    LGUI_InputManager* getInputManager(void) const { return _im; }
    LGUI_OnScreenKeyboard* getKeyboard(void) { return &_keyboard; }

    std::uint32_t getMsec(void) const { return _im->getMsec(); }

    LovyanGUI(void);
    virtual ~LovyanGUI(void);


/**
 * @brief  キャンバスの内容を描画する
 * 
 * @param rect 描画対象の座標
 */
    void pushCanvas(const Rectangle& rect);


/**
 * @brief 矩形領域が他のコントロールに完全に隠れているか判定する
 * 
 * @param rect 描画予定の領域
 * @return true 完全に隠れている
 * @return false 隠れていない
 */
    bool checkFullMasked(const Rectangle& rect);


/**
 * @brief 次回の描画時に再描画を行うよう指示する
 * 
 * @param rect 再描画が必要な領域
 */
  void invalidate(const Rectangle& rect) { rect.addRectangle(_invalidated_rects); }

/**
 * @brief 矩形領域の再描画が必要か判定する
 * 
 * @param rect 判定対象の領域
 * @return true 再描画が必要
 * @return false 再描画は不要
 */
  bool hasInvalidate(const Rectangle& rect) const
  {
    return rect.intersectsWithArray(_invalidated_rects);
  }

  const std::vector<Rectangle>& getInvalidatedRects(void) const { return _invalidated_rects; }

  void addMaskRect(const Rectangle& rect) { rect.addRectangle(_mask_rects); rect.removeRectangle(_invalidated_rects); }

  protected:
    LGFX_Device* _gfx = nullptr;
    LGFX_Sprite* _canvas = nullptr;
    LGUI_InputManager* _im = nullptr;

    std::vector<Rectangle> _mask_rects;
    std::vector<Rectangle> _invalidated_rects;
    LGUI_Base* _touchControl = nullptr;

  private:
    std::vector<LGUI_Base*> _ctl_stayontop;
    LGUI_Base _magnifire;
    LGUI_OnScreenKeyboard _keyboard;
    void showMagnifire_impl(void) override;
    void showKeyboard_impl(LGUI_Base* target) override;

    LGUI_Base* getControlAtPos_impl(std::int32_t x, std::int32_t y) override;
  };
}

typedef lgui::LovyanGUI LGUI;

#endif
