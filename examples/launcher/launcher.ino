#define LGFX_AUTODETECT

#include <LovyanGFX.hpp>
#include <LovyanGUI.hpp>
#include <Update.h>

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <SD.h>
#include <SPIFFS.h>

#include <cstdio>
#include <string>
#include <vector>

static LGFX lcd;                        // GFXライブラリのインスタンス
static lgui::LovyanGUI gui;             // GUIライブラリのインスタンス
static lgui::LGUI_TextBox textbox;      // テキストボックス
static lgui::LGUI_Label label1;         // ラベル
static lgui::LGUI_TreeView tv;          // ツリービュー
static lgui::LGUI_TreeItem* tiSD;       // ツリーアイテム(SD)
static lgui::LGUI_TreeItem* tiWifi;     // ツリーアイテム(WiFi Client)
static std::vector<std::string> gridstrs(26*30); // グリッドの文字列保持用変数

static constexpr char tiWifiTitle[] = "WiFi client";

static volatile int wifi_ap_count = 0;
static String wifi_ssid;

static void wifi_scan_task(void*)
{
  wifi_ap_count = -1;
  if (WiFi.status() != WL_CONNECTED) WiFi.disconnect();
  wifi_ap_count = WiFi.scanNetworks();
  vTaskDelete(NULL);
}

typedef std::vector<lgui::LGUI_TreeItem*> items;

static lgui::LGUI_TreeItem* treeitem(const char* title)
{
  auto res = new lgui::LGUI_TreeItem();
  res->title = title;
  res->setDestRect(0, 0, 160, 32);
  tv.addControl(res);
  return res;
}

static lgui::LGUI_TreeItem* treeitem(const char* title, items subitems)
{
  auto res = treeitem(title);
  for (auto& ctl : subitems)
  {
    ctl->setTreeParent(res);
  }
  return res;
}

static void SDProgress( int state, int size )
{
  int percent = ( state * 100 ) / size;
  lcd.fillRect((lcd.width()-100)>>1, (lcd.height()-20)>>1, percent, 20, 0x00FF00U);
}

static void onChangeExpand_FSitem(lgui::LGUI_TreeItem* sender, bool& expand)
{
  expand = false;

  lcd.drawRect((lcd.width()-102)>>1, (lcd.height()-22)>>1, 102, 22, 0x00FF00U);
  lcd.fillRect((lcd.width()-100)>>1, (lcd.height()-20)>>1, 100, 20, 0x000000U);

  lcd.endWrite();

  File file = SD.open(sender->title.c_str());
  auto updateSize = file.size();
  Update.onProgress( SDProgress );
  if (Update.begin( updateSize )) {
    size_t written = Update.writeStream( file );
    if ( written == updateSize ) {
      Serial.println( "Written : " + String(written) + " successfully" );
    } else {
      Serial.println( "Written only : " + String(written) + "/" + String(updateSize) + ". Retry?" );
    }

    if ( Update.end() ) {
      Serial.println( "OTA done!" );
      if ( Update.isFinished() ) {
        Serial.println( "Update successfully completed. Rebooting." );
        ESP.restart();
      } else {
        Serial.println( "Update not finished? Something went wrong!" );
      }
    } else {
      Serial.println( "Error Occurred. Error #: " + String( Update.getError() ) );
    }
  }

  lcd.startWrite();
}

static void onChangeExpand_FS(lgui::LGUI_TreeItem* sender, bool& expand)
{
  if (expand)
  {
    auto items = tv.getSubItems(tiSD, true);
    for (auto ctl : items)
    {
      tv.removeControl(ctl);
      delete ctl;
    }
    lcd.endWrite();

    SD.end();
    SD.begin(4, SPI, 20000000);

    lgui::LGUI_TreeItem* ti;
    String path = "";

    File root = SD.open(path.length() ? path : "/");
    File file = root.openNextFile();
    String ptmp;
    String fn;
    String ext;
    while (file) {
      ptmp = file.name();
      //fn = ptmp.substring(path.length() + 1);
      fn = ptmp;
      if (!file.isDirectory()) {
        int idx = fn.lastIndexOf('.');
        ext = fn.substring(idx + 1);
        ext.toLowerCase();
        //fn = fn.substring(0, idx);
        if (ext == "bin" && !fn.startsWith(".") && fn != "menu" && file.size() > 100) {
          ti = treeitem(fn.c_str());
          ti->setTreeParent(sender);
          ti->onChangeExpand = onChangeExpand_FSitem;
        }
      }
      file = root.openNextFile();
    }
    root.close();

    lcd.startWrite();
  }  
}

static void onChangeExpand_wifiitem(lgui::LGUI_TreeItem* sender, bool& expand)
{
  expand = false;
  wifi_ssid = sender->title.c_str();

  textbox.show();
  textbox.showKeyboard();
}

