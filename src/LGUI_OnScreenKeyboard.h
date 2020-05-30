#ifndef _LGUI_ONSCREENKEYBOARD_H_
#define _LGUI_ONSCREENKEYBOARD_H_

#include "LGUI_GridView.h"

namespace lgui
{
  class LGUI_OnScreenKeyboard : public LGUI_GridView
  {
  public:
    char getKeyCode() { char res = _keyCode; _keyCode = 0; return res; }
    void setInputGUI(LGUI_Base* gui) { _target_gui = gui; }

  private:
    LGUI_Base* _target_gui = nullptr;
    int _panel = 0;
    char _keyCode = 0;

    bool onDrawCell(LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool redraw) override;
    void onInput(input_t code) override;

    void setup_impl(void) override;
//    bool loop_impl(bool drawall) override;
//    void drawColumn(int col, const Rectangle& rect);
//    void drawColumn(LovyanGFX* sp, int col, const Rectangle& rect);
  };

}

/*

public:

  static bool useTextbox;
  static bool useOver0x80Chars;

  static uint32_t fontColor[2];
  static uint32_t backColor[2];
  static uint32_t frameColor[2];
  static uint32_t textboxFontColor;
  static uint32_t textboxBackColor;
  static uint_fast8_t keyHeight;

  static uint_fast8_t maxlength;
  static void setFont(IFont* f) { _font = f; }

  void setup(const String& value = "");
  bool loop();
  void close();

  void draw();
  void clearString();
  String getString() const { return _string; }
  void setString(const String& value = "");
  char getKeyCode() const { return _keyCode; }
private:
  static const IFont* _font;

  enum eState
  { APPEAR
  , LEFTRIGHT
  , UPDOWN
  , MORSE
  };
  eState _state;
  int8_t _fn = 0;
  int8_t _tbl = 0;
  int8_t _col = 0;
  int8_t _row = 0;
  int8_t _cursorPos = 0;
  int16_t _cursorX = 0;
  uint32_t _msec = 0;
  uint32_t _msecLast = 0;
  int _repeat;
  char _keyCode;
  char _pressed;
  String _string;
  uint8_t _morseInputBuf;
  bool _flgFACESKB;
  M5ButtonDrawer _btnDrawer;
  uint8_t _btnHeight;

  int getX(int col) const;
  int getY(int row) const;
  void updateButton();
  void switchTable();
  bool inputKB(char key);
  void pressKey();
  void pressKey(char keycode);
  void clearMorse();
  void pressMorse(bool longTone);
  void inputMorse();
  void drawKeyTop(int c, int r, int x, int y, int keyh);
  void drawMorse(uint8_t m, int x, int y, uint16_t color);
  void drawTextbox();
  void drawKeyboard(int h = -1);
  void drawColumn(int col);
  void drawColumn(int col, int x, int y, int h);
  void applyFont();
  bool appear();
};
//*/
#endif
