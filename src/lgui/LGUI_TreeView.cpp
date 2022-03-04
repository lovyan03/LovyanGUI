#include "LGUI_TreeView.hpp"
#include "../LovyanGUI.hpp"

namespace lgui
{
  bool LGUI_TreeItem::loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect)
  {
/*
    bool drawall = (_redraw & redraw_body) || gui->hasInvalidate(_visible_client_rect);
    if (!drawall) return true;

    auto gfx = gui->getGFX();

    auto clientrect = _visible_client_rect;

    auto canvas = gui->getCanvas(clientrect.width(), clientrect.height());
    canvas->clear(gui->color.back);
    canvas->drawString(title.c_str(), 2, 2);

    auto tv = reinterpret_cast<LGUI_TreeView*>(_parent);
    if (tv)
    {
      auto cr = tv->_cursor_rect_move;
      std::int32_t offset_x = tv->_visible_client_rect.left - tv->_hscroll_move;
      std::int32_t offset_y = tv->_visible_client_rect.top  - tv->_vscroll_move;
      cr.offset(offset_x, offset_y);
      canvas->fillRect(cr.left, cr.top, cr.width(), cr.height(), gui->color.activeCursor);
    }

    gui->pushCanvas(clientrect);
    canvas->pushSprite(gfx, clientrect.left, clientrect.top, gui->color.mask);
//*/
    return true;
  }

  bool LGUI_TreeView::input_impl(input_t& code)
  {
    auto ti = reinterpret_cast<LGUI_TreeItem*>(_focusControl);
    if (ti == nullptr)
    {
      if (_ctl_zorder.empty()) return false;
      ti = reinterpret_cast<LGUI_TreeItem*>(_ctl_zorder.front());
      if (ti == nullptr) return false;
    }

    if (code == input_fn1)
    {
      if (ti->_expanded) collapse(ti);
      else if (ti->_tree_parent != nullptr) ti->_tree_parent->setFocus();
    }
    else
    if (code == input_fn2)
    {
      if (!ti->_expanded) expand(ti);
      else
      {
        auto next = getNextControl(_focusControl, true, false, false);
        if (next != nullptr) next->setFocus();
      }
    }
    else
    if (code == input_t::input_ok || code == input_t::input_enter)
    {
      if (ti->_expanded) collapse(ti);
      else               expand(ti);
    }
    else
    if (code == input_t::input_tab)
    {
      auto next = getNextControl(_focusControl, true, false, false);
      if (next != nullptr) next->setFocus();
    }
    else
    if (code == input_t::input_back)
    {
      auto next = getNextControl(_focusControl, false, false, false);
      if (next != nullptr) next->setFocus();
    }

    return false; 
  }

  bool LGUI_TreeView::touch_impl(touch_t touchstate, LovyanUOM* uom)
  {
    if (touchstate & touch_t::mask_moving && !(touchstate & touch_t::mask_holding))
    {
      return LGUI_Scrollable::touch_impl(touchstate, uom);
    }
    else if ((touchstate & touch_t::mask_moving) || (touchstate == touch_t::touch_end))
    {
      std::int32_t x, y;
      uom->getTouchXY(&x, &y);
      auto ti = reinterpret_cast<LGUI_TreeItem*>(getControlAtPos(x, y));
      if ((touchstate & ~(touch_t::mask_holding | touch_t::mask_moving)) == touch_t::touch_end && _focusControl == ti)
      {
        if (ti->_expanded)
        {
          collapse(ti);
        }
        else
        {
          expand(ti);
        }
      }
      else
      {
        if (ti != nullptr) ti->setFocus();
      }
    }
    return true;
  }

  std::vector<LGUI_Base*>::iterator LGUI_TreeView::sortTree(std::vector<LGUI_Base*>::iterator it, LGUI_Base* target)
  {
    if (it == _ctl_zorder.end()) return it;
    std::int32_t left = 0;
    if (target != nullptr)
    {
      left = target->getDestRect().left + offsetItem;
    }
    do
    {
      std::vector<LGUI_Base*>::iterator it2 = it;
      while (!static_cast<LGUI_TreeItem*>(*it2)->isTreeParent(target))
      {
        if (++it2 == _ctl_zorder.end()) return it;
      } 

      auto tmp = *it2;
      if (it != it2)
      {
        _ctl_zorder.erase(it2);
        _ctl_zorder.insert(it, tmp);
      }
      auto r = tmp->getDestRect();
      r.offset(left - r.left, 0);
      tmp->setDestRect(r);
      it = sortTree(++it, tmp);
    } while (it != _ctl_zorder.end());

    return it;
  }

  void LGUI_TreeView::initTree(void)
  {
    LGUI_TreeView::sortTree(_ctl_zorder.begin());

    Rectangle hiderect;
    std::uint32_t collapse_left = ~0;
    std::int32_t bottom = 0;
    std::int32_t right = 0;
    for (auto ctl : _ctl_zorder)
    {
      auto ti = reinterpret_cast<LGUI_TreeItem*>(ctl);
      ti->_tree_firstchild = nullptr;
      if (ti->_tree_parent != nullptr && ti->_tree_parent->_tree_firstchild == nullptr)
      {
        ti->_tree_parent->_tree_firstchild = ti;
      }
      if (collapse_left < ti->_relative_dest_rect.left)
      {
        //hideitem
        ti->setHideRect(hiderect);
        // 画面外にあるアイテムは移動処理を省略し速やかにhideする
        if (ti->_relative_move_rect.bottom > ti->_parent->getClientRect().height())
        {
          ti->_relative_move_rect = hiderect;
        }

        //ti->_relative_hide_rect.offset(hideleft - _relative_hide_rect.left, top - _relative_hide_rect.top);
        ti->hide();
        ti->_expanded = false;
        continue;
      }

      if (ti->_expanded)
      {
        collapse_left = ~0;
      }
      else
      {
        collapse_left = ti->_relative_dest_rect.left;
        hiderect = ti->getDestRect();
        hiderect.bottom = hiderect.top;
      }
      if (ti->getState() != state_t::state_visible)
      {
        ti->show();
        if (ti->_tree_parent != nullptr)
        {
          ti->setHideRect(ti->_tree_parent->getDestRect());
        }
      }
      //if (!ctl->isVisible()) continue;
      auto r = ctl->getDestRect();
      r.offset(0, bottom - r.top);
      ctl->setDestRect(r);
      //auto tp = reinterpret_cast<LGUI_TreeItem*>(&ctl)->getTreeParent();
      //if (tp)
      //  ctl->setHideRect(tp->getDestRect());
      bottom = r.bottom + 1;
      if (right < r.right) right = r.right;
    }

    if (_vscroll_size != bottom)
    {
      _vscroll_size = bottom;
      _redraw = static_cast<redraw_t>(_redraw | redraw_t::redraw_vscroll);
/*
      std::int32_t tmp = bottom - getClientRect().height();
      if (tmp < 0) tmp = 0;
      if (_vscroll_dest > tmp) _vscroll_dest = tmp;
//*/
    }
    if (_hscroll_size != right)
    {
      _hscroll_size = right;
      _redraw = static_cast<redraw_t>(_redraw | redraw_t::redraw_hscroll);
/*

      std::int32_t tmp = right - getClientRect().width();
      if (tmp < 0) tmp = 0;
      if (_hscroll_dest > tmp) _hscroll_dest = tmp;
//*/
    }
    ensureVisible(_focusControl);
/*
    if (_focusControl == nullptr && !_ctl_zorder.empty())
    {
      auto item = _ctl_zorder.front();
    }
//*/
  }

  void LGUI_TreeView::expand(LGUI_TreeItem* target)
  {
    target->setRedraw(redraw_t::redraw_body);
    target->_expanded = true;
    if (onChangeExpand)
    {
      onChangeExpand(this, target, target->_expanded);
      if (!target->_expanded) return;
    }
    if (target->onChangeExpand)
    {
      target->onChangeExpand(target, target->_expanded);
      if (!target->_expanded) return;
    }

    LGUI_TreeItem* lastitem = nullptr;
    for (auto ctl : _ctl_zorder)
    {
      auto ti = reinterpret_cast<LGUI_TreeItem*>(ctl);
      if (ti->isTreeParent(target))
      {
        lastitem = ti;
        ti->show();
      }
    }
    initTree();
    if (lastitem != nullptr)
    {
      // 展開したアイテムが表示範囲に入るようにスクロール制御
      ensureVisible(lastitem);
      ensureVisible(_focusControl);
    }
  }

  void LGUI_TreeView::collapse(LGUI_TreeItem* target)
  {
    target->_expanded = false;
    if (onChangeExpand)
    {
      onChangeExpand(this, target, target->_expanded);
      if (target->_expanded) return;
    }
    if (target->onChangeExpand)
    {
      target->onChangeExpand(target, target->_expanded);
      if (target->_expanded) return;
    }

    target->setRedraw(redraw_t::redraw_body);
    for (auto ctl : _ctl_zorder)
    {
      auto ti = reinterpret_cast<LGUI_TreeItem*>(ctl);
      if (ti->getTreeParent() == target) { ti->hide(); }
    }
    initTree();
  }

  std::vector<LGUI_TreeItem*> LGUI_TreeView::getSubItems(LGUI_TreeItem* target, bool nest)
  {
    std::vector<LGUI_TreeItem*> res;
    auto it = std::find(_ctl_zorder.begin(), _ctl_zorder.end(), target);
    if (it == _ctl_zorder.end()) return res;
    std::int32_t left = (*it)->getDestRect().left;
    while (++it != _ctl_zorder.end() && left < (*it)->getDestRect().left)
    {
      auto ti = reinterpret_cast<LGUI_TreeItem*>(*it);
      if (nest || ti->isTreeParent(target))
      {
        res.emplace_back(reinterpret_cast<LGUI_TreeItem*>(*it));
      }
    }
    return res;
  }

  bool LGUI_TreeView::loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& clip)
  {
    static constexpr std::uint8_t icon_ext[2][7] = {{ 0b10000000,0b11100000,0b10011000,0b10000110,0b10011000,0b11100000,0b10000000 }
                                                   ,{ 0b11111110,0b01000100,0b01000100,0b00101000,0b00101000,0b00010000,0b00010000 }
                                                   };

    if (!LGUI_Scrollable::loop_impl(gui, has_focus, clip)) return false;
    auto cliprect = _visible_client_rect.intersect(clip);

    Rectangle cursor_rect = _cursor_rect_move;
    std::int32_t offset_x = _visible_client_rect.left - _hscroll_move;
    std::int32_t offset_y = _visible_client_rect.top - _vscroll_move;
    cursor_rect.offset(offset_x, offset_y);

    bool drawall = _redraw & redraw_body;

    if (_focusControl && _focusControl->isVisible())
    {
      auto &dest = _focusControl->getDestRect();
      if (_cursor_rect_move != dest)
      {
        gui->invalidate(cursor_rect.intersect(cliprect));
        _cursor_rect_move.moveTo(_focusControl->getTargetRect(), gui->smoothMove, drawall ? 32 : 16);
        cursor_rect = _cursor_rect_move;
        cursor_rect.offset(offset_x, offset_y);
        gui->invalidate(cursor_rect);
      }
    }

    auto gfx = gui->getGFX();
    auto ctrls = _ctl_zorder; // create copy
    for (auto& ctl : ctrls)
    {
      if (!ctl->isVisible()) continue;
      auto oldrect = ctl->getClientRect();
      bool redraw = drawall || ctl->hasRedraw(redraw_t::redraw_body);
      if (!ctl->loop(gui, has_focus && ctl == _focusControl, cliprect)) continue;

      auto rect = ctl->getClientRect();
      if (!rect.empty())
      {
        if (redraw || rect != oldrect || gui->hasInvalidate(rect))
        {
          gfx->setClipRect(cliprect.left, cliprect.top, cliprect.width(), cliprect.height());

          auto ti = reinterpret_cast<LGUI_TreeItem*>(ctl);
          auto canvas = gui->getCanvas(rect.width(), rect.height());
          canvas->clear(gui->color.border);
//canvas->clear(esp_random());
          auto r = cursor_rect;
          r.offset(-rect.left, -rect.top);
          canvas->setColor( gui->color.cursor[ has_focus ]);
          canvas->fillRect(r.left, r.top, r.width(), r.height());
          canvas->setFont(ti->getFont());
          canvas->setTextColor(0xFFFF);
          canvas->setTextDatum(lgfx::textdatum_t::middle_left);
          canvas->drawString(ti->title.c_str(), 16, canvas->height() >> 1);
          if (ti->canExpand || ti->_tree_firstchild != nullptr)
          {
            canvas->drawBitmap(4, (canvas->height() - 7) >> 1, icon_ext[ti->_expanded], 8, 7, 0xFFFF);
          }
          gui->pushCanvas(rect);
    //gfx->drawRect(rect.left, rect.top, rect.width(), rect.height(), 0xFFFF);
        }
      }

      gui->addMaskRect(ctl->getVisibleRect().intersect(cliprect));
    }

    gfx->clearClipRect();
    {
      for (auto& invrect : gui->getInvalidatedRects())
      {
        auto r = invrect.intersect(cliprect);
        if (r.empty()) continue;
        if (gui->checkFullMasked(r)) continue;
        auto canvas = gui->getCanvas(r.width(), r.height());
        canvas->clear(gui->color.back);
        gui->pushCanvas(r);
      }
    }

    return true;
  }
}
