#ifndef LGUI_LABEL_HPP_
#define LGUI_LABEL_HPP_

#include "LGUI_Base.hpp"
#include <string>

namespace lgui
{
  class LGUI_Label : public LGUI_Base
  {
  public:
    void setText(const char* text);
    const char* getText(void) const;
  protected:
    bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) override;
  private:
    std::string _data;
  };
};

#endif
