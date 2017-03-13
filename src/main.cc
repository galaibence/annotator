#include <iostream>

#include "common/cloud.h"
#include "gl_headers.h"
#include "visualization/viewer.h"

using namespace common;
int main(int argc, char* argv[]) {
  std::srand(std::time(0));

  Cloud cloud;
  cloud.addFloatAttribute(new PositionAttribute());
  cloud.addUintAttribute(new ColorAttribute());

  float x, y, z;
  uint8_t r, g, b;
  Point p;
  for (size_t i = 0; i < 1000; i++) {
    x = ((std::rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 1.0f;
    y = ((std::rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 1.0f;
    z = ((std::rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 1.0f;
    r = rand() % 256;
    g = rand() % 256;
    b = rand() % 256;
    p.setXYZ(x, y, z);
    p.setRGB(r, g, b);
    cloud.addPoint(p);
  }

  visualization::Viewer viewer(&cloud, 600, 600);
  viewer.init();
  viewer.draw();
  return 0;
}
