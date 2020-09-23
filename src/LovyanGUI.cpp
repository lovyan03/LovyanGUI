#include "LovyanGUI.hpp"
#include <LovyanGFX.hpp>

#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (ESP_PLATFORM)
#include <soc/sens_reg.h>
#include <esp_log.h>
#endif

#include <vector>
#include <algorithm>

namespace lgui
{
  static LGFX_Sprite* get_default_canvas(void)
  {
    static LGFX_Sprite canvas;
    return &canvas;
  }

  static LGUI_InputManager* create_default_im(LGFX_Device* gfx)
  {
    LGUI_InputManager *im = nullptr;
    auto board = gfx->getBoard();

    switch (board) {

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32) || (ESP_PLATFORM)

    case lgfx::board_M5StickC:
    case lgfx::board_M5StickCPlus:
      im = new LGUI_InputManager_M5StickC();
      break;

    case lgfx::board_ODROID_GO:
      im = new LGUI_InputManager_ODROIDGO();
      break;

    case lgfx::board_TTGO_TWatch:
      im = new LGUI_InputManager_TWatch();
      break;

    case lgfx::board_M5StackCore2:
      im = new LGUI_InputManager_M5StackCore2();
      break;

#endif

    default:
      im = new LGUI_InputManager();
      break;
    }

    im->init();

    switch (board) {
#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32) || (ESP_PLATFORM)
    case lgfx::board_M5Stack:
      im->setGPIO(raw_key_t::btn_a, 39, true);
      im->setGPIO(raw_key_t::btn_b, 38, true);
      im->setGPIO(raw_key_t::btn_c, 37, true);
      break;

    case lgfx::board_M5StickC:
    case lgfx::board_M5StickCPlus:

      im->setGPIO(raw_key_t::btn_a, 39, true);
     // M5StickC の場合、 btn_b には電源ボタンを割り当てる
      im->setGPIO(raw_key_t::btn_c, 37, true);
      break;

    case lgfx::board_TTGO_TWatch:
      im->setGPIO(raw_key_t::btn_c, 36, true);
      lgfx::i2c::init(I2C_NUM_0, 21, 22, 400000); // for AXP202
      break;

    case lgfx::board_ODROID_GO:
      // enable ADC
      SET_PERI_REG_MASK(SENS_SAR_READ_CTRL_REG, SENS_SAR1_DATA_INV);

      SET_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_START_FORCE_M); //SAR ADC1 controller (in RTC) is started by SW
      SET_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_SAR1_EN_PAD_FORCE_M); //SAR ADC1 pad enable bitmap is controlled by SW

      CLEAR_PERI_REG_MASK(SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_SAR_M); //force XPD_SAR=0, use XPD_FSM
      SET_PERI_REG_BITS(  SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_AMP, 0x2, SENS_FORCE_XPD_AMP_S); //force XPD_AMP=0

      CLEAR_PERI_REG_MASK(SENS_SAR_MEAS_CTRL_REG, 0xfff << SENS_AMP_RST_FB_FSM_S);  //clear FSM
      SET_PERI_REG_BITS(SENS_SAR_MEAS_WAIT1_REG, SENS_SAR_AMP_WAIT1, 0x1, SENS_SAR_AMP_WAIT1_S);
      SET_PERI_REG_BITS(SENS_SAR_MEAS_WAIT1_REG, SENS_SAR_AMP_WAIT2, 0x1, SENS_SAR_AMP_WAIT2_S);
      SET_PERI_REG_BITS(SENS_SAR_MEAS_WAIT2_REG, SENS_SAR_AMP_WAIT3, 0x1, SENS_SAR_AMP_WAIT3_S);

      im->setGPIO(raw_key_t::trigg_a, 32, true);
      im->setGPIO(raw_key_t::trigg_b, 33, true);
      im->setGPIO(raw_key_t::btn_a  , 13, true); // BUTTON_MENU_PIN
      im->setGPIO(raw_key_t::btn_b  ,  0, true); // BUTTON_VOLUME_PIN
      im->setGPIO(raw_key_t::btn_d  , 27, true); // BUTTON_SELECT_PIN
      im->setGPIO(raw_key_t::btn_e  , 39, true); // BUTTON_START_PIN

      break;

