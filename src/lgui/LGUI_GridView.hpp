#ifndef LGUI_GRIDVIEW_HPP_
#define LGUI_GRIDVIEW_HPP_

#include "LGUI_Scrollable.hpp"

namespace lgui
{
  class LGUI_GridView : public LGUI_Scrollable
  {
  public:
    void setDefaultColumnWidth(std::int32_t width);
    void setColumnWidth(std::uint32_t column, std::int32_t width);

    void setDefaultRowHeight(std::int32_t height);
    void setRowHeight(std::uint32_t row, std::int32_t height);

    /// 指定したセルにカーソルを移動します。
    void selectCell(std::int32_t column, std::int32_t row);

    /// 指定したセルを次回のloopで再描画します。
    void invalidateCell(std::int32_t column, std::int32_t row);

    /// 指定したセルにenter操作をします。
    void enterCell(std::int32_t column, std::int32_t row);


    // callback event handlers.
    /// セルの描画時に呼ばれます。描画処理を記述してください。
    void (*onDrawCell  )(LGUI_GridView* sender, LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool& redraw) = nullptr;

    /// カーソルの移動前に呼ばれます。移動を禁止したり別のセルへ移動させたい場合に処理を記述してください。
    void (*onSelectCell)(LGUI_GridView* sender, std::int32_t& newColumn, std::int32_t& newRow, bool& canselect);

    /// カーソルが移動し、スクロールやスムース移動が静止した時に呼ばれます。
    /// ※ 連続移動時には呼ばれない事があるので注意してください。
    void (*onSelectedCell)(LGUI_GridView* sender, std::int32_t column, std::int32_t row);

    /// カーソルのあるセルにてenter操作が行われた時に呼ばれます。
    void (*onEnterCell)(LGUI_GridView* sender, std::int32_t column, std::int32_t row);

    std::int32_t cursorRow = 0;
    std::int32_t cursorColumn = 0;
    std::int32_t rowCount = 1;
    std::int32_t columnCount = 1;
    std::int32_t fixedRowCount = 0;
    std::int32_t fixedColumnCount = 0;

    std::int32_t borderWidth = 1;

    bool simpleMode = true;
    bool fixedRowSelect = false;
    bool fixedColumnSelect = false;
    bool columnSelect = true;

  protected:
    bool _prev_columnSelect = true;
    Rectangle _cursor_rect_move;
    Rectangle _cursor_rect_dest;

    void updateCursorRect(bool adjustscroll = false);

    bool show_impl(void) override;
    bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) override;
    bool input_impl(input_t& code) override;
    bool touch_impl(touch_t touchstate, LovyanUOM* uom) override;
    void update_scroll_impl(void) override;
    std::int32_t getColumnPos(std::int32_t column, std::int32_t width);
    std::int32_t getRowPos(std::int32_t row, std::int32_t height);
    std::int32_t getColumnIndex(std::int32_t x);
    std::int32_t getRowIndex(std::int32_t y);
    virtual void drawCell_impl(LovyanGFX* canvas, std::int32_t column, std::int32_t row, bool& redraw);
    virtual void selectCell_impl(std::int32_t& newColumn, std::int32_t& newRow);
    virtual bool enterCell_impl(std::int32_t column, std::int32_t row);

    //const Rectangle& getCursorRectMove(void) const { return _cursor_rect_move; }
    //const Rectangle& getCursorRectDest(void) const { return _cursor_rect_dest; }
    //void setCursorRectDest(const Rectangle& rect) { _cursor_rect_dest = rect; }
    //void setCursorRectDest(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) { _cursor_rect_dest = { x, y, x+w-1, y+h-1 }; }

  private:
    PositionStocker _pos_columns;
    PositionStocker _pos_rows;
    bool _unselected = true;
    std::uint32_t _update_count = 0;
  };
}

#endif
