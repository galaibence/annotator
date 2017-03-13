#include "common/attributes.h"

#include <cfloat>

namespace common {

void PositionAttribute::getMinMax3D() {
  min_x_ = FLT_MAX;
  min_y_ = FLT_MAX;
  min_z_ = FLT_MAX;

  max_x_ = FLT_MIN;
  max_y_ = FLT_MIN;
  max_z_ = FLT_MIN;

  float x, y, z;
  float* positions = data_.data();
  for (size_t i = 0; i < data_.size(); i += 3) {
    x = positions[i * 3 + 0];
    y = positions[i * 3 + 1];
    z = positions[i * 3 + 2];

    if (x < min_x_) min_x_ = x;
    if (y < min_y_) min_y_ = y;
    if (z < min_z_) min_z_ = z;

    if (x > max_x_) max_x_ = x;
    if (y > max_y_) max_y_ = y;
    if (z > max_z_) max_z_ = z;
  }
}
}
