
#include <LovyanGUI.hpp>

#include <string>
#include <vector>

static LGFX lcd;                        // GFXライブラリのインスタンス
static lgui::LovyanGUI gui;             // GUIライブラリのインスタンス
static lgui::LGUI_TextBox textbox;      // テキストボックス
static lgui::LGUI_GridView grid;        // グリッドビュー
static std::vector<std::string> gridstrs(26*30); // グリッドの文字列保持用変数

/**
 * @fn
 * グリッドのセルが描画される時のコールバック関数
 * @brief グリッドのセルが描画される時のコールバック関数 onDrawCellに代入する
 * @param (sender) コールバック関数を呼び出したグリッドのポインタ
 * @param (canvas) 描画対象スプライトのポインタ (セル一つ分の範囲) ※背景色・カーソル・グリッド枠線(右端と下端) が描画済
 * @param (column) 描画対象の列番号
 * @param (row) 描画対象の行番号
 * @param (&redraw) 再描画指示の有無 (trueの場合は必ず描画を行う。 falseの場合は描画するか否かは任意) ※ redraw がfalseの場合に描画更新したい場合は redrawにtrue を代入してください
 */
static void onDrawCell_grid(lgui::LGUI_GridView* sender, LovyanGFX* canvas, std::int32_t column, std::int32_t row, bool& redraw)
{
  // 再描画指示がない場合は終了
  if (!redraw) return;

  // 縦方向の中心座標を求める (テキストを描画する際に使用します)
  std::int32_t y = (canvas->height() - sender->borderWidth)>>1;

  // 描画対象のセルが左端の固定列の場合
  if (column < sender->fixedColumnCount)
  {
    // 左上隅の固定列かつ固定行の場合は何も描画しないので終了
    if (row < sender->fixedRowCount) return;

    // テキスト描画の縦位置を中心、横位置を右寄せに設定
    canvas->setTextDatum(middle_right);

    // 行数を表示。右端は２ドット空ける
    canvas->drawNumber(row - sender->fixedRowCount + 1, canvas->width()-2, y);

    return;
  }

  if (row < sender->fixedRowCount)
  {
    canvas->setTextDatum(middle_left);
    char buf[2] = { 0 };
    buf[0] = 'A' + column - sender->fixedColumnCount;
    canvas->drawString(buf, 2, y);
  }
  else
  {
    size_t index = (column - sender->fixedColumnCount) + (row - sender->fixedRowCount) * sender->columnCount;
    if (index < gridstrs.size() && !gridstrs[index].empty())
    {
      canvas->setTextDatum(middle_left);
      canvas->drawString(gridstrs[index].c_str(), 2, y);
/*
} else {
static int count;
char buf[20] = { 0 };
sprintf(buf, "%d", ++count);
canvas->setTextDatum(middle_left);
canvas->drawString(buf, 2, y);
//*/
    }
  }
}

static void onChangeState_osk(lgui::LGUI_Base* sender, lgui::state_t state)
{
  if (state == lgui::state_t::state_visible)
  { // スクリーンキーボードが表示される時のコールバック関数
    auto osk = gui.getKeyboard();
    auto osk_hight = osk->getDestRect().height();
    textbox.setDestRect(0, lcd.height()-osk_hight-16, lcd.width(), 16);
    textbox.setHideRect(0, lcd.height()-osk_hight-16, 0, 16);
    textbox.show();

    auto r = grid.getDestRect();
    r.bottom = textbox.getDestRect().top - 1;
    grid.setDestRect(r);
    grid.selectCell(grid.cursorColumn, grid.cursorRow);
  }
  else if (state == lgui::state_t::state_disappear)
  { // スクリーンキーボードが非表示になる時のコールバック関数
    // テキストボックスも非表示にする
    textbox.hide();

    // 選択中のグリッドのセルにテキストボックスの内容を反映する
    size_t index = (grid.cursorColumn - grid.fixedColumnCount) + (grid.cursorRow - grid.fixedRowCount) * grid.columnCount;
    if (index >= gridstrs.size()) {
      if (index > grid.columnCount * grid.rowCount) return;
      gridstrs.resize(index + 1);
    }
    gridstrs[index] = textbox.getText();

    auto r = grid.getDestRect();
    r.bottom = lcd.height() - 1;
    grid.setDestRect(r);
    grid.setFocus();

    //r = panel.getDestRect();
    //r.bottom = lcd.height() - 1;
    //panel.setDestRect(r);
  }
}
// グリッドのセルにEnterされた時のコールバック関数
static void onEnterCell_grid(lgui::LGUI_GridView* sender, std::int32_t column, std::int32_t row)
{
  size_t index = (grid.cursorColumn - grid.fixedColumnCount) + (grid.cursorRow - grid.fixedRowCount) * grid.columnCount;
  if (index >= gridstrs.size()) gridstrs.resize(index + 1);
  textbox.setText(gridstrs[index].c_str());

  //r = panel.getDestRect();
  //r.bottom = textbox.getDestRect().top - 1;
  //panel.setDestRect(r);
  textbox.showKeyboard();
}

