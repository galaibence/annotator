#include <chrono>
#include <fstream>
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
  cloud.addUintAttribute(new ValidityAttribute());

  float x, y, z;
  int r, g, b;
  Point p;

  std::ifstream ifs;
  ifs.open(argv[1]);

  while (ifs >> x >> y >> z >> r >> g >> b) {
    p.setXYZ(x, y, z);
    p.setRGB((uint32_t)r, (uint32_t)g, (uint32_t)b);
    p.setValidity(255);
    cloud.addPoint(p);
  }

  visualization::Viewer viewer(&cloud);
  viewer.init(600, 600);
  viewer.draw();
  viewer.close();

  return 0;
}
