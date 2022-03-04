
#include "LGUI_InputManager.hpp"
#include "../LovyanGUI.hpp"

#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (ESP_PLATFORM)
#include <soc/sens_reg.h>
#endif

namespace lgui
{
  void LovyanUOM::init(void)
  {
    _raw_before_db = 0;
    _raw_after_db = 0;
    _raw_after_db_prev = 0;
    _msec = 0;
    _msec_last = 0;
    _msec_db_changed = 0;
    _msec_raw_changed = 0;
    _gpio.assign(raw_key_max, -1);

    init_impl();
  }

  void LovyanUOM::addCommand(std::uint32_t value, const std::uint32_t* inputs)
  {
    commands.emplace_back(command_holder_t(value, inputs));
  }

  void LovyanUOM::clearCommand(void)
  {
    commands.clear();
  }


  void LovyanUOM::setGPIO(raw_key_t name, std::int32_t ionum, bool invert)
  {
    _gpio[name] = ionum;
    std::uint32_t val = 1U << name;
    if (invert) _raw_invert |=  val;
    else        _raw_invert &= ~val;
    if (0 <= ionum) lgfx::lgfxPinMode(ionum, lgfx::pin_mode_t::input_pullup);
  }

  input_t LovyanUOM::update(lgfx::LGFX_Device* gfx)
  {
    _msec_last = _msec;
    _msec = lgfx::millis();

    if (gfx->touch()) {
      std::int32_t x, y;
      touch_t tm = _touch_state;
      tm = static_cast<touch_t>(tm & ~touch_t::mask_change);
      if (tm) {
        _touch_prev_x = _touch_x;
        _touch_prev_y = _touch_y;
      }
      bool flgtouch = gfx->getTouchRaw(&x, &y);
// ESP_LOGE("DEBUG","%d", flgtouch);
      if (flgtouch) {
        _touch_x = x;
        _touch_y = y;
        gfx->convertRawXY(&_touch_x, &_touch_y);
        if (!(tm & touch_t::mask_moving))
        { // まだフリック開始してない
          if (tm & touch_t::mask_touch)
          { // タッチ直後でない
            if (abs(_touch_begin_x - _touch_x) > TOUCH_FLICK_DISTANCE
             || abs(_touch_begin_y - _touch_y) > TOUCH_FLICK_DISTANCE)
            {
              _touch_prev_x = _touch_begin_x;
              _touch_prev_y = _touch_begin_y;
              tm = static_cast<touch_t>(tm | touch_t::flick_begin);
            } else if (!(tm & touch_t::mask_holding))
            { // ホールド時間が経過していない
              if (_msec - _touch_begin_msec > TOUCH_HOLD_MSEC) {
                tm = touch_t::hold_begin;
              }
            }
          } else {
            tm = touch_t::touch_begin;
            _touch_begin_raw_x = x;
            _touch_begin_raw_y = y;
            _touch_begin_x = _touch_x;
            _touch_begin_y = _touch_y;
            _touch_begin_msec = _msec;
          }
        }
      } else if (tm != touch_t::none) {
        if (tm & touch_t::mask_touch) {
          tm = static_cast<touch_t>((tm | touch_t::mask_change) & ~touch_t::mask_touch);
        } else {
          tm = touch_t::none;
        }
      }
//if (_touch_state != tm) ESP_LOGI("LGUI", "TM:%02x", tm);
      _touch_state = tm;
    }

    // 現在の入力状態を取得
    std::uint32_t raw = get_raw_impl(0);
    if (onGetRaw)
    {
      onGetRaw(this, raw);
    }

    // 回転方向に応じて入力装置の上下左右を回転
    if (_rotation)
    {
      static constexpr std::uint32_t bitmask = raw_bit_t::bit_stick_l | raw_bit_t::bit_stick_u | raw_bit_t::bit_stick_r | raw_bit_t::bit_stick_d;
      std::uint32_t rr = raw & bitmask;
      raw = (raw & ~bitmask) | (((rr | rr << 4) >> _rotation) & bitmask);
    }

    bool changed = false;

    if (_raw_stack[0] != raw)
    {
      // 入力履歴を後ろにずらす。
      memmove(&_raw_stack[1], _raw_stack, (COMMAND_STACK_LENGTH - 1) * sizeof(std::uint32_t));
      _raw_stack[0] = raw;
      _msec_raw_changed = _msec;
      changed = true;
    }
    else
    if (_raw_stack[1] != raw && _msec - _msec_raw_changed > _msec_long_hold)
    { // 長押し時間経過判定
      // 入力履歴を後ろにずらす。(長押し時は履歴に同じ値が２回連続して入る)
      memmove(&_raw_stack[1], _raw_stack, (COMMAND_STACK_LENGTH - 1) * sizeof(std::uint32_t));
      changed = true;
    }

    input_t res = input_none;
    if (changed)
    {
      // update処理の本体を呼び出す
      res = update_impl();
      if (onUpdate)
      {
        onUpdate(this, res);
      }

      // 入力値の変化があるか判定
      if (_input_last != res)
      {
        _msec_last_changed = _msec;
        _input_last = res;
        _msec_repeated = _msec_long_hold<<1;
        return res;
      }
    }

    // 入力値が変化してなければリピート入力処理を行う
    if (_input_last && _raw_stack[0] && _msec - _msec_last_changed > _msec_repeated)
    {
      _msec_last_changed = _msec;
      if (_msec_repeated > 10) {
        if (--_msec_repeated > _msec_long_hold) _msec_repeated = _msec_long_hold>>1;
      }
      return _input_last;
    }

    return input_none;
  }