// グリッドに入力が行われた時のコールバック関数
static void onInput_grid(lgui::LGUI_Base* sender, lgui::input_t key)
{
  auto me = (lgui::LGUI_GridView*)sender;
/*
  if (key == lgui::input_t::input_enter || key == lgui::input_t::input_ok)
  {
    size_t index = (grid.cursorColumn - grid.fixedColumnCount) + (grid.cursorRow - grid.fixedRowCount) * grid.columnCount;
    if (index >= gridstrs.size()) gridstrs.resize(index + 1);
    textbox.setText(gridstrs[index].c_str());

    osk.show();
    textbox.show();
    return;
  }

  if (osk.isVisible())
  {
    size_t index = (me->cursorColumn - me->fixedColumnCount) + (me->cursorRow - me->fixedRowCount) * me->columnCount;
    if (index >= gridstrs.size()) gridstrs.resize(index + 1);
    auto &str = gridstrs[index];
    if ((char)key == 0x08)
    {
      if (!str.empty()) str.resize(str.size()-1);
    }
    else
    {
      str.append(1, key);
    }
    return;
  }
  //*/
}

// グリッドに入力が行われた時のコールバック関数
static void onTouch_grid(lgui::LGUI_Base* sender, lgui::touch_t touchstate, lgui::LovyanUOM* uom)
{
}

// セルが選択される時のコールバック関数
static void onSelectCell_grid(lgui::LGUI_GridView* sender, std::int32_t& column, std::int32_t& row, bool& canselect)
{
/*
  // 選択しようとするセルが列3、行3の場合は選択を禁止する
  if (column == 3 && row == 3)
  {
    canselect = false;
  }

  // 選択しようとするセルが列2、行2の場合は強制的に列4、行4を選択させる
  if (column == 2 && row == 2)
  {
    column = 4;
    row = 4;
  }
//*/
}

// セルの選択が完了した時のコールバック関数 (スムーズ移動が完了した時に呼ばれる)
static void onSelectedCell_grid(lgui::LGUI_GridView* sender, std::int32_t column, std::int32_t row)
{
/* 
  // 選択されたセルの内容を書き換える
  size_t index = (sender->cursorColumn - sender->fixedColumnCount) + (sender->cursorRow - sender->fixedRowCount) * sender->columnCount;
  if (index >= gridstrs.size()) gridstrs.resize(index + 1);
  auto &str = gridstrs[index];
  str = "selected";
//*/
}