static void onChangeExpand_wifi(lgui::LGUI_TreeItem* sender, bool& expand)
{
  if (expand)
  {
    auto items = tv.getSubItems(tiWifi, false);
    for (auto ctl : items)
    {
      tv.removeControl(ctl);
      delete ctl;
    }

    if (wifi_ap_count == 0)
    {
      xTaskCreatePinnedToCore(wifi_scan_task, "wifi_scan_task", 2048, NULL, 1, NULL, 0);
      tiWifi->title = "WiFi scanning...";
    }

    for (int i = 0; i < wifi_ap_count; ++i)
    {
      //auto atmp = WiFi.encryptionType(i);
      auto ti = treeitem(WiFi.SSID(i).c_str());
      ti->setTreeParent(sender);
      ti->onChangeExpand = onChangeExpand_wifiitem;
    }
  }
  else
  {
    WiFi.scanDelete();
  }  
}

// スクリーンキーボードの表示状態が変更される時のコールバック関数
static void onChangeState_osk(lgui::LGUI_Base* sender, lgui::state_t state)
{
  if (state == lgui::state_t::state_disappear)
  {
    textbox.hide();
    auto r = tv.getDestRect();
    r.bottom = lcd.height() - 1;
    tv.setDestRect(r);
    gui.setFocus(&tv);
    WiFi.begin(wifi_ssid.c_str(), textbox.getText());
    return;
  }

  if (state != lgui::state_t::state_visible) return;
  auto osk = gui.getKeyboard();
  auto osk_hight = osk->getDestRect().height();
  textbox.setDestRect(0, lcd.height()-osk_hight-16, lcd.width(), 16);
  textbox.setHideRect(0, lcd.height()-osk_hight-16, 0, 16);
  textbox.show();

  auto r = tv.getDestRect();
  r.bottom = textbox.getDestRect().top - 1;
  tv.setDestRect(r);

  tv.ensureVisible(tv.getFocusControl());
}
//*/
void setup2(void)
{
  lcd.begin();
  //SD.begin(4, SPI, 20000000);
  //SD.end();
  //SD.begin(4, SPI, 20000000);
}

//*/
void setup(void)
{
  //SD.begin();
  //SD.end();
  //SPI.end();
  Serial.begin(115200);

  lcd.begin();
  if (lcd.getBoard() == lgfx::board_t::board_M5Stack
   || lcd.getBoard() == lgfx::board_t::board_M5StackCore2)
  {
    tiSD = treeitem("SD-Updater");
    tiSD->onChangeExpand = onChangeExpand_FS;
    SD.end();
    SD.begin(4, SPI, 20000000);
  }
//*/
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
  //tv.visibleHScroll = true;     // 横スクロールバー表示フラグ
  //tv.scrollBarWidth = 5;        // スクロールバー表示幅
  //tv.setDefaultRowHeight(16);   // 行の高さのデフォルト値
  //tv.setDefaultColumnWidth(72); // 列の幅のデフォルト値
  //tv.setColumnWidth(0, 24);     // 列の幅の個別設定 (先頭列の幅を24に
  //std::int32_t treewidth = lcd.width()*2/3;
  std::int32_t treewidth = lcd.width();
  tv.setDestRect(0, 16, treewidth, lcd.height()-16);

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

  gui.addControl(&textbox);
  gui.addControl(&tv);
  gui.addControl(&label1);

//  panel1.setDestRect(treewidth, 16, lcd.width() - treewidth, lcd.height() - 16);

  label1.setDestRect(0, 0, lcd.width(), 16);

  {
    tiWifi = treeitem(tiWifiTitle);
    tiWifi->onChangeExpand = onChangeExpand_wifi;
    //auto ti = treeitem("WiFi");
    //tiWifi->setTreeParent(ti);
  }

  treeitem("Tools", items
  { treeitem("dummy")
  , treeitem("no implemented")
  });

  tv.initTree();

  disableCore0WDT();
  lcd.startWrite();
}

void loop(void)
{
//ESP_LOGI("main","loop");

  #if defined (ARDUINO)
  std::uint32_t msec = millis();
#else
  std::uint32_t msec = xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
  gui.loop();

  if (wifi_ap_count > 0)
  {
    tiWifi->title = tiWifiTitle;
    tv.expand(tiWifi);
    wifi_ap_count = 0;
  }

  wifi_mode_t mode;
  if (ESP_OK == esp_wifi_get_mode(&mode))
  {
    std::string str;
    if (mode == WIFI_AP || mode == WIFI_AP_STA)
    {
      str += "AP:";
      str += WiFi.softAPIP().toString().c_str();
    }
    if (mode == WIFI_STA || mode == WIFI_AP_STA)
    {
      if (!str.empty()) str += " / ";
      str += "STA:";
      str += WiFi.localIP().toString().c_str();
    }
    label1.setText(str.c_str());
  }
  label1.setRedraw();
/*
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
    vTaskDelete(NULL);
  }

  void app_main()
  {
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);
  }
}
#endif
