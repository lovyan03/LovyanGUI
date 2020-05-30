
#include <LovyanGFX.hpp>
#include <LovyanGUI.hpp>

#include <string>

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
 #define BUTTON_A_PIN 39
 #define BUTTON_B_PIN 38
 #define BUTTON_C_PIN 37
 #define BUTTON_U_PIN -1
 #define BUTTON_D_PIN -1
 #define BUTTON_L_PIN -1
 #define BUTTON_R_PIN -1
 #define BUTTON_P_PIN -1

#elif defined(ARDUINO_M5Stick_C)

 #define BUTTON_A_PIN 37
 #define BUTTON_B_PIN 39
 #define BUTTON_C_PIN -1

 #include <AXP192.h>
 AXP192 axp;

#elif defined (ARDUINO_WIO_TERMINAL)

 #define BUTTON_A_PIN 0x0200|28
 #define BUTTON_B_PIN 0x0200|27
 #define BUTTON_C_PIN 0x0200|26
 #define BUTTON_U_PIN 0x0300|20
 #define BUTTON_L_PIN 0x0300|12
 #define BUTTON_R_PIN 0x0300| 9
 #define BUTTON_D_PIN 0x0300| 8
 #define BUTTON_P_PIN 0x0300|10

#endif


static lgui::input_t getInput(void)
{

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack

  if (lgfx::gpio_in(BUTTON_A_PIN) == 0) { return lgui::input_down  ; }
  if (lgfx::gpio_in(BUTTON_B_PIN) == 0) { return lgui::input_enter ; }
  if (lgfx::gpio_in(BUTTON_C_PIN) == 0) { return lgui::input_tab   ; }

#elif defined (ARDUINO_WIO_TERMINAL)

  if (lgfx::gpio_in(BUTTON_U_PIN) == 0) { return lgui::input_up    ; }
  if (lgfx::gpio_in(BUTTON_D_PIN) == 0) { return lgui::input_down  ; }
  if (lgfx::gpio_in(BUTTON_L_PIN) == 0) { return lgui::input_left  ; }
  if (lgfx::gpio_in(BUTTON_R_PIN) == 0) { return lgui::input_right ; }
  if (lgfx::gpio_in(BUTTON_P_PIN) == 0) { return lgui::input_enter ; }
  if (lgfx::gpio_in(BUTTON_B_PIN) == 0) { return lgui::input_enter ; }
  if (lgfx::gpio_in(BUTTON_A_PIN) == 0) { return lgui::input_change; }
  if (lgfx::gpio_in(BUTTON_C_PIN) == 0) { return lgui::input_tab   ; }

#endif

  return lgui::input_none;
}

static LGFX lcd;
static lgui::LGUI_OnScreenKeyboard osk;
static lgui::LovyanGUI gui;
static int selectedcell;

static std::string gridstrs[] = {"circle","arc","triangle","rectangle","line","bezier"};
static bool gridmode[] = {true, true, true, true, true, true};


class UISelect : public lgui::LGUI_GridView
{
  void onInput(lgui::input_t code) override
  {
    hide();
    //gui.removeControl(this);
    //_window_rect.left = _window_rect.right = (_window_dest.left + _window_dest.right)>>1;
    //_window_rect.top = _window_rect.bottom = (_window_dest.top + _window_dest.bottom)>>1;
  }

  void onHide() override
  {
    gui.removeControl(this);
  }

  void onSelectCell(std::int32_t column, std::int32_t row) override
  {
    gridmode[selectedcell] = column;
  }

  bool onDrawCell(LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool redraw) override
  {
    int px = gfx->width()>>1;
    int py = gfx->height()>>1;

    auto f = (float)millis() / 128;
    auto sinf = sin(f);
    auto cosf = cos(f);

    gfx->setColor(_color_text);
    bool mode = column;

    gfx->drawString(column ? "fill" : "draw", gfx->width()>>1, gfx->height() - 16);

    switch (selectedcell) {
    case 0:
      if (mode) {
        gfx->fillCircle(gfx->width()>>1, gfx->height()>>1, (gfx->height()>>3) + sinf*15);
      } else {
        gfx->drawCircle(gfx->width()>>1, gfx->height()>>1, (gfx->height()>>3) + sinf*15);
      }
      break;
    case 1:
      if (mode) {
        gfx->fillArc(gfx->width()>>1, gfx->height()>>1, gfx->height()>>2, gfx->height()>>3, f*20, f*20+300);
      } else {
        gfx->drawArc(gfx->width()>>1, gfx->height()>>1, gfx->height()>>2, gfx->height()>>3, f*20, f*20+300);
      }
      break;
    case 2:
      if (mode) {
        gfx->fillTriangle( px + cosf      *20, py + sinf      *20
                         , px + cos(f+2.1)*20, py + sin(f+2.1)*20
                         , px + cos(f+4.2)*20, py + sin(f+4.2)*20
                         );
      } else {
        gfx->drawTriangle( px + cosf      *20, py + sinf      *20
                         , px + cos(f+2.1)*20, py + sin(f+2.1)*20
                         , px + cos(f+4.2)*20, py + sin(f+4.2)*20
                         );
      }
      break;
    case 3:
      sinf *= 20;
      cosf *= 20;
      if (mode) {
        gfx->fillRect(px - sinf, py - cosf, sinf*2, cosf*2);
      } else {
        gfx->drawRect(px - sinf, py - cosf, sinf*2, cosf*2);
      }
      break;
    case 4:
      gfx->drawLine(px + sinf, py + cosf, px - sinf, py - cosf);
      break;
    case 5:
      gfx->drawBezier(px - 20, py , px + sinf, py + cosf, px + 20, py);
      break;
    }
    return true;
  }

public:
  UISelect(void)
  {
    _columnCount = 2;
    _rowCount = 1;
  }
};


