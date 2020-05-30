#ifndef _LOVYANGUI_HPP_
#define _LOVYANGUI_HPP_

#include "LGUI_OnScreenKeyboard.h"

namespace lgui
{
  class LovyanGUI
  {
  public:
    void addControl(LGUI_Base*);
    void removeControl(LGUI_Base*);
    void setFocusControl(LGUI_Base*);
    void input(input_t);
    bool loop(void);
    void setup(LovyanGFX*);

    LovyanGUI(void) {}
    virtual ~LovyanGUI(void) {}
  protected:
    LovyanGFX* _gfx;
  };
}

#endif
