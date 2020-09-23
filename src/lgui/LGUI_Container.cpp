#include "LGUI_Container.hpp"
#include "LovyanGUI.hpp"
#include <LovyanGFX.hpp>

namespace lgui
{
  void LGUI_Container::addControl(LGUI_Base* ctl)
  {
    if (ctl == nullptr) return;
    removeControl(ctl);
    _ctl_zorder.push_back(ctl);
    ctl->_parent = this;
    auto rect = ctl->getDestRect();
    if (_vscroll_size < rect.bottom) _vscroll_size = rect.bottom;
    if (_hscroll_size < rect.right)  _hscroll_size = rect.right;
    setRedraw();
  }

  void LGUI_Container::removeControl(LGUI_Base* ctl)
  {
    if (ctl->isVisible()) { setRedraw(); }
    _ctl_zorder.erase(remove(_ctl_zorder.begin(), _ctl_zorder.end(), ctl), _ctl_zorder.end());
  }

  void LGUI_Container::setFocus_impl(void)
  {
    LGUI_Scrollable::setFocus_impl();
    if (_focusControl == nullptr)
    {
      setFocus(getNextControl(nullptr, true, false, false));
    }
  }

  void LGUI_Container::setFocus_impl(LGUI_Base* ctl)
  {
    if (!contain(ctl)) return;
    ensureVisible(ctl);
    if (_parent) _parent->setFocus(this);
    if (_focusControl != ctl)
    {
      auto old = _focusControl;
      _focusControl = ctl;
      if (old) old->setRedraw();
      if (ctl) ctl->setRedraw();
    }
  }

  bool LGUI_Container::contain(LGUI_Base* ctl)
  {
    return std::find(_ctl_zorder.begin(), _ctl_zorder.end(), ctl) != _ctl_zorder.end();
  }

  LGUI_Base* LGUI_Container::getNextControl(LGUI_Base* target, bool forward, bool nested, bool wrap)
  {
    if (target == nullptr) target = _focusControl;
    if (target == nullptr)
    {
      if (_ctl_zorder.empty()) return nullptr;
      target = _ctl_zorder.front();
      if (target->getState() & (state_t::state_appear | state_t::state_visible)) return target;
    }
    if (target->isContainer() && nested)
    {
      auto res = reinterpret_cast<LGUI_Container*>(target)->getNextControl(nullptr, forward, true, false);
      if (res != nullptr) return res;
    }
    auto it = std::find(_ctl_zorder.begin(), _ctl_zorder.end(), target);
    if (it == _ctl_zorder.end()) return nullptr;

    do
    {
      if (forward)
      {
        ++it;
        if (it == _ctl_zorder.end())
        {
          if (!wrap) return nullptr;
          it = _ctl_zorder.begin();
        }
      }
      else
      {
        if (it == _ctl_zorder.begin())
        {
          if (!wrap) return nullptr;
          it = _ctl_zorder.end();
        }
        --it;
      }
      if (*it == target) return target;
    } while (!((*it)->getState() & (state_t::state_appear | state_t::state_visible)));
    auto res = *it;
    if (nested)
    {
      while (res->isContainer())
      {
        auto r = res->getFocusControl();
        if (r == res) break;
        res = r;
      }
    }
    return res;
  }

  bool LGUI_Container::loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect)
  {
    bool res = LGUI_Scrollable::loop_impl(gui, has_focus, cliprect);
    auto crect = _visible_client_rect.intersect(cliprect);

    auto zo = _ctl_zorder; // create copy
    for (auto& ctl : zo)
    {
      if (!ctl->isVisible()) continue;
      bool r = ctl->loop(gui, has_focus && ctl == _focusControl, crect);
      if (r) {
        res = true;
        gui->addMaskRect(ctl->getVisibleRect().intersect(crect));
      }
    }
    crect = cliprect.intersect(_visible_client_rect);
    {
      for (auto& invrect : gui->_invalidated_rects)
      {
        auto r = invrect.intersect(crect);
        if (r.empty()) continue;
        if (gui->checkFullMasked(r)) continue;
        auto canvas = gui->getCanvas(r.width(), r.height());
        canvas->clear(gui->color.back);
//canvas->clear(esp_random());
//canvas->drawRect(0,0,canvas->width(), canvas->height(), 0xFFFF);
        gui->pushCanvas(r);
      }
    }
//*/
    return res;
  }

  LGUI_Base* LGUI_Container::getControlAtPos_impl(std::int32_t x, std::int32_t y)
  {
    if (!isVisible()) return nullptr;
    if (!_visible_rect.contains(x, y)) return nullptr;

    //x = _visible_client_rect.left;
    //y = _visible_client_rect.top;

    for (auto& ctl : _ctl_zorder)
    {
      if (!ctl->isVisible()) continue;
      auto res = ctl->getControlAtPos(x, y);
      if (res) return res;
    }
    return this;
  }

  void LGUI_Container::ensureVisible(LGUI_Base* ctl)
  {
    if (!contain(ctl)) return;

    auto r = ctl->getDestRect();
    if (_vscroll_size < r.bottom)
    {
      _vscroll_size = r.bottom;
      setRedraw(redraw_t::redraw_vscroll);
    }
    if (_hscroll_size < r.right)
    {
      _hscroll_size = r.right;
      setRedraw(redraw_t::redraw_hscroll);
    }
    r.inflate(16);

    {
      std::int32_t height = getDestRect().height();
      height -= (frameWidth << 1) + visibleHScroll * scrollBarWidth;
      if (r.bottom > _vscroll_size) r.bottom = _vscroll_size;
      std::int32_t tmp = r.bottom - height;
      if (tmp < 0) tmp = 0;
      if (_vscroll_dest < tmp) _vscroll_dest = tmp;
      tmp = _vscroll_size - height;
      if (tmp > r.top) tmp = r.top;
      if (tmp < 0) tmp = 0;
      if (_vscroll_dest > tmp) _vscroll_dest = tmp;
    }

    {
      std::int32_t width = getDestRect().width();
      width -= (frameWidth << 1) + visibleVScroll * scrollBarWidth;
      if (r.right > _hscroll_size) r.right = _hscroll_size;
      std::int32_t tmp = r.right - width;
      if (tmp < 0) tmp = 0;
      if (_hscroll_dest < tmp) _hscroll_dest = tmp;
      tmp = _hscroll_size - width;
      if (tmp > r.left) tmp = r.left;
      if (tmp < 0) tmp = 0;
      if (_hscroll_dest > tmp) _hscroll_dest = tmp;
    }
  }
}
