#ifndef _LGUI_GRIDVIEW_H_
#define _LGUI_GRIDVIEW_H_

#include "LGUI_Base.h"

namespace lgui
{
  class LGUI_GridView : public LGUI_Base
  {
  public:
    void selectCell(std::int32_t column, std::int32_t row);

    virtual bool onDrawCell(LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool redraw);
    virtual void onSelectCell(std::int32_t column, std::int32_t row);
    virtual void onInput(input_t);

  protected:
    std::int32_t _row = 0;
    std::int32_t _col = 0;
    std::int32_t _columnCount = 1;
    std::int32_t _rowCount = 1;
    bool _columnSelect = false;

    void updateCursorRect(void);

    void setup_impl(void) override;
    bool loop_impl(bool drawall, const std::vector<const Rectangle*>* rects) override;
    void input_impl(input_t) override;
  };

}

#endif