  input_t LovyanUOM::update_impl(void)
  {
    std::uint32_t res = 0;
    std::size_t hitlen = 0;
    for (auto& cmd : commands)
    {
      std::size_t idx = 0;
      bool hit = false;
      while (!hit && cmd.inputs[idx] == _raw_stack[idx]) { hit = (cmd.inputs[++idx] == ~0U); }
      if (hit && hitlen < idx)
      {
        hitlen = idx;
        res = cmd.value;
      }
    }
    return (input_t)res;
  }

  std::int_fast8_t LovyanUOM::getStickX(void) const
  {
    return ((_raw_stack[0] & raw_bit_t::bit_stick_r) != 0)
         - ((_raw_stack[0] & raw_bit_t::bit_stick_l) != 0);
  }

  std::int_fast8_t LovyanUOM::getStickY(void) const
  {
    return ((_raw_stack[0] & raw_bit_t::bit_stick_d) != 0)
         - ((_raw_stack[0] & raw_bit_t::bit_stick_u) != 0);
  }




  void LGUI_InputManager::init_impl(void)
  {
    for (auto i : _gpio)  if (0 <= i) lgfx::lgfxPinMode(i, lgfx::pin_mode_t::input);
  }

  std::uint32_t LGUI_InputManager::get_raw_impl(std::uint32_t value)
  { // GPIOや入力デバイスの状態を確認し、入力操作を取得する
    for (int i = 0; i < raw_key_max; ++i)
    { // GPIOチェック
      if (_gpio[i] >= 0 && lgfx::gpio_in(_gpio[i]) != (bool)(_raw_invert & 1<<i))
      {
        value |= 1<<i;
      }
    }

    if (_raw_before_db != value)
    { // 入力値の変化があればまずデバウンス処理のため_raw_before_dbを更新する。
      _raw_before_db = value;
      value = _raw_after_db;
      _msec_db_changed = _msec;
    }
    else
    if (_msec - _msec_db_changed > _msec_db)
    { // 入力値の変化からデバウンス時間が経過していれば、入力操作の変化として扱う。
      _raw_after_db = value;
    }

    return value;
  }


#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (ESP_PLATFORM)

