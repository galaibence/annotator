#ifndef COMMON_POINT_H
#define COMMON_POINT_H

#include <cstdint>

namespace common {

static uint8_t kXYZ = 1;
static uint8_t kRGB = 2;
static uint8_t kIntensity = 4;
static uint8_t kValidity = 8;

struct Point {
  uint8_t tag;

  float x;
  float y;
  float z;

  uint32_t r;
  uint32_t g;
  uint32_t b;

  uint8_t intensity;

  uint32_t valid;

  Point() : valid{false} {}

  void setXYZ(float X, float Y, float Z) {
    x = X;
    y = Y;
    z = Z;
    tag = tag | kXYZ;
  }

  void setRGB(uint32_t R, uint32_t G, uint32_t B) {
    r = R;
    g = G;
    b = B;
    tag = tag | kRGB;
  }

  void setInensity(uint8_t I) {
    intensity = I;
    tag = tag | kIntensity;
  }

  void setValidity(uint32_t V) {
    valid = V;
    tag = tag | kValidity;
  }
};
}

#endif
