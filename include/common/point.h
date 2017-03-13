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

  uint8_t r;
  uint8_t g;
  uint8_t b;

  uint8_t intensity;

  uint8_t valid;

  Point() : valid{false} {}

  void setXYZ(float X, float Y, float Z) {
    x = X;
    y = Y;
    z = Z;
    tag = tag | kXYZ;
  }

  void setRGB(uint8_t R, uint8_t G, uint8_t B) {
    r = R;
    g = G;
    b = B;
    tag = tag | kRGB;
  }

  void setInensity(uint8_t I) {
    intensity = I;
    tag = tag | kIntensity;
  }

  void setValidity(uint8_t V) {
    valid = V;
    tag = tag | kValidity;
  }
};
}

#endif
