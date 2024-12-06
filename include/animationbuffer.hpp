#pragma once

#include "animation.hpp"
#include <new>

class AnimationBuffer {
public:
  Animation* get() {
    if (!_created) return nullptr;
    return reinterpret_cast<Animation*>(_animationData);
  }

  template<class T, class... Args>
  void create(Args&&... args) {
    static_assert(sizeof(T) <= animationDataSize);
    Animation* animation = get();
    if (animation != nullptr) {
      animation->~Animation();
    }
    _created = false;
    new (_animationData) T(args...);
    _created = true;
  }
private:
  // largest size of any animation class, maybe this can be automated?
  static constexpr uint8_t animationDataSize = 155;
  uint8_t _animationData[animationDataSize];
  bool _created { false };
};

AnimationBuffer animationBuffer;
