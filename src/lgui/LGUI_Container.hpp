#ifndef LGUI_CONTAINER_HPP_
#define LGUI_CONTAINER_HPP_

#include "LGUI_Scrollable.hpp"

namespace lgui
{
  class LGUI_Container : public LGUI_Scrollable
  {
  public:
    void addControl(LGUI_Base*);

    template <typename T>
    void addControl(T it, T it_end)
    {
      for (; it != it_end; ++it) addControl(*it);
    }

    void removeControl(LGUI_Base*);

    template <typename T>
    void removeControl(T it, T it_end)
    {
      for (; it != it_end; ++it) removeControl(*it);
    }

    bool contain(LGUI_Base* ctl);

    void setFocus(LGUI_Base* ctl) { setFocus_impl(ctl); }

    LGUI_Base* getNextControl(LGUI_Base* ctl, bool forward, bool nested, bool wrap);
/**
 * @fn
 * 対象のコントロールが画面内に表示されるようスクロールする
 * @param (ctl) 対象のコントロール
 */
    void ensureVisible(LGUI_Base* ctl);

  protected:
    LGUI_Base* _focusControl = nullptr;
    std::vector<LGUI_Base*> _ctl_zorder;

    bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) override;

    virtual void setFocus_impl(LGUI_Base* ctl);

    LGUI_Base* getFocusControl_impl(void) override { return _focusControl ? _focusControl->getFocusControl() : this; }
    LGUI_Base* getControlAtPos_impl(std::int32_t x, std::int32_t y) override;

    bool isContainer(void) override { return true; }

    void setFocus_impl(void) override;
  };
}

#endif