void setup(void)
{
  lcd.begin();
  lcd.startWrite();
  lcd.setBrightness(128);
  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

  grid.setFont(&fonts::Font2);

  //gui.setHideRect(10, 10, lcd.width()-20, 1);
  //gui.setDestRect(10, 10, lcd.width()-20, lcd.height()-20);
  //gui.visibleVScroll = true;     // 縦スクロールバー表示フラグ
  //gui.visibleHScroll = true;     // 横スクロールバー表示フラグ

  gui.colorDepth = 8;
  gui.setup(&lcd);

/*
  gui.colorMask = 0xFF00FFU;
  gui.colorFrameInactive    = 0x999999U;
  gui.colorFrameActive      = 0x1F7FFFU;
  gui.colorCursorInactive   = 0xCCCCCCU;
  gui.colorCursorActive     = 0x7F7FFFU;
  gui.colorSubCursorInactive= 0xEEEEEEU;
  gui.colorSubCursorActive  = 0xBBBBFFU;
  gui.colorBorder           = 0x000000U;
  gui.colorFixed            = 0xBFBFBFU;
  gui.colorBack             = 0xFFFFFFU;
*/
/*/
  canvas.setColorDepth(4);
  canvas.createPalette();

  gui.colorMask = 0;
  gui.colorFrameInactive    = 1;   canvas.setPaletteColor(1, 0x999999U);
  gui.colorFrameActive      = 2;   canvas.setPaletteColor(2, 0x1F7FFFU);
  gui.colorCursorInactive   = 3;   canvas.setPaletteColor(3, 0x888888U);
  gui.colorCursorActive     = 4;   canvas.setPaletteColor(4, 0x005FFFU);
  gui.colorSubCursorInactive= 5;   canvas.setPaletteColor(5, 0x777777U);
  gui.colorSubCursorActive  = 6;   canvas.setPaletteColor(6, 0x6666AAU);
  gui.colorBorder = 12;   canvas.setPaletteColor(12, 0x000000U);
  gui.colorFixed  = 13;   canvas.setPaletteColor(13, 0x555555U);
  gui.colorBack   = 14;   canvas.setPaletteColor(14, 0x333333U);
  gui.smoothMove = 32;           // スムーズ移動係数 (0でスムーズ移動無効)

//*/

  // グリッドの設定
  grid.onInput        = onInput_grid;        // 入力処理関数の設定
  grid.onTouch        = onTouch_grid;        // タッチ処理関数の設定
  grid.onEnterCell    = onEnterCell_grid;    // セル選択処理関数の設定
  grid.onDrawCell     = onDrawCell_grid;     // セル描画関数の設定
  grid.onSelectCell   = onSelectCell_grid;   // コールバック関数の設定
  grid.onSelectedCell = onSelectedCell_grid; // コールバック関数の設定
  grid.frameWidth = 2;            // 外枠の幅
  grid.borderWidth = 1;           // グリッド間の枠の幅
  grid.simpleMode = false;        // シンプルモード解除(縦横移動可能になる)
  grid.rowCount = 30;             // 行数 (固定行数を含む)
  grid.columnCount = 27;          // 列数 (固定列数を含む)
  grid.fixedRowCount = 1;         // 固定行数
  grid.fixedColumnCount = 1;      // 固定列数
  grid.visibleVScroll = true;     // 縦スクロールバー表示フラグ
  grid.visibleHScroll = true;     // 横スクロールバー表示フラグ
  grid.scrollBarWidth = 5;        // スクロールバー表示幅
  grid.setDefaultRowHeight(16);   // 行の高さのデフォルト値
  grid.setDefaultColumnWidth(72); // 列の幅のデフォルト値
  grid.setColumnWidth(0, 24);     // 列の幅の個別設定 (先頭列の幅を24に
  grid.setDestRect(0, 0, lcd.width(), lcd.height());

  // スクリーンキーボードの設定
  auto osk = gui.getKeyboard();
  osk->onChangeState = onChangeState_osk;
  //osk->onShow = onShow_osk;
  osk->frameWidth = 0;
  osk->setFont(&fonts::Font0);
  auto osk_hight = osk->getDestRect().height();
  textbox.setDestRect(0, lcd.height()-osk_hight-16, lcd.width(), 16);
  textbox.setHideRect(0, lcd.height()-osk_hight-16, 0, 16);
  textbox.hide();

//osk.setup(0, lcd.height()-64, lcd.width(), 64);
//osk.input(lgui::input_none);

  gui.addControl(&textbox);
  gui.addControl(&grid);
}


void loop(void)
{
#if defined (ARDUINO)
  std::uint32_t msec = millis();
#else
  std::uint32_t msec = xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
  gui.loop();
/*
  std::int32_t x, y;
  if (lcd.getTouch(&x, &y)) {
    lcd.fillRect(x-2,y-2,5,5,0xFFFF);
  }
//*/
/*
  if (osk.isHiding())
  {
    grid.setFrameRect(lgui::Rectangle(10, 10, lcd.width()-11, lcd.height() - 11));
  }
//*/
#if defined (ARDUINO)
  msec = millis() - msec;
#else
  msec = xTaskGetTickCount() * portTICK_PERIOD_MS - msec;
#endif
  if (10 > msec)  delay(10 - msec);
}
#if !defined ( ARDUINO )
extern "C" {
  void loopTask(void*)
  {
    setup();
    for (;;) {
      loop();
    }
  }

  void app_main()
  {
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);
  }
}
#endif
