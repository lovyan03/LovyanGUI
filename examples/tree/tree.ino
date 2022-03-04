#include <WiFi.h>

#include <LovyanGUI.hpp>

#include <cstdio>
#include <string>
#include <vector>

static LGFX lcd;                        // GFXライブラリのインスタンス
static lgui::LovyanGUI gui;             // GUIライブラリのインスタンス
static lgui::LGUI_TextBox textbox;      // テキストボックス
static lgui::LGUI_Container panel1;     // コンテナ
static lgui::LGUI_TextBox textbox1;     // テキストボックス
static lgui::LGUI_TextBox textbox2;     // テキストボックス
static lgui::LGUI_TextBox textbox3;     // テキストボックス
static lgui::LGUI_Label label1;         // ラベル
static lgui::LGUI_Label label2;         // ラベル
static lgui::LGUI_Label label3;         // ラベル
static lgui::LGUI_TreeView tv;          // ツリービュー
static lgui::LGUI_TreeItem tiWifi;      // ツリービュー(WiFi)
static std::vector<std::string> gridstrs(26*30); // グリッドの文字列保持用変数

static void onChangeExpand_wifi(lgui::LGUI_TreeItem* sender, bool &expand)
{
  if (expand)
  {
    for (int i = 0; i < 10; ++i)
    {
      auto ti = new lgui::LGUI_TreeItem();
      ti->title = "test";
      ti->setDestRect(0,0,160,32);
      tv.addControl(ti);
    }
  }
}

static void onChangeState_wifi(lgui::LGUI_Base* sender, lgui::state_t state)
{
  if (state == lgui::state_t::state_invisible)
  {
    auto items = tv.getSubItems(&tiWifi);
    for (auto ctl : items)
    {
      tv.removeControl(ctl);
      delete ctl;
    }
    tv.initTree();
  }
}

// スクリーンキーボードが表示される時のコールバック関数
static void onChangeState_osk(lgui::LGUI_Base* sender, lgui::state_t state)
{
  if (state != lgui::state_t::state_visible) return;
  auto osk = gui.getKeyboard();
  auto osk_hight = osk->getDestRect().height();
  textbox.setDestRect(0, lcd.height()-osk_hight-16, lcd.width(), 16);
  textbox.setHideRect(0, lcd.height()-osk_hight-16, 0, 16);
  textbox.show();

  auto r = tv.getDestRect();
  r.bottom = textbox.getDestRect().top - 1;
  tv.setDestRect(r);
}

typedef std::vector<lgui::LGUI_TreeItem*> items;

static lgui::LGUI_TreeItem* treeitem(const char* title)
{
  auto res = new lgui::LGUI_TreeItem();
  res->title = title;
//  res->setDestRect(0,0,(esp_random() & 63) + 80,(esp_random() & 15)+20);
  res->setDestRect(0, 0, 160, 32);
  return res;
}

