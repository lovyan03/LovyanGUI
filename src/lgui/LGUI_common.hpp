#ifndef LGUI_COMMON_HPP_
#define LGUI_COMMON_HPP_

#define LGFX_USE_V1

#include <LovyanGFX.h>

#include <cstdint>
#include <algorithm>
#include <vector>

namespace lgui
{
  class LovyanGUI;

  struct point_t
  {
    std::int32_t x = 0;
    std::int32_t y = 0;
    point_t() = default;
    point_t(std::int32_t x, std::int32_t y)
    : x ( x )
    , y ( y )
    {}
  };

  struct Rectangle
  {
    union {
      point_t left_top = {0, 0};
      struct {
        std::int32_t left;
        std::int32_t top;
      };
    };
    union {
      point_t right_bottom = {0, 0};
      struct {
        std::int32_t right;
        std::int32_t bottom;
      };
    };

    Rectangle() = default;
    Rectangle(std::int32_t left, std::int32_t top, std::int32_t right, std::int32_t bottom)
     : left   ( left   )
     , top    ( top    )
     , right  ( right  )
     , bottom ( bottom )
     {}
    std::int32_t width(void) const { return right - left + 1; }
    std::int32_t height(void) const { return bottom - top + 1; }
    void setWidth(std::int32_t w) { right = left + w - 1; }
    void setHeight(std::int32_t h) { bottom = top + h - 1; }

    bool empty(void) const { return right < left || bottom < top; }

    bool equal(const Rectangle& rhs) const
    {
      return left   == rhs.left
          && top    == rhs.top
          && right  == rhs.right
          && bottom == rhs.bottom;
    }

    Rectangle intersect(const Rectangle& rhs) const
    {
      return Rectangle( std::max(rhs.left  , left  )
                      , std::max(rhs.top   , top   )
                      , std::min(rhs.right , right )
                      , std::min(rhs.bottom, bottom));
    }

    bool intersectsWith(const Rectangle& r) const {
       return (r.left <= right) && (left <= r.right) && (r.top <= bottom) && (top <= r.bottom);
    }

    template <typename T>
    bool intersectsWithArray(const T& rects) const {
      for (const auto& r : rects) if (intersectsWith(r)) return true;
      return false;
    }

    bool contains(std::int32_t x, std::int32_t y) const {
       return (left <= x) && (x <= right) && (top <= y) && (y <= bottom);
    }

    bool contains(const Rectangle& r) const {
       return (left <= r.left) && (r.right <= right) && (top <= r.top) && (r.bottom <= bottom);
    }

    void offset(std::int32_t x, std::int32_t y) { left += x; right += x; top += y; bottom += y; }

    void inflate(std::int32_t inc) { left -= inc; top -= inc; right += inc; bottom += inc; };

    void inflate(std::int32_t inc_x, std::int32_t inc_y) { left -= inc_x; top -= inc_y; right += inc_x; bottom += inc_y; };

    void moveTo(const Rectangle& dst, std::uint_fast8_t src_weight = 1, std::uint_fast8_t dst_weight = 1);

    static int32_t mixPos(std::int32_t src, std::int32_t dst, std::uint_fast8_t src_weight = 1, std::uint_fast8_t dst_weight = 1)
    {
      if (src_weight == 0 || dst_weight == 0) return dst;
      src += ((dst - src) * (std::int32_t)dst_weight / ((std::int32_t)src_weight + (std::int32_t)dst_weight));
      if (src < dst) return src + 1;
      if (src > dst) return src - 1;
      return src;
    }

    void addRectangle(std::vector<Rectangle> &rects) const
    {
      if (empty()) return;
      for (auto& r : rects)
      {
        if (r.contains(*this))
        {
          return;
        }
        if (contains(r))
        {
          r = *this;
          return;
        }
/*
        if (r.left == left && r.right == right)
        {
          if (r.top == bottom + 1)
          {
            r.top = top;
            return;
          }
          if (top == r.bottom + 1)
          {
            r.bottom = bottom;
            return;
          }
        }
        if (r.top == top && r.bottom == bottom)
        {
          if (r.left == right + 1)
          {
            r.left = left;
            return;
          }
          if (left == r.right + 1)
          {
            r.right = right;
            return;
          }
        }
//*/
      }
      rects.emplace_back(*this);
    }

    void removeRectangle(std::vector<Rectangle> &rects) const
    {
/*
      if (empty()) return;
      for (auto it = rects.begin(); it != rects.end(); ++it)
      {
        if (intersectsWith(*it))
        {
          it->right = it->left;
          it->bottom = it->top;
        }
      }
//*/
    }
  };
  inline bool operator==(const Rectangle& lhs, const Rectangle& rhs) { return lhs.equal(rhs); }
  inline bool operator!=(const Rectangle& lhs, const Rectangle& rhs) { return !lhs.equal(rhs); }


  struct PositionStocker
  {
    std::int32_t defaultSize = 0;

    std::int32_t getPosition(std::uint32_t index);
    std::uint32_t getIndex(std::int32_t position);
    void setSize(std::int32_t size, std::uint32_t index, std::uint32_t count = 1);
    std::int32_t getSize(std::uint32_t index);

    bool empty(void) const { return _positions.empty(); }
    void clear(void) { _positions.clear(); }

  private:
    std::int32_t lastpos(void);
    std::vector<std::int32_t> _positions;
    void expand(std::uint32_t count);
  };
}

#endif
