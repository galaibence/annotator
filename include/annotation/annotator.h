#ifndef ANNOTATION_ANNOTATOR_H
#define ANNOTATION_ANNOTATOR_H

#include <functional>
#include <vector>

#include "annotation/selection.h"
#include "gl_headers.h"

namespace annotation {

class Annotator {
 public:
  struct Rectangle {
    float topLeftX;
    float topLeftY;
    float bottomRightX;
    float bottomRightY;
  };

  bool inSelection();
  void triggerSelectionStart();
  void triggerSelectionEnd();

  void startSelection(Selection selection);
  void startNegativeSelection(Selection selection);
  void stopSelection(Selection selection);
  void stopNegativeSelection(Selection selection);

  Selection getCurrentSelection(
      std::function<void(float, float, glm::vec3&, glm::vec3&)>);

  void changeTopLeft(float x, float y);
  void changeBottomRight(float x, float y);

  int numberOfActiveSelections() { return selections_.size(); }
  int numberOfActiveNegativeSelections() { return negative_selections_.size(); }

  Selection* getActiveSelectionFloatArray() { return selections_.data(); }
  Selection* getActiveNegativeSelectionFloatArray() {
    return negative_selections_.data();
  }

  void getRectangle(float& topLeftX, float& topLeftY, float& bottomRightX,
                    float& bottomRightY) {
    topLeftX = std::min(rectangle_.topLeftX, rectangle_.bottomRightX);
    topLeftY = std::min(rectangle_.topLeftY, rectangle_.bottomRightY);
    bottomRightX = std::max(rectangle_.topLeftX, rectangle_.bottomRightX);
    bottomRightY = std::max(rectangle_.topLeftY, rectangle_.bottomRightY);
  }

  void clearSelection() {
    selections_.clear();
    negative_selections_.clear();
    active_selection_count_ = 0;
  }

 private:
  std::vector<Selection> selections_;
  std::vector<Selection> negative_selections_;
  int active_selection_count_;

  Rectangle rectangle_;

  bool selection_started_;
};
}

#endif