static lgui::LGUI_TreeItem* treeitem(const char* title, items subitems)
{
  auto res = treeitem(title);
  for (auto& ctl : subitems)
  {
    ctl->setTreeParent(res);
  }
  tv.addControl(subitems.begin(), subitems.end());
  return res;
}
//*/
void setup(void)
{
  lcd.begin();
  lcd.startWrite();
  lcd.setBrightness(128);
  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

  gui.setFont(&fonts::Font2);

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

//*/

  // グリッドの設定
  //tv.onInput        = onInput_grid;        // 入力処理関数の設定
  //tv.onTouch        = onTouch_grid;        // タッチ処理関数の設定
  tv.frameWidth = 2;            // 外枠の幅
  //tv.borderWidth = 1;           // グリッド間の枠の幅
  //tv.simpleMode = false;        // シンプルモード解除(縦横移動可能になる)
  //gui.smoothMove = 32;           // スムーズ移動係数 (0でスムーズ移動無効)
  tv.visibleVScroll = true;     // 縦スクロールバー表示フラグ
  tv.visibleHScroll = true;     // 横スクロールバー表示フラグ
  tv.scrollBarWidth = 5;        // スクロールバー表示幅
  //tv.setDefaultRowHeight(16);   // 行の高さのデフォルト値
  //tv.setDefaultColumnWidth(72); // 列の幅のデフォルト値
  //tv.setColumnWidth(0, 24);     // 列の幅の個別設定 (先頭列の幅を24に
  std::int32_t treewidth = lcd.width()*2/3;
  tv.setDestRect(0, 0, treewidth, lcd.height());
  tiWifi.onChangeExpand = onChangeExpand_wifi;
  tiWifi.onChangeState = onChangeState_wifi;

  // スクリーンキーボードの設定
  auto osk = gui.getKeyboard();
  //osk->onHiding = onHiding_osk;
  osk->onChangeState = onChangeState_osk;
  osk->frameWidth = 0;
  osk->setFont(&fonts::Font0);
  osk->setTarget(&textbox);
  auto osk_hight = osk->getDestRect().height();
  textbox.setDestRect(0, lcd.height()-osk_hight-16, lcd.width(), 16);
  textbox.setHideRect(0, lcd.height()-osk_hight-16, 0, 16);
  textbox.hide();

//osk.setup(0, lcd.height()-64, lcd.width(), 64);
//osk.input(lgui::input_none);

  gui.addControl(&tv);
  gui.addControl(&panel1);
  gui.addControl(&textbox);

  panel1.setDestRect(treewidth, 0, lcd.width() - treewidth, lcd.height());

  panel1.addControl(&label1);
  panel1.addControl(&textbox1);
  panel1.addControl(&label2);
  panel1.addControl(&textbox2);
  panel1.addControl(&label3);
  panel1.addControl(&textbox3);
//*/

  label1.setDestRect(10,10,100,16);
  label2.setDestRect(20,50,100,16);
  label3.setDestRect(30,90,100,16);
  textbox1.setDestRect(40, 30,100,16);
  textbox2.setDestRect(50, 70,100,16);
  textbox3.setDestRect(60,110,100,16);

  tiWifi.title = "WiFi client";
  tiWifi.setDestRect(0,0,160,32);
  tv.addControl(&tiWifi);

  tv.addControl(treeitem("test1", items
                 { treeitem("foo")
                 , treeitem("bar")
                 }
               ));

  tv.addControl(treeitem("test2", items
                 { treeitem("hogehoge")
                 , treeitem("fugafuga")
                 }
               ));

  tv.addControl(treeitem("test3", items
                 { treeitem("cat", items
                   { treeitem("mike")
                   , treeitem("shiro")
                   , treeitem("tora")
                   }
                 )
                 , treeitem("dog")
                 , treeitem("mouse")
                 }
               ));

  tv.addControl( treeitem("test4", items
                 { treeitem("012", items
                   { treeitem("123", items
                     { treeitem("234", items
                       { treeitem("345", items
                         { treeitem("456", items
                           { treeitem("567", items
                             { treeitem("678", items
                               { treeitem("789", items
                                 { treeitem("89A", items
                                   { treeitem("9AB")
                                   })
                                 })
                               })
                             })
                           })
                         })
                       })
                     })
                   })
                 })
               );
//*
  static auto ti5 = treeitem("test5");
  tv.addControl(ti5);
  for (int i = 0; i < 100; ++i) {
    char title[] = "title___\0\0";
    itoa(i, &title[5], 10);
    auto item = treeitem(title, items 
      { treeitem("5dummy1")
      , treeitem("5dummy2")
      , treeitem("5dummy3")
      } );
    item->setTreeParent(ti5);
    tv.addControl(item);
  }
//*/
  tv.addControl( treeitem("test6", items
                 { treeitem("012", items
                   { treeitem("123", items
                     { treeitem("234", items
                       { treeitem("345", items
                         { treeitem("456", items
                           { treeitem("567", items
                             { treeitem("678", items
                               { treeitem("789", items
                                 { treeitem("89A", items
                                   { treeitem("9AB")
                                   })
                                 })
                               })
                             })
                           })
                         })
                       })
                     })
                   })
                 })
               );
  tv.initTree();
}

void loop(void)
{
  #if defined (ARDUINO)
  std::uint32_t msec = millis();
#else
  std::uint32_t msec = xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
  gui.loop();
//*
  auto freemem = esp_get_free_heap_size();
  //lcd.setCursor(0,0);
  //lcd.printf("Free%7d Byte", freemem);
  char strtmp[50];
  sprintf(strtmp, "%d", freemem);
  textbox1.setText(strtmp);
  textbox1.setRedraw();
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
