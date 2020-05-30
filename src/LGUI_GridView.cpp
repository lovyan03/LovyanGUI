#include <LGUI_GridView.h>
#include <LovyanGFX.hpp>

namespace lgui
{
  static LGFX_Sprite sp_buf;

  void LGUI_GridView::setup_impl(void)
  {
    _cursor_rect = _window_rect;

    sp_buf.setColorDepth(_color_depth);
    sp_buf.createSprite(1, 1);
    sp_buf.setTextDatum(textdatum_t::middle_center);
    sp_buf.setTextColor(_color_text);
    sp_buf.setFont(&fonts::Font2);
    sp_buf.setPaletteColor(_color_back,  0x444444U); // base   color
    sp_buf.setPaletteColor(_color_frame, 0x111111U); // frame  color
    sp_buf.setPaletteColor(_color_text,  0xFFFFFFU); // text   color
  }

  bool LGUI_GridView::loop_impl(bool drawall, const std::vector<const Rectangle*>* rects)
  {
    bool hasFocus = (rects == nullptr || rects->empty());

    sp_buf.setPaletteColor(_color_focus, hasFocus ? 0x5555FFU : 0x888888U); // cursor color

    drawall |= _drawall;
    if (drawall) updateCursorRect();

    auto oldrect = _cursor_rect;
    if (_columnSelect)
    {
      oldrect.top = _window_rect.top;
      oldrect.bottom = _window_rect.bottom;
    }

    bool moving = _cursor_rect.moveTo(_cursor_dest, 16, _moveSpeed);
    auto newrect = _cursor_rect;
    if (_columnSelect)
    {
      newrect.top = _window_rect.top;
      newrect.bottom = _window_rect.bottom;
    }

    int updatecell = _loop_counter % (_rowCount * _columnCount);
    Rectangle rect;
    for (int row = 0; row < _rowCount; ++row)
    {
      rect.top    = _window_rect.top + (_window_rect.height()-1) *  row    / _rowCount;
      rect.bottom = _window_rect.top + (_window_rect.height()-1) * (row+1) / _rowCount;
      for (int col = 0; col < _columnCount; ++col)
      {
        rect.left  = _window_rect.left + (_window_rect.width()-1) *  col    / _columnCount;
        rect.right = _window_rect.left + (_window_rect.width()-1) * (col+1) / _columnCount;

        bool redraw = drawall || (moving && (rect.intersectsWith(oldrect) || rect.intersectsWith(newrect)));
        if (!redraw && 0 != updatecell--) continue;

        if (sp_buf.width() != rect.width() || sp_buf.height() != rect.height())
        {
          sp_buf.createSprite(rect.width(), rect.height());
        }
        sp_buf.clear(_color_back);

        sp_buf.setColor(_color_focus);
        if (_columnSelect && rect.intersectsWith(newrect)) {
          sp_buf.fillRect( newrect.left - rect.left, newrect.top    - rect.top + 1, newrect.width(), 2 );
          sp_buf.fillRect( newrect.left - rect.left, newrect.bottom - rect.top - 2, newrect.width(), 2 );
          sp_buf.fillRect( newrect.left - rect.left    , newrect.top  - rect.top + 3, 2, newrect.height() - 6 );
          sp_buf.fillRect( newrect.right- rect.left - 1, newrect.top  - rect.top + 3, 2, newrect.height() - 6 );
        }
        if (rect.intersectsWith(_cursor_rect))
        {
          sp_buf.fillRect( _cursor_rect.left - rect.left
                         , _cursor_rect.top  - rect.top
                         , _cursor_rect.width()
                         , _cursor_rect.height()
                         );
        }

        if (onDrawCell(&sp_buf, col, row, redraw))
        {
          sp_buf.setColor(_color_frame);
          sp_buf.drawRect(0, 0, sp_buf.width(), sp_buf.height(), 2);
          sp_buf.setColor(_color_focus);
          if (row == 0) sp_buf.drawFastHLine(0, 0, sp_buf.width());
          if (row+1 == _rowCount ) sp_buf.drawFastHLine(0, sp_buf.height()-1, sp_buf.width());
          if (col == 0) sp_buf.drawFastVLine(0, 0, sp_buf.height());
          if (col+1==_columnCount) sp_buf.drawFastVLine(sp_buf.width()-1, 0, sp_buf.height());

          if (rects != nullptr)
          {
            sp_buf.setColor(_color_mask);
            for (auto& r : *rects)
            {
              if (r->intersectsWith(rect))
              {
                sp_buf.fillRect( r->left - rect.left
                               , r->top  - rect.top
                               , r->width()
                               , r->height()
                               );
              }
            }
          }

          //if (rect.equal(_window_dest)) {
          sp_buf.pushSprite(_lcd, rect.left, rect.top, 0);
          //} else {
          //  _lcd->setClipRect(rect.left, rect.top, rect.width(), rect.height());
          //  int diff = abs(_window_rect.left   - _window_dest.left  )
          //           + abs(_window_rect.top    - _window_dest.top   )
          //           + abs(_window_rect.right  - _window_dest.right )
          //           + abs(_window_rect.bottom - _window_dest.bottom)
          //           ;
          //  sp_buf.pushRotateZoom(_lcd, (rect.left+rect.right)>>1, (rect.top+rect.bottom)>>1, -diff, 1, 1);
          //  _lcd->clearClipRect();
          //}
        }
      }
    }
    sp_buf.createSprite(1, 1);
    _drawall = false;
    return true;
  }

  bool LGUI_GridView::onDrawCell(LovyanGFX*, std::int32_t, std::int32_t, bool redraw) { return redraw; }
  void LGUI_GridView::onInput(input_t) {}
  void LGUI_GridView::onSelectCell(std::int32_t, std::int32_t) {}

  void LGUI_GridView::selectCell(std::int32_t column, std::int32_t row)
  {
    _col = column;
    _row = row;
    updateCursorRect();

    onSelectCell(_col, _row);
  }

  void LGUI_GridView::updateCursorRect(void)
  {
    auto col = _col;
    if (col < 0) _col = col = _columnCount - 1;
    else if (col >= _columnCount) { _col = col = 0; }
    _cursor_dest.left   = _window_dest.left + (_window_dest.width() -1) *  col    / _columnCount + 1;
    _cursor_dest.right  = _window_dest.left + (_window_dest.width() -1) * (col+1) / _columnCount - 1;

    auto row = _row;
    if (row < 0) { _row = row = _rowCount    - 1; }
    else if (row >= _rowCount   ) { _row = row = 0; }
    _cursor_dest.top    = _window_dest.top  + (_window_dest.height()-1) *  row    / _rowCount + 1;
    _cursor_dest.bottom = _window_dest.top  + (_window_dest.height()-1) * (row+1) / _rowCount - 1;
  }

  void LGUI_GridView::input_impl(input_t code)
  {
    if (_hiding) return;
    switch (code)
    {
    case input_none : return;
    case input_left : selectCell(_col-1, _row  ); return;
    case input_up   : selectCell(_col  , _row-1); return;
    case input_right: selectCell(_col+1, _row  ); return;
    case input_down : selectCell(_col  , _row+1); return;
    case input_tab  :
      if (_col+1 == _columnCount) { selectCell(0, _row+1); } 
      else {                        selectCell(_col+1, _row  ); }
      return;
    default: break;
    }
    _drawall = true;
    onInput(code); 
  }
}

