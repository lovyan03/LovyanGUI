#ifndef LGUI_INPUTMANAGER_HPP_
#define LGUI_INPUTMANAGER_HPP_

#include <cstdint>
#include <vector>

namespace lgfx
{
  class LGFX_Device;
}
namespace lgui
{
  class LovyanGUI;

  enum input_t : std::uint32_t
  { input_none   = 0
  , input_right  = 0x1C  // 右へ進む
  , input_left   = 0x1D  // 左へ進む
  , input_up     = 0x1E  // 上へ進む
  , input_down   = 0x1F  // 下へ進む

  , input_back      = 0x02 // 前の要素に戻る
  , input_forward   = 0x03 // 次の要素に進む
  , input_specialAB = 0x04
  , input_confirm   = 0x05
  , input_specialAC = 0x05
  , input_specialBC = 0x06
  , input_specialABC= 0x07
  , input_cancel    = 0x1b

  , input_enter  = 0x0D
  , input_ok     = 0x0F
  , input_tab    = 0x09
  , input_fn1    = 0x0B
  , input_fn2    = 0x0C
  , input_fn1dbl = 0x0E
  };

  enum raw_key_t : std::uint8_t
  { stick_l =  0  // コントローラ 左
  , stick_u =  1  // コントローラ 上
  , stick_r =  2  // コントローラ 右
  , stick_d =  3  // コントローラ 下
  , trigg_a =  4  // トリガー A
  , trigg_b =  5  // トリガー B
  , btn_a   =  6  // 戻る
  , btn_b   =  7  // 決定
  , btn_c   =  8  // 進む
  , btn_d   =  9  // SELECT
  , btn_e   = 10  // START
  , raw_key_max
  };

  enum raw_bit_t : std::uint32_t
  { bit_stick_l = 1U << stick_l
  , bit_stick_u = 1U << stick_u
  , bit_stick_r = 1U << stick_r
  , bit_stick_d = 1U << stick_d
  , bit_trigg_a = 1U << trigg_a
  , bit_trigg_b = 1U << trigg_b
  , bit_btn_a   = 1U << btn_a
  , bit_btn_b   = 1U << btn_b
  , bit_btn_c   = 1U << btn_c
  , bit_btn_d   = 1U << btn_d
  , bit_btn_e   = 1U << btn_e
  };


  // bit0 : touching on it now.
  // bit1 : change touch/release.
  // bit2 : touching it no move.
  // bit3 : holding it no move.
  enum touch_t : std::uint32_t
  { none         = 0b0000
  , touch        = 0b0001
  , touch_end    = 0b0010
  , touch_begin  = 0b0011
  , hold         = 0b0101
  , hold_end     = 0b0110
  , hold_begin   = 0b0111
  , flick        = 0b1001
  , flick_end    = 0b1010
  , flick_begin  = 0b1011

  , mask_touch  = 0b0001
  , mask_change = 0b0010
  , mask_holding= 0b0100
  , mask_moving = 0b1000
  };

  class LovyanUOM // UserOperationManager
  {
  public:
    LovyanUOM(void) {}
    virtual ~LovyanUOM(void) {}

    void addCommand(std::uint32_t value, const std::uint32_t* inputs);

    void clearCommand(void);

    void init(void);
    input_t update(lgfx::LGFX_Device* gfx);

    void setGPIO(raw_key_t name, std::int32_t ionum, bool invert = false);

    // 方向入力デバイスの回転設定
    void setRotation(std::uint8_t rotation) { _rotation = rotation & 3; }
    std::uint8_t getRotation(void) const { return _rotation; }

    // 方向キー押下状態の取得  0:無入力 / 1=右 / -1=左
    std::int_fast8_t getStickX(void) const;

    // 方向キー押下状態の取得  0:無入力 / 1=下 / -1=上
    std::int_fast8_t getStickY(void) const;

