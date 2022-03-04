#include "LGUI_GridView.hpp"
#include "../LovyanGUI.hpp"

namespace lgui
{
  std::int32_t LGUI_GridView::getColumnPos(std::int32_t column, std::int32_t width)
  {
    if (columnCount && _pos_columns.empty() && _pos_columns.defaultSize == 0)
    {
      return (width + borderWidth) * column / columnCount;
    }
    return column == 0 ? 0 : _pos_columns.getPosition(column -1);
  }

  std::int32_t LGUI_GridView::getColumnIndex(std::int32_t x)
  {
    if (_pos_columns.empty() && _pos_columns.defaultSize == 0)
    {
      return x * columnCount / (_visible_client_rect.width() + borderWidth);
    }
    return _pos_columns.getIndex(x);
  }

  void LGUI_GridView::setDefaultColumnWidth(std::int32_t width)
  {
    _pos_columns.defaultSize = width;
  }

  void LGUI_GridView::setColumnWidth(std::uint32_t column, std::int32_t width)
  {
    _pos_columns.setSize(width, column);
    update_scroll_impl();
  }

  std::int32_t LGUI_GridView::getRowPos(std::int32_t row, std::int32_t height)
  {
    if (rowCount && _pos_rows.empty() && _pos_rows.defaultSize == 0)
    {
      return (height + borderWidth) * row / rowCount;
    }
    return row == 0 ? 0 : _pos_rows.getPosition(row -1);
  }

  std::int32_t LGUI_GridView::getRowIndex(std::int32_t y)
  {
    if (_pos_rows.empty() && _pos_rows.defaultSize == 0)
    {
      return y * rowCount / (_visible_client_rect.height() + borderWidth);
    }
    return _pos_rows.getIndex(y);
  }

  void LGUI_GridView::setDefaultRowHeight(std::int32_t height)
  {
    _pos_rows.defaultSize = height;
  }

  void LGUI_GridView::setRowHeight(std::uint32_t row, std::int32_t height)
  {
    _pos_rows.setSize(height, row);
    update_scroll_impl();
  }

  void LGUI_GridView::selectCell(std::int32_t column, std::int32_t row)
  {
    selectCell_impl(column, row);

    if (onSelectCell)
    {
      bool canselect = true;
      onSelectCell(this, column, row, canselect);
      if (!canselect) return;
    }

    if (cursorColumn != column
     || cursorRow != row)
    {
      _unselected = true;
      cursorColumn = column;
      cursorRow = row;
    }
    updateCursorRect(true);
  }

  void LGUI_GridView::selectCell_impl(std::int32_t& column, std::int32_t& row)
  {
    // 端を超えた場合は反対側の端を選択する
    std::int32_t col_min = fixedColumnSelect ? 0 : fixedColumnCount;
    if (column >= columnCount) column = col_min;
    else if (column < col_min) column = columnCount - 1;

    std::int32_t row_min = fixedRowSelect ? 0 : fixedRowCount;
    if (row >= rowCount) row = row_min;
    else if (row < row_min) row = rowCount - 1;
  }

  void LGUI_GridView::enterCell(std::int32_t column, std::int32_t row)
  {
    setRedraw();

    if (cursorColumn != column || cursorRow != row) selectCell(column, row);

    if (enterCell_impl(column, row) && onEnterCell) onEnterCell(this, column, row);

    update_scroll_impl();
    updateCursorRect(true);
  }

  bool LGUI_GridView::enterCell_impl(std::int32_t column, std::int32_t row)
  {
    return true;
  }

  bool LGUI_GridView::show_impl(void)
  {
    updateCursorRect();
    //_cursor_rect_move.left = _cursor_rect_move.right = (_relative_hide_rect.left + _relative_hide_rect.right) >> 1;
    //_cursor_rect_move.top = _cursor_rect_move.bottom = (_relative_hide_rect.top + _relative_hide_rect.bottom) >> 1;
    return true;
  }

