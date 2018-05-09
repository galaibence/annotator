#ifndef COMMON_ATTRIBUTES_H
#define COMMON_ATTRIBUTES_H

#include <cfloat>
#include <cstdint>
#include <string>
#include <vector>

namespace common {

template <typename T>
class Attribute {
 protected:
  std::vector<T> data_;
  size_t component_size_;

 public:
  std::string attribute;

  Attribute(std::string attribute_name = "", size_t component_size = 3)
      : component_size_{component_size}, attribute{attribute_name} {}

  T* data() {
    if (data_.size() > 0)
      return data_.data();
    else
      return nullptr;
  }

  void addElement(T t) { data_.push_back(t); }

  size_t size() { return data_.size(); }

  size_t componentSize() { return component_size_; }
};

class FloatAttribute : public Attribute<float> {
 public:
  FloatAttribute(std::string attribute_name = "", size_t component_size = 3)
      : Attribute{attribute_name, component_size} {}
};

class UintAttribute : public Attribute<uint32_t> {
 public:
  UintAttribute(std::string attribute_name = "", size_t component_size = 3)
      : Attribute{attribute_name, component_size} {}
};

class PositionAttribute : public FloatAttribute {
 private:
  float min_x_;
  float min_y_;
  float min_z_;

  float max_x_;
  float max_y_;
  float max_z_;

 public:
  PositionAttribute()
      : FloatAttribute{"position", 3},
        min_x_{FLT_MAX},
        min_y_{FLT_MAX},
        min_z_{FLT_MAX},
        max_x_{FLT_MIN},
        max_y_{FLT_MIN},
        max_z_{FLT_MIN} {}

  void getMinMax3D();

  float minX() { return min_x_; }
  float minY() { return min_y_; };
  float minZ() { return min_z_; };

  float maxX() { return max_x_; };
  float maxY() { return max_y_; };
  float maxZ() { return max_z_; };
};

class ColorAttribute : public UintAttribute {
 public:
  ColorAttribute() : UintAttribute{"color", 3} {}
};

class IntensityAttribute : public UintAttribute {
 public:
  IntensityAttribute() : UintAttribute{"intensity", 1} {}
};

class ValidityAttribute : public UintAttribute {
 public:
  ValidityAttribute() : UintAttribute{"validity", 1} {}
};
}

#endif