  static std::uint32_t read_adc1(std::int8_t adchannel)
  {
    CLEAR_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_START_SAR_M);
    SET_PERI_REG_BITS(SENS_SAR_MEAS_START1_REG, SENS_SAR1_EN_PAD, (1 << adchannel), SENS_SAR1_EN_PAD_S);
    SET_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_START_SAR_M);

    while (GET_PERI_REG_MASK(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_DONE_SAR) == 0); //wait for conversion
    return GET_PERI_REG_BITS2(SENS_SAR_MEAS_START1_REG, SENS_MEAS1_DATA_SAR, SENS_MEAS1_DATA_SAR_S);
  }

  std::uint32_t LGUI_InputManager_ODROIDGO::get_raw_impl(std::uint32_t value)
  {
    auto ad = read_adc1(6);  // ADC ch6 = gpio34
    if (     ad > 3000) value |= bit_stick_u;
    else if (ad > 1500) value |= bit_stick_d;

    ad = read_adc1(7);  // ADC ch7 = gpio35
    if (     ad > 3000) value |= bit_stick_r;
    else if (ad > 1500) value |= bit_stick_l;

    return LGUI_InputManager::get_raw_impl(value);
  }

  std::uint32_t LGUI_InputManager_M5StickC::get_raw_impl(std::uint32_t value)
  {
    value = LGUI_InputManager::get_raw_impl(value);

// M5StickC 電源ボタン操作の取得。 押下状態ではなく、押下があった事を取得するため、デバウンス処理より後に行う。
    std::uint8_t tmp[2] = {0x46, 0};
    if (lgfx::i2c::readRegister(I2C_NUM_1, 0x34, 0x46, &tmp[1], 1)) { // M5StickC AXP192
      if (tmp[1] & 0x02) {
        value |= bit_btn_b;
        tmp[1] = 0x02;
        lgfx::i2c::beginTransaction(I2C_NUM_1, 0x34, 400000);
        lgfx::i2c::writeBytes(I2C_NUM_1, tmp, 2);
        lgfx::i2c::endTransaction(I2C_NUM_1);
      }
    }

    return value;
  }

  std::uint32_t LGUI_InputManager_TWatch::get_raw_impl(std::uint32_t value)
  {
    value = LGUI_InputManager::get_raw_impl(value);

    std::uint8_t tmp[2] = {0x4A, 0};
    if (lgfx::i2c::readRegister(I2C_NUM_0, 0x35, 0x4A, &tmp[1], 1)) { // TWatch AXP202
      if (tmp[1] & 0x02) {
        value |= bit_btn_b;
        tmp[1] = 0x02;
        lgfx::i2c::beginTransaction(I2C_NUM_0, 0x35, 400000);
        lgfx::i2c::writeBytes(I2C_NUM_0, tmp, 2);
        lgfx::i2c::endTransaction(I2C_NUM_0);
      }
    }

    return value;
  }

  std::uint32_t LGUI_InputManager_M5StackCore2::get_raw_impl(std::uint32_t value)
  {
    value = LGUI_InputManager::get_raw_impl(value);
/* // 電源ボタン操作取得
    std::uint8_t tmp[2] = {0x46, 0};
    if (lgfx::i2c::readRegister(I2C_NUM_1, 0x34, 0x46, &tmp[1], 1)) { // M5StickC AXP192
      if (tmp[0] & 0x02) {
        value |= bit_btn_d;
        tmp[0] = 0x02;
        lgfx::i2c::writeBytes(I2C_NUM_1, 0x34, tmp, 2);
      }
    }
//*/
    // 画面外の下端エリアで仮想３ボタン入力取得
    if (_touch_begin_raw_y >= 256 && _touch_state && !(_touch_state & touch_t::mask_moving)) {
      std::int32_t idx = _touch_begin_raw_x * 3 / 320;
      if (     idx == 0) value |= bit_btn_a;
      else if (idx == 1) value |= bit_btn_b;
      else if (idx == 2) value |= bit_btn_c;
    }

    return value;
  }
#endif

}