  bool LGUI_GridView::loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& clip)
  {
//    bool drawall = (_vscroll_move != _vscroll_dest) || (_hscroll_move != _hscroll_dest);
    LGUI_Scrollable::loop_impl(gui, has_focus, clip);
    auto clientrect = _visible_client_rect;
    if (clientrect.empty()) return true;

    auto cliprect = clip.intersect(_visible_client_rect);
    if (cursorRow    < fixedRowCount   ) cursorRow    = fixedRowCount;
    if (cursorColumn < fixedColumnCount) cursorColumn = fixedColumnCount;

    auto gfx = gui->getGFX();

    bool drawall = _redraw & redraw_body;
    if (drawall) updateCursorRect();

    auto oldrect = _cursor_rect_move;
    oldrect.offset(clientrect.left-_hscroll_move, clientrect.top-_vscroll_move);
    if (!simpleMode)
    {
      if (_prev_columnSelect)
      {
        oldrect.left  = clientrect.left;
        oldrect.right = clientrect.right;
      }
      else
      {
        oldrect.top    = clientrect.top;
        oldrect.bottom = clientrect.bottom;
      }
    }
    _prev_columnSelect = columnSelect;

    bool _redraw = (_cursor_rect_move != _cursor_rect_dest);
    // カーソル移動処理（全再描画時は移動速度を上げる）
    if (_redraw) _cursor_rect_move.moveTo(_cursor_rect_dest, gui->smoothMove, drawall ? 32 : 16);
    auto cursorrect = _cursor_rect_move;
    cursorrect.offset(clientrect.left-_hscroll_move, clientrect.top-_vscroll_move);
    auto newrect = cursorrect;
    if (!simpleMode)
    {
      if (columnSelect)
      {
        newrect.left  = clientrect.left;
        newrect.right = clientrect.right;
      }
      else
      {
        newrect.top    = clientrect.top;
        newrect.bottom = clientrect.bottom;
      }
    }
    Rectangle rect;

    std::int32_t client_width = clientrect.width();
    std::int32_t fixed_x = 0;
    if (fixedColumnCount) fixed_x = getColumnPos(fixedColumnCount, client_width);
    std::int32_t colStart = getColumnIndex(_hscroll_move + fixed_x     );
    std::int32_t colEnd   = getColumnIndex(_hscroll_move + client_width) + 1;
    if (colEnd > columnCount) colEnd = columnCount;

    std::int32_t client_height = clientrect.height();
    std::int32_t fixed_y = 0;
    if (fixedRowCount) fixed_y = getRowPos(fixedRowCount, client_height);
    std::int32_t rowStart = getRowIndex(_vscroll_move + fixed_y      );
    std::int32_t rowEnd   = getRowIndex(_vscroll_move + client_height) + 1;
    if (rowEnd > rowCount) rowEnd = rowCount;

    std::int32_t update_cell = _update_count;
    for (std::int32_t row = 0; row <= rowEnd; ++row)
    {
      if (row == fixedRowCount)
      {
        row = rowStart;
        clientrect.top += fixed_y;
        client_height -= fixed_y;
      }
      rect.top    = clientrect.top + getRowPos(row  , client_height);
      rect.bottom = clientrect.top + getRowPos(row+1, client_height) - 1;

      if (row >= fixedRowCount)
      {
        rect.top    -= _vscroll_move + fixed_y;
        rect.bottom -= _vscroll_move + fixed_y;
      }
      if (rect.top > clientrect.bottom) break;
      if (row == rowEnd)
      {
        rect.bottom = clientrect.bottom;
      }

      auto r = clientrect.intersect(cliprect);
      gfx->setClipRect(r.left, r.top, r.width(), r.height());

      for (std::int32_t col = 0; col <= colEnd; ++col)
      {
        if (col == fixedColumnCount)
        {
          col = colStart;
          auto r = clientrect;
          r.left += fixed_x;
          r.right += fixed_x;
          r = cliprect.intersect(r);
          gfx->setClipRect(r.left, r.top, r.width(), r.height());
        }
        rect.left  = clientrect.left + getColumnPos(col  , client_width);
        rect.right = clientrect.left + getColumnPos(col+1, client_width) - 1;

        if (col >= fixedColumnCount && _hscroll_move)
        {
          rect.left  -= _hscroll_move;
          rect.right -= _hscroll_move;
        }
        if (rect.left > clientrect.right) break;

        if (gui->checkFullMasked(rect)) continue;

        if (col == colEnd)
        {
          rect.right = clientrect.right;
        }

        bool redraw = drawall || (_redraw && (rect.intersectsWith(oldrect) || rect.intersectsWith(newrect)));
        if (!redraw)
        {
          redraw = gui->hasInvalidate(rect);
          if (!redraw && 0 != --update_cell) continue;
        }
        auto canvas = gui->getCanvas(rect.width(), rect.height());
        bool fixed = row < fixedRowCount || col < fixedColumnCount;
        canvas->clear(fixed ? gui->color.fixed : gui->color.back);
//canvas->clear(esp_random());
        if (col == colEnd || row == rowEnd)
        {
          if (redraw)
          {
            gui->pushCanvas(rect);
          }
          continue;
        }

        if (!fixed && rect.intersectsWith(newrect))
        {
          if (!simpleMode && newrect != cursorrect)
          { // サブカーソル描画
/*
            canvas->setColor( gui->color.subCursor[ has_focus ]);
            canvas->fillRect( newrect.left - rect.left
                            , newrect.top  - rect.top
                            , newrect.width()
                            , newrect.height()
                            );
/*/
            canvas->setColor( gui->color.cursor[ has_focus ]);
            if (columnSelect) {
              canvas->fillRect( newrect.left - rect.left
                              , newrect.top  - rect.top
                              , newrect.width()
                              , 2
                              );
              canvas->fillRect( newrect.left   - rect.left
                              , newrect.bottom - rect.top - 1
                              , newrect.width()
                              , 2
                              );
            } else {
              canvas->fillRect( newrect.left - rect.left
                              , newrect.top  - rect.top + 2
                              , 2
                              , newrect.height() - 4
                              );
              canvas->fillRect( newrect.right - rect.left - 1
                              , newrect.top   - rect.top
                              , 2
                              , newrect.height()
                              );
            }
//*/
          }
          if (rect.intersectsWith(cursorrect))
          { // カーソル描画
            canvas->setColor( gui->color.cursor[ has_focus ]);
            canvas->fillRect( cursorrect.left - rect.left
                            , cursorrect.top  - rect.top
                            , cursorrect.width()
                            , cursorrect.height()
                            );

          }
        }
        if (0 < borderWidth)
        { // グリッド罫線描画
          canvas->setColor(gui->color.border);
          canvas->fillRect(0, rect.height() - borderWidth, rect.width(), borderWidth);
          canvas->fillRect(rect.width() - borderWidth, 0, borderWidth, rect.height() - borderWidth);
          canvas->setClipRect(0, 0, rect.width() - borderWidth, rect.height() - borderWidth);
        }

        drawCell_impl(canvas, col, row, redraw);
        if (redraw)
        {
          gui->pushCanvas(rect);
        }
        else
        {
          if (update_cell == 0)
          {
            ++_update_count;
            ++update_cell;
          }
        }
      }
    }
    if (update_cell < 0)
    {
      ++_update_count;
    }
    else
    {
      _update_count = 1;
    }
//*/
    gfx->clearClipRect();

    if (_unselected
     && _vscroll_move == _vscroll_dest
     && _hscroll_move == _hscroll_dest
     && _cursor_rect_move == _cursor_rect_dest
     ) {
      _unselected = false;
      if (onSelectedCell)
      {
        onSelectedCell(this, cursorColumn, cursorRow);
      }
    }
    return true;
  }

  void LGUI_GridView::drawCell_impl(LovyanGFX* canvas, std::int32_t column, std::int32_t row, bool& redraw)
  {
    if (onDrawCell) { onDrawCell(this, canvas, column, row, redraw); }
  }

  void LGUI_GridView::updateCursorRect(bool adjustscroll)
  {
    auto rect = getTargetRect();
    auto width  = std::max<std::int32_t>(1, rect.width() - ((frameWidth<<1) + visibleVScroll * scrollBarWidth));
    auto height = std::max<std::int32_t>(1, rect.height() - ((frameWidth<<1) + visibleHScroll * scrollBarWidth));
    //*/
    auto col = cursorColumn;
    _cursor_rect_dest.left   = getColumnPos(col  ,width);
    _cursor_rect_dest.right  = getColumnPos(col+1,width)-borderWidth -1;
    if (col >= fixedColumnCount)
    {
      if (adjustscroll) {
        std::int32_t fixed_left = getColumnPos(fixedColumnCount, width);
        if (_hscroll_dest > _cursor_rect_dest.left - fixed_left)
        {
          _hscroll_dest = _cursor_rect_dest.left - fixed_left;
        }
        else if (_hscroll_dest < _cursor_rect_dest.right - width)
        {
          _hscroll_dest = _cursor_rect_dest.right - width + 1;
        }
      }
    }


    auto row = cursorRow;
    _cursor_rect_dest.top    = getRowPos(row  , height);
    _cursor_rect_dest.bottom = getRowPos(row+1, height)-borderWidth -1;
    if (row >= fixedRowCount)
    {
      if (adjustscroll) {
        std::int32_t fixed_top = getRowPos(fixedRowCount, height);
        if (_vscroll_dest > _cursor_rect_dest.top - fixed_top)
        {
          _vscroll_dest = _cursor_rect_dest.top - fixed_top;
        }
        else if (_vscroll_dest < _cursor_rect_dest.bottom - height)
        {
          _vscroll_dest = _cursor_rect_dest.bottom - height + 1;
        }
      }
    }
  }

  bool LGUI_GridView::input_impl(input_t& code)
  {
    if (simpleMode)
    {
      switch (code)
      {
//    case input_fn1:
      case input_back:
        if (cursorColumn == (fixedColumnSelect ? 0 : fixedColumnCount))
        {
          selectCell(columnCount-1, cursorRow-1);
        }
        else
        {
          selectCell(cursorColumn-1, cursorRow);
        }
        return false;

//    case input_fn2:
      case input_tab  :
        if (cursorColumn+1 == columnCount)
        {
          selectCell(fixedColumnSelect ? 0 : fixedColumnCount, cursorRow+1);
        }
        else
        {
          selectCell(cursorColumn+1, cursorRow);
        }
        return false;

      default: break;
      }
    }

    switch (code)
    {
    case input_none : return false;
    case input_left : selectCell(cursorColumn-1, cursorRow  ); return false;
    case input_up   : selectCell(cursorColumn  , cursorRow-1); return false;
    case input_right: selectCell(cursorColumn+1, cursorRow  ); return false;
    case input_down : selectCell(cursorColumn  , cursorRow+1); return false;

    case input_fn1:
    case input_fn2:
      columnSelect = !columnSelect;
      _redraw = static_cast<redraw_t>(_redraw | redraw_body);
      return false;

    case input_back :
      if (columnSelect)
      {
        selectCell(cursorColumn-1, cursorRow);
/*
        if (cursorColumn == (fixedColumnSelect ? 0 : fixedColumnCount))
        {
          selectCell(columnCount-1, cursorRow-1);
        }
        else
        {
          selectCell(cursorColumn-1, cursorRow);
        }
//*/
      }
      else
      {
        selectCell(cursorColumn, cursorRow-1);
/*
        if (cursorRow == (fixedRowSelect ? 0 : fixedRowCount))
        {
          selectCell(cursorColumn, rowCount-1);
        }
        else
        {
          selectCell(cursorColumn, cursorRow-1);
        }
//*/
      }
      return false;

    case input_tab  :
      if (columnSelect)
      {
        selectCell(cursorColumn+1, cursorRow);
/*
        if (cursorColumn+1 == columnCount)
        {
          selectCell(fixedColumnSelect ? 0 : fixedColumnCount, cursorRow+1);
        }
        else
        {
          selectCell(cursorColumn+1, cursorRow);
        }
//*/
      }
      else
      {
        selectCell(cursorColumn, cursorRow+1);
/*
        if (cursorRow+1 == rowCount)
        {
          selectCell(cursorColumn, fixedRowSelect ? 0 : fixedRowCount);
        }
        else
        {
          selectCell(cursorColumn, cursorRow+1);
        }
//*/
      }
      return false;

    case input_ok:
    case input_enter :

      enterCell(cursorColumn, cursorRow);
      break;

    default: break;
    }
    setRedraw();
    return true;
  }

  bool LGUI_GridView::touch_impl(touch_t touchstate, LovyanUOM* uom)
  {
    if (touchstate & touch_t::mask_moving && !simpleMode && !(touchstate & touch_t::mask_holding))
    {
      return LGUI_Scrollable::touch_impl(touchstate, uom);
    }
    else if ((touchstate & touch_t::mask_moving) || (touchstate == touch_t::touch_end))
    {
      std::int32_t x, y;
      uom->getTouchXY(&x, &y);
      x -= _visible_client_rect.left;
      y -= _visible_client_rect.top;
      std::int32_t col = std::max<std::int32_t>(std::min(getColumnIndex(_hscroll_move + x), columnCount-1), fixedColumnCount);
      std::int32_t row = std::max<std::int32_t>(std::min(getRowIndex(   _vscroll_move + y), rowCount-1   ), fixedRowCount);

      if (col >= columnCount) col = columnCount-1;
      if (row >= rowCount)    row = rowCount-1;
      if ((touchstate & ~(touch_t::mask_holding | touch_t::mask_moving)) == touch_t::touch_end && cursorColumn == col && cursorRow == row)
      {
        enterCell(col, row);
      }
      else
      {
        selectCell(col, row);
      }
    }
    return true;
  }

  void LGUI_GridView::update_scroll_impl(void)
  {
    auto rect = getTargetRect();
    auto width  = std::max<std::int32_t>(1, rect.width() - ((frameWidth<<1) + visibleVScroll * scrollBarWidth));
    auto height = std::max<std::int32_t>(1, rect.height() - ((frameWidth<<1) + visibleHScroll * scrollBarWidth));

    auto tmp = getColumnPos(columnCount, width) - borderWidth;
    if (_hscroll_size != tmp)
    {
      _hscroll_size = tmp;
      _redraw = static_cast<redraw_t>(_redraw | redraw_hscroll);
    }

    tmp = getRowPos(rowCount, height) - borderWidth;
    if (_vscroll_size != tmp)
    {
      _vscroll_size = tmp;
      _redraw = static_cast<redraw_t>(_redraw | redraw_vscroll);
    }
  }
}

