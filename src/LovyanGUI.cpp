#include "LovyanGUI.hpp"

#include <vector>
#include <algorithm>

namespace lgui
{
  static std::vector<LGUI_Base*> gui_zorder;

  void LovyanGUI::addControl(LGUI_Base* gui)
  {
    removeControl(gui);
    gui_zorder.insert(gui_zorder.begin(), gui);
  }

  void LovyanGUI::removeControl(LGUI_Base* gui)
  {
    gui_zorder.erase(remove(gui_zorder.begin(), gui_zorder.end(), gui), gui_zorder.end());
  }

  void LovyanGUI::setFocusControl(LGUI_Base* gui)
  {
    addControl(gui);
  }

  void LovyanGUI::input(input_t code)
  {
    if (gui_zorder.empty()) return;
    for (auto& gui : gui_zorder)
    {
      if (!gui->isVisible()) continue;
      gui->input(code);
      break;
    }
  }

  bool LovyanGUI::loop(void)
  {
    std::vector<const Rectangle*> rects(gui_zorder.size());
    rects.clear();
    bool res = false;
    for (auto& gui : gui_zorder)
    {
      bool r = gui->loop(&rects);
      res |= r;
      rects.push_back(&gui->getWindowRect());
    }
    return res;
  }
}

