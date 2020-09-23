#ifndef LGUI_TREEVIEW_HPP_
#define LGUI_TREEVIEW_HPP_

#include "LGUI_Container.hpp"
#include <string>
#include <vector>

namespace lgui
{
  class LGUI_TreeView;

  class LGUI_TreeItem : public LGUI_Base
  {
  friend LGUI_TreeView;
  public:
    std::string title;

    void (*onChangeExpand)(LGUI_TreeItem* sender, bool& expand) = nullptr;

    void setTreeParent(LGUI_TreeItem* parent) { _tree_parent = parent; }
    LGUI_TreeItem* getTreeParent(void) { return _tree_parent; }
    bool isTreeParent(LGUI_Base* parent) { return parent == _tree_parent; }
  private:
    bool _expanded = false;
    LGUI_TreeItem* _tree_parent = nullptr;
    LGUI_TreeItem* _tree_firstchild = nullptr;
    bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) override;
  };

  class LGUI_TreeView : public LGUI_Container
  {
  friend LGUI_TreeItem;
  public:

    /// 伸縮操作を行う直前に起きるイベント
    void (*onChangeExpand)(LGUI_TreeView* sender, LGUI_TreeItem* item, bool& expand) = nullptr;

    std::int32_t offsetItem = 16;
    void initTree(void);
    void expand(LGUI_TreeItem* target);
    void collapse(LGUI_TreeItem* target);

    std::vector<LGUI_TreeItem*> getSubItems(LGUI_TreeItem* target, bool nest = false);

  private:
    Rectangle _cursor_rect_move;
    bool loop_impl(LovyanGUI* gui, bool has_focus, const Rectangle& cliprect) override;
    bool input_impl(input_t&) override;
    bool touch_impl(touch_t touchstate, LovyanUOM* uom) override;

    std::vector<LGUI_Base*>::iterator sortTree(std::vector<LGUI_Base*>::iterator it, LGUI_Base* target = nullptr);
  };
}

#endif