static UISelect selector;


class UIMenu : public lgui::LGUI_GridView
{
public:
  UIMenu(void)
  {
    _columnCount = 3;
    _rowCount = 2;
    _moveSpeed = 6;
//    LGUI_GridView::setup_impl();
  }

  bool onDrawCell(LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool redraw) override
  {
    int px = gfx->width()>>1;
    int py = gfx->height()>>1;

    auto f = (float)millis() / 128;
    auto sinf = sin(f);
    auto cosf = cos(f);

    gfx->setColor(_color_text);
    int focuscell = row*3+column;
    bool mode = gridmode[focuscell];

    gfx->drawString(gridstrs[focuscell].c_str(), gfx->width()>>1, gfx->height() - 16);

    switch (row) {
    case 0:
      switch (column) {
      case 0:
        if (mode) {
          gfx->fillCircle(gfx->width()>>1, gfx->height()>>1, (gfx->height()>>3) + sinf*15);
        } else {
          gfx->drawCircle(gfx->width()>>1, gfx->height()>>1, (gfx->height()>>3) + sinf*15);
        }
        break;
      case 1:
        if (mode) {
          gfx->fillArc(gfx->width()>>1, gfx->height()>>1, gfx->height()>>2, gfx->height()>>3, f*20, f*20+300);
        } else {
          gfx->drawArc(gfx->width()>>1, gfx->height()>>1, gfx->height()>>2, gfx->height()>>3, f*20, f*20+300);
        }
        break;
      case 2:
        if (mode) {
          gfx->fillTriangle( px + cosf      *20, py + sinf      *20
                           , px + cos(f+2.1)*20, py + sin(f+2.1)*20
                           , px + cos(f+4.2)*20, py + sin(f+4.2)*20
                           );
        } else {
          gfx->drawTriangle( px + cosf      *20, py + sinf      *20
                           , px + cos(f+2.1)*20, py + sin(f+2.1)*20
                           , px + cos(f+4.2)*20, py + sin(f+4.2)*20
                           );
        }
        break;
      }
      break;
    case 1:
      sinf *= 20;
      cosf *= 20;
      switch (column) {
      case 0:
        if (mode) {
          gfx->fillRect(px - sinf, py - cosf, sinf*2, cosf*2);
        } else {
          gfx->drawRect(px - sinf, py - cosf, sinf*2, cosf*2);
        }
        break;
      case 1:
        gfx->drawLine(px + sinf, py + cosf, px - sinf, py - cosf);
        break;
      case 2:
        gfx->drawBezier(px - 20, py , px + sinf, py + cosf, px + 20, py);
        break;
      }
      break;
    }
    return true;
  }

  void onInput(lgui::input_t) override
  {
    selectedcell = _col + _row*3;
    if (_col == 2 && _row == 1)
    {
      osk.setInputGUI(this);
      gui.addControl(&osk);
    }
    else
    {
      selector.selectCell(gridmode[selectedcell], 0);
      gui.addControl(&selector);
      selector.show();
    }

  }
};

static UIMenu menu;

void setup(void)
{
  lcd.begin();

  lgfx::lgfxPinMode(BUTTON_A_PIN, lgfx::pin_mode_t::input);
  lgfx::lgfxPinMode(BUTTON_B_PIN, lgfx::pin_mode_t::input);
  lgfx::lgfxPinMode(BUTTON_C_PIN, lgfx::pin_mode_t::input);
  lgfx::lgfxPinMode(BUTTON_U_PIN, lgfx::pin_mode_t::input);
  lgfx::lgfxPinMode(BUTTON_D_PIN, lgfx::pin_mode_t::input);
  lgfx::lgfxPinMode(BUTTON_L_PIN, lgfx::pin_mode_t::input);
  lgfx::lgfxPinMode(BUTTON_R_PIN, lgfx::pin_mode_t::input);
  lgfx::lgfxPinMode(BUTTON_P_PIN, lgfx::pin_mode_t::input);


  menu.setup(&lcd, (lcd.width()/20)-2, 0, (lcd.width() - lcd.width()/10)+4, (lcd.height() - lcd.height()/5)+3);
  //menu.setup(&lcd, (lcd.width()/20)-2, lcd.height()/10, (lcd.width() - lcd.width()/10)+4, (lcd.height() - lcd.height()/5)+3);

//  osk.setup(&lcd, 10, 100, 300, 130 );
  osk.setup(&lcd, 0, 140, 320, 100 );
  osk.input(lgui::input_none);
  lcd.setCursor(10,100);
  selector.setup(&lcd, 60, 60, 200, 100 );

  gui.addControl(&menu);
}


void loop(void)
{
  auto input_prev = lgui::input_t::input_none;
  uint32_t ms = millis();
  uint32_t ms_prev = ms;

  while (gui.loop())
  {
    auto input = getInput();
    if (input_prev != input)
    {
      input_prev = input;
      gui.input(input);
    }
  }
}