    // デバウンス済み生データの取得
    std::uint32_t getRaw(void) const { return _raw_after_db; }

    std::uint32_t getMsec(void) const { return _msec; }

    touch_t getTouchState(void) { return _touch_state; }
    void getTouchXY(std::int32_t* x, std::int32_t* y) { *x = _touch_x; *y = _touch_y; }
    void getTouchPrevXY(std::int32_t* x, std::int32_t* y) { *x = _touch_prev_x; *y = _touch_prev_y; }
    void getTouchBeginXY(std::int32_t* x, std::int32_t* y) { *x = _touch_begin_x; *y = _touch_begin_y; }
    void getTouchDeltaXY(std::int32_t* x, std::int32_t* y) { *x = _touch_x - _touch_prev_x; *y = _touch_y - _touch_prev_y; }

    void (*onGetRaw)(LovyanUOM* sender, std::uint32_t& raw) = nullptr;

    void (*onUpdate)(LovyanUOM* sender, input_t&) = nullptr;

  protected:
    static constexpr std::int32_t TOUCH_FLICK_DISTANCE = 8;
    static constexpr std::int32_t TOUCH_HOLD_MSEC = 500;

    struct command_holder_t
    {
      std::uint32_t value;
      const std::uint32_t* inputs;
      command_holder_t(std::uint32_t value, const std::uint32_t* inputs) : value(value), inputs(inputs) {};
    };

    std::vector<command_holder_t> commands;

    static constexpr std::size_t COMMAND_STACK_LENGTH = 8;
    std::uint32_t _raw_stack[COMMAND_STACK_LENGTH];
    std::uint32_t _multi_push;
    std::uint32_t _raw_before_db;
    std::uint32_t _raw_after_db;
    std::uint32_t _raw_after_db_prev;
    std::uint32_t _raw_invert;
    std::vector<std::int32_t> _gpio;

    std::uint32_t _msec;       // "update()" this time millis
    std::uint32_t _msec_last;  // "update()" last time millis
    std::uint32_t _msec_db_changed;
    std::uint32_t _msec_raw_changed;
    std::uint32_t _msec_last_changed;
    input_t _input_last;
    std::uint8_t _rotation = 1;

    static constexpr std::uint32_t _msec_db = 10;            //debounce time
    static constexpr std::uint32_t _msec_long_hold = 250;    //longhold threshold time

    std::uint32_t _msec_repeated;

    touch_t _touch_state;
    std::int32_t _touch_x;
    std::int32_t _touch_y;
    std::int32_t _touch_prev_x;
    std::int32_t _touch_prev_y;
    std::int32_t _touch_begin_x;
    std::int32_t _touch_begin_y;
    std::int32_t _touch_begin_raw_x;
    std::int32_t _touch_begin_raw_y;
    std::int32_t _touch_begin_msec;

    virtual void init_impl(void) = 0;
    virtual input_t update_impl(void);
    virtual std::uint32_t get_raw_impl(std::uint32_t value) = 0;
  };

  class LGUI_InputManager : public LovyanUOM
  {
  protected:
    void init_impl(void) override;
    std::uint32_t get_raw_impl(std::uint32_t value) override;
  };

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32) || (ESP_PLATFORM)
  class LGUI_InputManager_ODROIDGO : public LGUI_InputManager
  {
  protected:
    std::uint32_t get_raw_impl(std::uint32_t value) override;
  };

  class LGUI_InputManager_M5StickC : public LGUI_InputManager
  {
  protected:
    std::uint32_t get_raw_impl(std::uint32_t value) override;
  };

  class LGUI_InputManager_TWatch : public LGUI_InputManager
  {
  protected:
    std::uint32_t get_raw_impl(std::uint32_t value) override;
  };

  class LGUI_InputManager_M5StackCore2 : public LGUI_InputManager
  {
  protected:
    std::uint32_t get_raw_impl(std::uint32_t value) override;
  };
#endif
}

#endif
