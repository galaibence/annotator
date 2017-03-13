#ifndef COMMON_CLOUD_H
#define COMMON_CLOUD_H

#include <cstdint>
#include <map>

#include "common/attributes.h"
#include "common/point.h"

namespace common {

typedef std::pair<std::string, FloatAttribute*> FloatAttributePair;
typedef std::map<std::string, FloatAttribute*> FloatAttributeMap;
typedef std::pair<std::string, UintAttribute*> UintAttributePair;
typedef std::map<std::string, UintAttribute*> UintAttributeMap;

class Cloud {
 private:
  FloatAttributeMap float_attributes_;
  UintAttributeMap uint_attributes_;
  size_t size_;

 public:
  Cloud();

  FloatAttributeMap getFloatAttributes();
  UintAttributeMap getUintAttributes();

  void addFloatAttribute(FloatAttribute* attribute);
  void addUintAttribute(UintAttribute* attribute);

  void addPoint(Point p);

  float* getPositionData();
  uint8_t* getColorData();
  uint8_t* getIntensityData();
  uint8_t* getValidityData();

  size_t size();
};
}

#endif
