#include "LGUI_OnScreenKeyboard.hpp"
#include "../LovyanGUI.hpp"
#include <LovyanGFX.hpp>

namespace lgui
{
  static constexpr std::int32_t TABLECOUNT  = 3;
  static constexpr std::int32_t ROWCOUNT    = 4;
  static constexpr std::int32_t COLUMNCOUNT =11;

  static const char BS   = 0x08;
  static const char OK   = input_confirm;
  static const char LEFT = input_left ;
  static const char RIGH = input_right;
  static const char PNL  = input_fn1;
//static const char DEL  = 0x7f;
//static const char ESC  = input_cancel ;
//static const char UP   = input_up   ;
//static const char DOWN = input_down ;

  static constexpr char _chartbl[TABLECOUNT][ROWCOUNT][COLUMNCOUNT] 
     = {{{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', BS }
       , {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', PNL}
       , {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '/', OK }
       , {'z', 'x', 'c', 'v', 'b', 'n', 'm', ' ', '@',LEFT,RIGH}
       }
      , {{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', BS }
       , {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', PNL}
       , {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '/', OK }
       , {'Z', 'X', 'C', 'V', 'B', 'N', 'M', ' ', '@',LEFT,RIGH}
       }
      , {{'!', '"', '#', '$', '%', '&','\'', '`', '^', '~',  BS }
       , {'\t','<', '>', '[', ']', '{', '}', '(', ')', '\\', PNL}
       , {'\r','|', ';', ':', '_', '=', '+', '-', '*', '/',  OK }
       , {'\n','.', '.', '.', '.', '?', ',', ' ', '@',LEFT, RIGH}
       }
       };
/*
  static constexpr uint8_t _morsetbl[TABLECOUNT][ROWCOUNT][COLUMNCOUNT] 
     = {{{0x30, 0x38, 0x3c, 0x3e, 0x3f, 0x2f, 0x27, 0x23, 0x21, 0x20, 0x10 }
       , {0x12, 0x0c, 0x03, 0x0d, 0x02, 0x14, 0x0e, 0x07, 0x08, 0x19, 0}
       , {0x06, 0x0f, 0x0b, 0x1d, 0x09, 0x1f, 0x18, 0x0a, 0x1b, 0x2d, 0}
       , {0x13, 0x16, 0x15, 0x1e, 0x17, 0x05, 0x04, 0x1c, 0x6a, 0x65, 0}
       }
      , {{0x54, 0x6d, 0x25, 0x7b, 0x22, 0x37, 0x61, 0x5a, 0x7f, 0x63, 0x10}
       , {0   , 0x28, 0x50, 0x33, 0x66, 0x32, 0x64, 0x29, 0x52, 0x2a, 0}
       , {0x1a, 0x4a, 0x55, 0x47, 0x72, 0x2e, 0x35, 0x5e, 0x3d, 0x2d, 0}
       , {0   , 0   , 0   , 0   , 0   , 0x73, 0x4c, 0x1c, 0x6a, 0x65, 0}
       }

      , {{0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0x10}
       , {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0}
       , {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0}
       , {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0}
      }};
//*/
  static const char* get_cell_str(char* buffer, std::int32_t panel, std::int32_t column, std::int32_t row)
  {
    buffer[0] = _chartbl[panel][row][column];
    switch (buffer[0]) {
//    case DEL : pstr = "DEL"; break;
//    case ESC : pstr = "ESC"; break;
//    case UP  : pstr = "UP";  break;
//    case DOWN: pstr = "DWN"; break;
    case BS  : return "BS";
    case OK  : return "OK";
    case LEFT: return "<<";
    case RIGH: return ">>";
    case PNL : return "A/a";
    case '\t': return "TAB";
    case '\r': return "CR";
    case '\n': return "LF";
    default:
      buffer[1] = 0;
      return buffer;
    }
  }

  void LGUI_OnScreenKeyboard::drawCell_impl(LovyanGFX* canvas, std::int32_t column, std::int32_t row, bool& redraw)
  {
    if (!redraw) return;
    canvas->setTextDatum(middle_center);
    char buf[2];
    const char* pstr = get_cell_str(buf, _panel, column, row);
    canvas->drawString(pstr, canvas->width() >> 1, canvas->height() >> 1);
  //canvas->setTextSize((float)random(5,30)/10);
  //canvas->drawString(pstr, random(-2,2) + (canvas->width() >> 1), random(-2,2) + (canvas->height() >> 1));
  //canvas->drawString(pstr, (canvas->width() >> 1) + (((column<<2) + (_loop_counter>>2))&63) - 32, random(-2,2) + (canvas->height() >> 1));
  }

  bool LGUI_OnScreenKeyboard::input_impl(input_t& code)
  {
    if (code == input_fn1dbl || code == input_fn1)
    {
      setRedraw();
      if (++_panel == TABLECOUNT) { _panel = 0; }
      return true;
    }
    else
    if (code == input_confirm)
    {
      hide();
      return true;
    }
/*
    else
    if (code == input_enter || code == input_ok)
    {
      _keyCode = _chartbl[_panel][cursorRow][cursorColumn];
      if (_keyCode == OK)
      {
        hide();
      }
      else
      if (_target_gui && _target_gui->isVisible())
      {
        _target_gui->input((input_t)_keyCode);
      }
      return true;
    }
*/
    if (code == input_t::input_down && cursorRow == rowCount - 1)
    {
      setRedraw();
      if (++_panel == TABLECOUNT) { _panel = 0; }
      return false;
    }
    else
    if (code == input_t::input_up && cursorRow == fixedRowCount)
    {
      setRedraw();
      if (--_panel == -1) { _panel = TABLECOUNT-1; }
      return false;
    }
    return LGUI_GridView::input_impl(code);
  }

  bool LGUI_OnScreenKeyboard::touch_impl(touch_t touchstate, LovyanUOM* uom)
  {
    std::int32_t x, y;
    uom->getTouchXY(&x, &y);
    std::int32_t col = std::max<std::int32_t>(std::min(getColumnIndex(_hscroll_move + x - _visible_client_rect.left), COLUMNCOUNT-1), 0);
    std::int32_t row = std::max<std::int32_t>(std::min(getRowIndex(   _vscroll_move + y - _visible_client_rect.top ), ROWCOUNT-1   ), 0);
    if (touchstate & touch_t::mask_touch)
    {
      if (cursorColumn != col || cursorRow != row)
      {
        selectCell(col, row);
      }
      if (touchstate == touch_t::touch_begin)
      {
        showMagnifire();
      }
    }
    else
    {
      enterCell(col, row);
    }

    return true;
  }

  bool LGUI_OnScreenKeyboard::enterCell_impl(std::int32_t column, std::int32_t row)
  {
    _keyCode = _chartbl[_panel][cursorRow][cursorColumn];
    if (_keyCode == OK)
    {
      hide();
    }
    else
    if (_keyCode == input_fn1)
    {
      setRedraw();
      if (++_panel == TABLECOUNT) { _panel = 0; }
    }
    else
    if (_target_gui && _target_gui->isVisible())
    {
      _target_gui->input((input_t)_keyCode);
    }
    return true;
  }

  LGUI_OnScreenKeyboard::LGUI_OnScreenKeyboard(void)
  {
    columnCount = COLUMNCOUNT;
    rowCount = ROWCOUNT;
    columnSelect = true;
    simpleMode = false;
  }

  void LGUI_OnScreenKeyboard::setup_impl(void)
  {
  }
}

