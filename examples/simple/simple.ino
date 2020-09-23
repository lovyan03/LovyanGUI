
#define LGFX_AUTODETECT

#include <LovyanGFX.hpp>
#include <LovyanGUI.hpp>

#include <cmath>
#include <string>

static LGFX lcd;
static lgui::LGUI_GridView selector;
static lgui::LGUI_GridView menu;
static lgui::LovyanGUI gui;
static float fmillis;
static int selectedcell;

static std::string gridstrs[6] = {"circle","arc","triangle","rectangle","roundrect","ellipse"};
static bool fillmode[6];


static void icon(LovyanGFX* gfx, int32_t idx, bool mode)
{
  int px = gfx->width()>>1;
  int py = (gfx->height()>>1) - 3;
  switch (idx)
  {
  case 0:
    {
      auto sinf = sin(fmillis) * 10;
      if (mode) gfx->fillCircle(px, py, (py>>2) + sinf);
      else      gfx->drawCircle(px, py, (py>>2) + sinf);
    }
    break;
  case 1:
    {
      auto f = fmillis * 20;
      if (mode) gfx->fillArc(px, py, py>>1, py>>2, f, f+300);
      else      gfx->drawArc(px, py, py>>1, py>>2, f, f+300);
    }
    break;
  case 2:
    if (mode) {
      gfx->fillTriangle( px + cos(fmillis    )*20, py + sin(fmillis    )*20
                       , px + cos(fmillis+2.1)*20, py + sin(fmillis+2.1)*20
                       , px + cos(fmillis+4.2)*20, py + sin(fmillis+4.2)*20
                       );
    } else {
      gfx->drawTriangle( px + cos(fmillis    )*20, py + sin(fmillis    )*20
                       , px + cos(fmillis+2.1)*20, py + sin(fmillis+2.1)*20
                       , px + cos(fmillis+4.2)*20, py + sin(fmillis+4.2)*20
                       );
    }
    break;
  case 3:
    {
      auto sinf = sin(fmillis) * 20;
      auto cosf = cos(fmillis) * 20;
      if (mode)     gfx->fillRect(px - sinf, py - cosf, sinf*2, cosf*2);
      else          gfx->drawRect(px - sinf, py - cosf, sinf*2, cosf*2);
    }
    break;
  case 4:
    {
      auto sinf = sin(fmillis) * 5;
      auto cosf = cos(fmillis) * 5;
      if (mode)     gfx->fillRoundRect(px - sinf-15, py - cosf-15, sinf*2+30, cosf*2+30, 7);
      else          gfx->drawRoundRect(px - sinf-15, py - cosf-15, sinf*2+30, cosf*2+30, 7);
    }
    break;
  case 5:
    {
      auto sinf = sin(fmillis) * 20;
      auto cosf = cos(fmillis) * 20;
      if (mode)     gfx->fillEllipse(px, py, fabs(sinf), fabs(cosf));
      else          gfx->drawEllipse(px, py, fabs(sinf), fabs(cosf));
    }
    break;
  }
}


static void onEnterCell_UI(lgui::LGUI_GridView* sender, std::int32_t column, std::int32_t row)
{
  selector.hide();
}

static void onSelectCell_UI(lgui::LGUI_GridView* sender, std::int32_t& column, std::int32_t& row, bool& canselect)
{
  fillmode[selectedcell] = column;
}

static void onDrawCell_UI(lgui::LGUI_GridView* sender, LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool& redraw)
{
  redraw = true;

  gfx->setTextDatum(textdatum_t::middle_center);
  gfx->drawString(column ? "fill" : "draw", gfx->width()>>1, gfx->height() - gfx->fontHeight() - 2);
  icon(gfx, selectedcell, column);
}

static void onDrawCell_menu(lgui::LGUI_GridView* sender, LovyanGFX* gfx, std::int32_t column, std::int32_t row, bool& redraw)
{
  redraw = true;

  int index = row * 3 + column;

  gfx->setTextDatum(textdatum_t::bottom_center);
  gfx->drawString(gridstrs[index].c_str(), gfx->width()>>1, gfx->height() - 2);
  icon(gfx, index, fillmode[index]);
}

static void onEnterCell_menu(lgui::LGUI_GridView* sender, std::int32_t column, std::int32_t row)
{
  // メニューのカーソル座標の矩形を取得
  auto rect = menu.getCursorRectDest();
  // 座標を矩形中心点に変更
  rect.left = rect.right = (rect.left + rect.right) >> 1;
  rect.top = rect.bottom = (rect.top + rect.bottom) >> 1;
  // セレクタの非表示座標を先ほど作成した矩形座標に変更する
  selector.setFrameRectHide(rect);

  selectedcell = column + row*3;
  selector.selectCell(fillmode[selectedcell], 0);
  selector.show();
  gui.addControl(&selector);
}


void setup(void)
{
  lcd.begin();
  lcd.startWrite();

  if (lcd.width() < lcd.height())
    lcd.setRotation(lcd.getRotation()^1);

  gui.setup(&lcd);
  gui.addControl(&menu);
  gui.addControl(&selector);

  menu.frameWidth = 3;   // メニューの外枠の太さ
  menu.rowCount = 2;     // メニューの縦マス数
  menu.columnCount = 3;  // メニューの横マス数
  menu.onEnterCell = onEnterCell_menu; // メニュー選択時のコールバック関数
  menu.onDrawCell = onDrawCell_menu;   // メニューのセル描画時のコールバック関数
  menu.setup(0, 0, lcd.width(), lcd.height());

  selector.frameWidth = 3;  // セレクタの外枠の太さ
  selector.rowCount = 1;    // セレクタの縦マス数
  selector.columnCount = 2; // セレクタの横マス数
  selector.onEnterCell = onEnterCell_UI;   // セレクタ選択時のコールバック関数
  selector.onDrawCell  = onDrawCell_UI;    // セレクタのセル描画時のコールバック関数
  selector.onSelectCell= onSelectCell_UI;  // セレクタのセル選択時のコールバック関数
  if (lcd.width() < 200)
  {
    selector.setup((lcd.width() - 120)>>1, (lcd.height() - 60)>>1, 120, 60 );
  }
  else
  {
    selector.setup((lcd.width() - 200)>>1, (lcd.height() - 100)>>1, 200, 100 );
  }
  selector.hide();     // セレクタは最初は非表示にしておく
}


void loop(void)
{
  std::uint32_t msec = xTaskGetTickCount() * portTICK_PERIOD_MS;
  fmillis = (float)msec / 128;
//menu.invalidate();   // メニューのフレームレートを上げたい場合はこれをコメントアウトする
  gui.loop();
  msec = xTaskGetTickCount() * portTICK_PERIOD_MS - msec;
  if (10 > msec)  delay(10 - msec);
}

#if !defined ( ARDUINO )
extern "C" {
  void app_main()
  {
    setup();
    for (;;) {
      loop();
    }
  }
}
#endif
