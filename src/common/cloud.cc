#include "common/cloud.h"

#include <iostream>

#include "common/attributes.h"
#include "common/point.h"

namespace common {

Cloud::Cloud() : size_{0} {}

size_t Cloud::size() { return size_; }

FloatAttributeMap Cloud::getFloatAttributes() { return float_attributes_; }

UintAttributeMap Cloud::getUintAttributes() { return uint_attributes_; }

void Cloud::addFloatAttribute(FloatAttribute* attribute) {
  float_attributes_.insert(FloatAttributePair(attribute->attribute, attribute));
}

void Cloud::addUintAttribute(UintAttribute* attribute) {
  uint_attributes_.insert(UintAttributePair(attribute->attribute, attribute));
}

void Cloud::addPoint(Point p) {
  bool added = false;
  if (p.tag & common::kXYZ) {
    auto iter = float_attributes_.find("position");
    if (iter != float_attributes_.end()) {
      iter->second->addElement(p.x);
      iter->second->addElement(p.y);
      iter->second->addElement(p.z);
    }
    added = true;
  }
  if (p.tag & common::kRGB) {
    auto iter = uint_attributes_.find("color");
    if (iter != uint_attributes_.end()) {
      iter->second->addElement(p.r);
      iter->second->addElement(p.g);
      iter->second->addElement(p.b);
    }
    added = true;
  }
  if (p.tag & common::kIntensity) {
    auto iter = uint_attributes_.find("intensity");
    if (iter != uint_attributes_.end()) iter->second->addElement(p.intensity);
    added = true;
  }
  if (p.tag & common::kValidity) {
    auto iter = uint_attributes_.find("validity");
    if (iter != uint_attributes_.end()) iter->second->addElement(p.valid);
    added = true;
  }

  if (added) size_++;
}

float* Cloud::getPositionData() {
  auto iter = float_attributes_.find("position");

  if (iter == float_attributes_.end())
    return nullptr;
  else
    return iter->second->data();
}

uint32_t* Cloud::getColorData() {
  auto iter = uint_attributes_.find("color");

  if (iter == uint_attributes_.end())
    return nullptr;
  else
    return iter->second->data();
}

uint32_t* Cloud::getIntensityData() {
  auto iter = uint_attributes_.find("intensity");

  if (iter == uint_attributes_.end())
    return nullptr;
  else
    return iter->second->data();
}

uint32_t* Cloud::getValidityData() {
  auto iter = uint_attributes_.find("validity");

  if (iter == uint_attributes_.end())
    return nullptr;
  else
    return iter->second->data();
}
}
