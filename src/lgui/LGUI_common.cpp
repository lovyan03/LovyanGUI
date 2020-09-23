#include "lgui_common.hpp"

namespace lgui
{
  void Rectangle::moveTo(const Rectangle& dst, std::uint_fast8_t src_weight, std::uint_fast8_t dst_weight)
  {
    if (dst_weight < 1) dst_weight = 1;
    if (left   != dst.left  ) { left   = mixPos( left  , dst.left  , src_weight, dst_weight); }
    if (top    != dst.top   ) { top    = mixPos( top   , dst.top   , src_weight, dst_weight); }
    if (right  != dst.right ) { right  = mixPos( right , dst.right , src_weight, dst_weight); }
    if (bottom != dst.bottom) { bottom = mixPos( bottom, dst.bottom, src_weight, dst_weight); }
  }


  void PositionStocker::setSize(std::int32_t size, std::uint32_t index, std::uint32_t count)
  {
    if (_positions.size() <= index + count) expand(index + count);

    std::int32_t diff = 0;
    std::int32_t range = index + count;
    std::int32_t prev = (0 >= index) ? 0 : _positions[index - 1];
    for (; index < range; ++index)
    {
      if (index < range)
      { // 直前のセル座標との差分に基づいて現在サイズを得る
        auto prevSize = _positions[index] - prev;

        // 設定するサイズと現在のサイズの差分を以後の全セルの差分値として加算する
        diff += (size - prevSize);
      }
      prev = _positions[index];
      _positions[index] += diff;
    }
    for (; index < _positions.size(); ++index)
    {
      _positions[index] += diff;
    }
  }

  std::int32_t PositionStocker::getSize(std::uint32_t index)
  {
    if (index < _positions.size())
    {
      std::int32_t prev = (0 >= index) ? 0 : _positions[index - 1];
      return _positions[index] - prev;
    }
    return defaultSize;
  }

  std::int32_t PositionStocker::getPosition(std::uint32_t index)
  {
    return (index < _positions.size())
         ? _positions[index]
         : lastpos() + (1 + index - _positions.size()) * defaultSize;
  }

  std::uint32_t PositionStocker::getIndex(std::int32_t position)
  {
    auto l = lastpos();
    if (position < l)
    {
      auto it = std::lower_bound(_positions.begin(), _positions.end(), position);
      return std::distance(_positions.begin(), it);
    }
    return _positions.size() + ((position - l) / defaultSize);
  }

  void PositionStocker::expand(std::uint32_t count)
  {
    auto l = lastpos();
    while (_positions.size() < count)
    {
      l += defaultSize;
      _positions.push_back(l);
    }
  }

  std::int32_t PositionStocker::lastpos(void)
  {
    return _positions.empty() ? 0 : _positions.back();
  }

}

