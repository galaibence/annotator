#include "annotation/annotator.h"

#include <functional>
#include <vector>

#include "annotation/selection.h"

namespace annotation {

bool Annotator::inSelection() { return selection_started_; }
void Annotator::triggerSelectionStart() { selection_started_ = true; }
void Annotator::triggerSelectionEnd() { selection_started_ = false; }

void Annotator::startSelection(Selection selection) {
  active_selection_count_++;
  selection.p[15] = active_selection_count_;
  selections_.push_back(selection);
}

void Annotator::startNegativeSelection(Selection selection) {
  active_selection_count_++;
  selection.p[15] = active_selection_count_;
  negative_selections_.push_back(selection);
}

void Annotator::stopSelection(Selection selection) {
  selection.p[15] = active_selection_count_;
  selections_[selections_.size() - 1] = selection;
}

void Annotator::stopNegativeSelection(Selection selection) {
  selection.p[15] = active_selection_count_;
  negative_selections_[negative_selections_.size() - 1] = selection;
}

Selection Annotator::getCurrentSelection(
    std::function<void(float, float, glm::vec3&, glm::vec3&)> clickToLine) {
  int rec_min_x = std::min(rectangle_.topLeftX, rectangle_.bottomRightX);
  int rec_max_x = std::max(rectangle_.topLeftX, rectangle_.bottomRightX);
  int rec_min_y = std::min(rectangle_.topLeftY, rectangle_.bottomRightY);
  int rec_max_y = std::max(rectangle_.topLeftY, rectangle_.bottomRightY);

  glm::vec3 orig, dirs[4];
  clickToLine(rec_min_x, rec_min_y, orig, dirs[0]);
  clickToLine(rec_max_x, rec_min_y, orig, dirs[1]);
  clickToLine(rec_max_x, rec_max_y, orig, dirs[2]);
  clickToLine(rec_min_x, rec_max_y, orig, dirs[3]);

  Selection selection;
  selection.p[0 * 3 + 0] = orig.x;
  selection.p[0 * 3 + 1] = orig.y;
  selection.p[0 * 3 + 2] = orig.z;
  for (int i = 0; i < 4; i++) {
    selection.p[(i + 1) * 3 + 0] = dirs[i].x;
    selection.p[(i + 1) * 3 + 1] = dirs[i].y;
    selection.p[(i + 1) * 3 + 2] = dirs[i].z;
  }
  return selection;
}

void Annotator::changeTopLeft(float x, float y) {
  rectangle_.topLeftX = x;
  rectangle_.topLeftY = y;
}

void Annotator::changeBottomRight(float x, float y) {
  rectangle_.bottomRightX = x;
  rectangle_.bottomRightY = y;
}
}