#endif
#if defined (__SAMD51__)
    case lgfx::board_WioTerminal:
      im->setGPIO(raw_key_t::stick_l, 0x0300| 8, true);
      im->setGPIO(raw_key_t::stick_u, 0x0300|12, true);
      im->setGPIO(raw_key_t::stick_r, 0x0300|20, true);
      im->setGPIO(raw_key_t::stick_d, 0x0300| 9, true);
      im->setGPIO(raw_key_t::trigg_a, 0x0300|10, true);
      im->setGPIO(raw_key_t::btn_a  , 0x0200|28, true);
      im->setGPIO(raw_key_t::btn_b  , 0x0200|27, true);
      im->setGPIO(raw_key_t::btn_c  , 0x0200|26, true);
      break;
#endif
    default:
      break;
    }


    static constexpr std::uint32_t stick_l[] = { raw_bit_t::bit_stick_l , ~0U };
    static constexpr std::uint32_t stick_r[] = { raw_bit_t::bit_stick_r , ~0U };
    static constexpr std::uint32_t stick_u[] = { raw_bit_t::bit_stick_u , ~0U };
    static constexpr std::uint32_t stick_d[] = { raw_bit_t::bit_stick_d , ~0U };

    static constexpr std::uint32_t trigg_a[] = { raw_bit_t::bit_trigg_a , ~0U };

    static constexpr std::uint32_t btn_a[] = { 0, 0, raw_bit_t::bit_btn_a, 0, 0, ~0U };
    static constexpr std::uint32_t btn_b[] = {    0, raw_bit_t::bit_btn_b, 0,    ~0U };
    static constexpr std::uint32_t btn_c[] = { 0, 0, raw_bit_t::bit_btn_c, 0, 0, ~0U };

    static constexpr std::uint32_t btn_a_hold[] = { raw_bit_t::bit_btn_a, raw_bit_t::bit_btn_a, 0, ~0U };
    static constexpr std::uint32_t btn_b_hold[] = { raw_bit_t::bit_btn_b, raw_bit_t::bit_btn_b, 0, ~0U };
    static constexpr std::uint32_t btn_c_hold[] = { raw_bit_t::bit_btn_c, raw_bit_t::bit_btn_c, 0, ~0U };

    static constexpr std::uint32_t btn_a_dbl[] = { 0, raw_bit_t::bit_btn_a, 0, raw_bit_t::bit_btn_a, 0, ~0U };
//  static constexpr std::uint32_t btn_b_dbl[] = { 0, raw_bit_t::bit_btn_b, 0, raw_bit_t::bit_btn_b, 0, ~0U };
    static constexpr std::uint32_t btn_c_dbl[] = { 0, raw_bit_t::bit_btn_c, 0, raw_bit_t::bit_btn_c, 0, ~0U };
    static constexpr std::uint32_t btn_c_dblhold[] = { raw_bit_t::bit_btn_c, raw_bit_t::bit_btn_c, 0, raw_bit_t::bit_btn_c, 0, ~0U };

    static constexpr std::uint32_t btn_ac0[] = {                    0, raw_bit_t::bit_btn_a|raw_bit_t::bit_btn_c, ~0U };
    static constexpr std::uint32_t btn_ac1[] = { raw_bit_t::bit_btn_a, raw_bit_t::bit_btn_a|raw_bit_t::bit_btn_c, ~0U };
    static constexpr std::uint32_t btn_ac2[] = { raw_bit_t::bit_btn_c, raw_bit_t::bit_btn_a|raw_bit_t::bit_btn_c, ~0U };

    im->addCommand(input_t::input_left  , stick_l);
    im->addCommand(input_t::input_right , stick_r);
    im->addCommand(input_t::input_up    , stick_u);
    im->addCommand(input_t::input_down  , stick_d);
    im->addCommand(input_t::input_enter , trigg_a);
    im->addCommand(input_t::input_fn1   , btn_a);
    im->addCommand(input_t::input_ok    , btn_b);
    im->addCommand(input_t::input_fn2   , btn_c);
    im->addCommand(input_t::input_back  , btn_a_hold);
    im->addCommand(input_t::input_ok    , btn_b_hold);
    im->addCommand(input_t::input_tab   , btn_c_hold);
    im->addCommand(input_t::input_back  , btn_c_dblhold);
    im->addCommand(input_t::input_fn1dbl, btn_a_dbl);
  //im->addCommand(input_t::input_ok    , btn_b_dbl);
    im->addCommand(input_t::input_enter , btn_c_dbl);
