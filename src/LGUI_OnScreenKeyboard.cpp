#include <LGUI_OnScreenKeyboard.h>
#include <LovyanGFX.hpp>

namespace lgui
{
  enum 
  { TABLECOUNT  = 4
  , ROWCOUNT    = 4
  , COLUMNCOUNT =11
  , KEYWIDTH   = 29
  };

  static const char BS   = 0x08;
  static const char DEL  = 0x7f;
  static const char LEFT = 0x11;
  static const char RIGH = 0x13;
  static LGFX_Sprite sptmp;

  static constexpr char _chartbl[TABLECOUNT][ROWCOUNT][COLUMNCOUNT] 
     = {{{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', BS }
       , {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', DEL}
       , {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '/',LEFT}
       , {'z', 'x', 'c', 'v', 'b', 'n', 'm', ' ', '.', '@',RIGH}
       }
      , {{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', BS }
       , {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', DEL}
       , {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '/',LEFT}
       , {'Z', 'X', 'C', 'V', 'B', 'N', 'M', ' ', '.', '@',RIGH}
       }
      , {{'!', '"', '#', '$', '%', '&','\'', '`', '^', '~', BS }
       , {'\t','<', '>', '[', ']', '{', '}', '(', ')', '\\', DEL}
       , {'\r','|', ';', ':', '_', '=', '+', '-', '*', '/',LEFT}
       , {'\n','.', '.', '.', '.', '?', ',', ' ', '.', '@',RIGH}
       }

      , {{0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, BS }
       , {0x81, 0x85, 0x89, 0x8d, 0x91, 0x95, 0x99, 0x9d, 0xa1, 0xa5, DEL}
       , {0x82, 0x86, 0x8a, 0x8e, 0x92, 0x96, 0x9a, 0x9e, 0xa2, 0xa6, LEFT}
       , {0x83, 0x87, 0x8b, 0x8f, 0x93, 0x97, 0x9b, 0x9f, 0xa3, 0xa7, RIGH}
      }};

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

  bool LGUI_OnScreenKeyboard::onDrawCell(LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool redraw)
  {
    if (!redraw) return false;

    char str[2] = { _chartbl[_panel][row][column], 0 };
    const char* pstr = str;
    switch (str[0]) {
    case BS  : pstr = "BS";  break;
    case DEL : pstr = "DEL"; break;
    case LEFT: pstr = "<<";  break;
    case RIGH: pstr = ">>";  break;
    case '\t': pstr = "TAB"; break;
    case '\r': pstr = "CR";  break;
    case '\n': pstr = "LF";  break;
    }
    gfx->drawString(pstr, gfx->width() >> 1, gfx->height() >> 1);
  //gfx->setTextSize((float)random(5,30)/10);
  //gfx->drawString(pstr, random(-2,2) + (gfx->width() >> 1), random(-2,2) + (gfx->height() >> 1));
  //gfx->drawString(pstr, (gfx->width() >> 1) + (((column<<2) + (_loop_counter>>2))&63) - 32, random(-2,2) + (gfx->height() >> 1));
    return true;
  }

  void LGUI_OnScreenKeyboard::onInput(input_t code)
  {
    if (code == input_change) { if (++_panel == TABLECOUNT - 1) _panel = 0; _drawall = true; }
    if (code == input_enter ) {_keyCode = _chartbl[_panel][_row][_col]; }

    if (_target_gui)
    {
//    _target_gui.
    }
  }

  void LGUI_OnScreenKeyboard::setup_impl(void)
  {
    LGUI_GridView::setup_impl();
    _columnCount = COLUMNCOUNT;
    _rowCount = ROWCOUNT;
    _columnSelect = true;
    _window_rect.left   = _window_dest.left;
    _window_rect.right  = _window_dest.right;
    _window_rect.top    = _window_dest.bottom;
    _window_rect.bottom = _window_dest.bottom;
    _cursor_rect.left   = _cursor_dest.left;
    _cursor_rect.right  = _cursor_dest.right;
    _cursor_rect.top    = _window_dest.bottom;
    _cursor_rect.bottom = _window_dest.bottom;
  }
}