//  im->addCommand(input_t::input_enter , btn_c_dblhold);

    im->addCommand(input_t::input_specialAC, btn_ac0);
    im->addCommand(input_t::input_specialAC, btn_ac1);
    im->addCommand(input_t::input_specialAC, btn_ac2);

    return im;
  }


  LovyanGUI::LovyanGUI(void)
  {
    _magnifire._state = state_invisible;
    _magnifire._parent = this;

    _keyboard._state = state_invisible;
    _keyboard._parent = this;

    _ctl_stayontop.emplace_back(&_magnifire);
    _ctl_stayontop.emplace_back(&_keyboard);
    frameWidth = 0;
  }

  LovyanGUI::~LovyanGUI(void)
  {
  }

  void LovyanGUI::setup(LGFX_Device* gfx, LGUI_InputManager* im, LGFX_Sprite* canvas)
  {
    _gfx = gfx;
    _im = im;
    _im->setRotation(_gfx->getRotation());

    _canvas = canvas;
    setDestRect(0, 0, gfx->width(), gfx->height());
  }

  void LovyanGUI::setup(LGFX_Device* gfx, LGFX_Sprite* canvas)
  {
    setup(gfx, create_default_im(gfx), canvas);
  }

  void LovyanGUI::setup(LGFX_Device* gfx, LGUI_InputManager* im)
  {
    setup(gfx, im, get_default_canvas());
    _canvas->setColorDepth(colorDepth);
  }

  void LovyanGUI::setup(LGFX_Device* gfx)
  {
    setup(gfx, create_default_im(gfx), get_default_canvas());
    _canvas->setColorDepth(colorDepth);
  }

  bool LovyanGUI::loop(void)
  {
    auto key = _im->update(_gfx);
    auto fctl = _focusControl;
    if (nullptr == fctl || !fctl->isVisible())
    {
      fctl = getNextControl(fctl, true, true, true);
    }

    if (_focusControl != fctl && fctl != nullptr)
    {
      fctl->setFocus();
    }

    if (_keyboard.isVisible())
    {
      _keyboard.input(key);
    }
    else if (fctl)
    {
      fctl->input(key);
    }

    touch_t ts = _im->getTouchState();
    if (ts) {
      std::int32_t x, y;
      _im->getTouchBeginXY(&x, &y);
      if (ts == touch_t::touch_begin) {
        auto tctrl = getControlAtPos(x, y);
        if (tctrl != nullptr && tctrl->isHiding()) tctrl = nullptr;
        if (_touchControl != tctrl)
        {
//ESP_LOGI("LGUI","tctrl:%08x", reinterpret_cast<size_t>(tctrl));
          _touchControl = tctrl;
//          if (tctrl) tctrl->setFocus();
        }
      }
      else if (ts == touch_t::touch_end)
      {
        if (_touchControl != _focusControl && _touchControl != nullptr)
        {
          _touchControl->setFocus();
        }
      }
      else if ( ts == touch_t::hold_begin && y < _gfx->height() && x < _gfx->width())
      {
        showMagnifire();
      }

      if (_touchControl) {
        _touchControl->touch(ts, _im);
      }

      if (_magnifire.isVisible()) {
        std::int32_t x, y;
        std::int32_t w = 96;
        std::int32_t h = 64;
        _im->getTouchXY(&x, &y);
        y -= _visible_client_rect.top;
        x -= _visible_client_rect.left;
        x -= w >> 1;
        if (x < 0) x = 0;
        else if (x > _visible_rect.width() - w) x = _visible_rect.width() - w;
        auto half =  _visible_rect.height() >> 1;
        x += _hscroll_move;
        _magnifire.setHideRect(x+(w>>1), y + _vscroll_move, 1, 1);
        if (y < half) {
          y += half - (y >> 1);
        } else {
          y = (y >> 1) - h;
        }
        y += _vscroll_move;
        _magnifire.setDestRect( x, y, w, h);
      }
    } else {
      _touchControl = nullptr;
      if (_magnifire.isVisible()) {
        _magnifire.hide();
      }
    }

    _gfx->startWrite();

    for (auto &ctl : _ctl_stayontop)
    {
      if (ctl->isVisible() && ctl->loop(this, true, _visible_client_rect))
      {
        _mask_rects.push_back(ctl->getVisibleRect());
      }
    }

    Rectangle r(0, 0, _gfx->width() - 1, _gfx->height() - 1);
    bool res = LGUI_Container::loop(this, true, r);

    _mask_rects.clear();
    _invalidated_rects.clear();

    if (_magnifire.isVisible())
    {
      auto clientrect = _magnifire._visible_client_rect;
      std::int32_t w = clientrect.width()  >> 1;
      std::int32_t h = clientrect.height() >> 1;
      if (w > 0 && h > 0) {
        std::int32_t x,y;
        _im->getTouchXY(&x, &y);
        x -= (w>>1);
        y -= (h>>1);
        if (x < 0) { x = 0; }
        else if (x > _gfx->width() - w) x = _gfx->width() - w;
        if (y < 0) { y = 0; }
        else if (y > _gfx->height() - h) y = _gfx->height() - h;
        LGFX_Sprite sprite;
        sprite.setColorDepth(16);
        sprite.createSprite(w, h);
        sprite.setPivot(0,0);
        _gfx->readRect(x, y, w, h, static_cast<lgfx::swap565_t*>(sprite.getBuffer()));
        sprite.pushRotateZoom(_gfx, clientrect.left + 1, clientrect.top + 1, 0.0, 2.0, 2.0);
      }
    }

    _gfx->endWrite();
    _canvas->deleteSprite();
    return res;
  }

  void LovyanGUI::showMagnifire_impl(void)
  {
    if (_magnifire.isVisible() && !_magnifire.isHiding()) return;
    if (!_gfx->isReadable()) return;
    if (!_magnifire.isHiding())
    {
      std::int32_t x, y;
      _im->getTouchXY(&x, &y);

      x += _hscroll_move - _visible_client_rect.left;
      y += _vscroll_move - _visible_client_rect.top ;

      _magnifire.setHideRect(x, y, 1, 1);
      _magnifire._relative_dest_rect.left = x - 1;
      _magnifire._relative_dest_rect.right = x + 1;
    }
    _magnifire.show();
  }

  void LovyanGUI::showKeyboard_impl(LGUI_Base* target)
  {
    _keyboard.setTarget(target);
    target->show();
    target->setFocus();
    if (_keyboard.isVisible() && !_keyboard.isHiding()) return;
    if (!_keyboard.isHiding())
    {
      int osk_hight = std::max<int>(64, _gfx->height() / 3);
      _keyboard.setDestRect(0, _gfx->height() - osk_hight, _gfx->width(), osk_hight);
      _keyboard.setHideRect(0, _gfx->height() - 1, _gfx->width(), 1);
    }
    _keyboard.show();
  }

  LGFX_Sprite* LovyanGUI::getCanvas(std::int32_t width, std::int32_t height)
  {
    _gfx->waitDMA();
    //if (_canvas->width() != width || _canvas->height() != height)
    //{
    _canvas->createSprite(width, height);
    //}
    //_canvas->clearClipRect();
    return _canvas;
  }

  void LovyanGUI::pushCanvas(const Rectangle& rect)
  {
    // 手前に表示されているフレーム群のマスク処理
    _canvas->setColor(color.mask);
    _canvas->clearClipRect();
    bool masked = false;
    for (const auto& r : _mask_rects)
    {
      if (r.intersectsWith(rect))
      {
        if (r.contains(rect)) return;
        _canvas->fillRect( r.left - rect.left
                         , r.top  - rect.top
                         , r.width()
                         , r.height()
                         );
        masked = true;
      }
    }
    if (masked)
    {
      _canvas->pushSprite(_gfx, rect.left, rect.top, color.mask);
    }
    else
    {
      _canvas->pushSprite(_gfx, rect.left, rect.top);
    }
  }

  /// 引数の矩形が完全にマスクエリアに含まれているか確認する
  bool LovyanGUI::checkFullMasked(const Rectangle& rect)
  {
    for (const auto& r : _mask_rects)
    {
      if (r.contains(rect)) return true;
    }
    return false;
  }

  LGUI_Base* LovyanGUI::getControlAtPos_impl(std::int32_t x, std::int32_t y)
  {
    if (!isVisible()) return nullptr;
    if (!_visible_rect.contains(x, y)) return nullptr;

    for (auto& ctl : _ctl_stayontop)
    {
      if (!ctl->isVisible()) continue;
      auto res = ctl->getControlAtPos(x, y);
      if (res) return res;
    }
    return LGUI_Container::getControlAtPos_impl(x, y);
  }
}

