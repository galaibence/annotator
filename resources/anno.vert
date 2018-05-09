#version 330

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 color;
layout (location = 2) in uint selected;

out uint g_selected;
out vec3 g_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int number_of_active_selections;
uniform int number_of_active_negative_selections;
uniform float active_selection[16 * 30];
uniform float active_negative_selection[16 * 30];

void main() {

  bool active_selected = false;
  int last_selection_if_selected = -1;
  for (int i = 0; i < number_of_active_selections; i++) {
    vec3 orig = vec3(
      active_selection[i * 16 + 0 * 3 + 0],
      active_selection[i * 16 + 0 * 3 + 1],
      active_selection[i * 16 + 0 * 3 + 2]);
    vec3 dir0 = vec3(
      active_selection[i * 16 + 1 * 3 + 0],
      active_selection[i * 16 + 1 * 3 + 1],
      active_selection[i * 16 + 1 * 3 + 2]);
    vec3 dir1 = vec3(
      active_selection[i * 16 + 2 * 3 + 0],
      active_selection[i * 16 + 2 * 3 + 1],
      active_selection[i * 16 + 2 * 3 + 2]);
    vec3 dir2 = vec3(
      active_selection[i * 16 + 3 * 3 + 0],
      active_selection[i * 16 + 3 * 3 + 1],
      active_selection[i * 16 + 3 * 3 + 2]);
    vec3 dir3 = vec3(
      active_selection[i * 16 + 4 * 3 + 0],
      active_selection[i * 16 + 4 * 3 + 1],
      active_selection[i * 16 + 4 * 3 + 2]);

    vec3 N0 = normalize(cross(normalize(dir0), normalize(dir1)));
    vec3 N1 = normalize(cross(normalize(dir1), normalize(dir2)));
    vec3 N2 = normalize(cross(normalize(dir2), normalize(dir3)));
    vec3 N3 = normalize(cross(normalize(dir3), normalize(dir0)));

    if ((dot(N0, (vertex - orig)) > 0 && dot(N1, (vertex - orig)) > 0 && dot(N2, (vertex - orig)) > 0 && dot(N3, (vertex - orig)) > 0)) {
      active_selected = true;
      last_selection_if_selected = int(active_selection[i * 16 + 15]);
    }
  }

  int last_negative_selection_if_selected = -1;
  for (int i = 0; i < number_of_active_negative_selections; i++) {
    vec3 orig = vec3(
      active_negative_selection[i * 16 + 0 * 3 + 0],
      active_negative_selection[i * 16 + 0 * 3 + 1],
      active_negative_selection[i * 16 + 0 * 3 + 2]);
    vec3 dir0 = vec3(
      active_negative_selection[i * 16 + 1 * 3 + 0],
      active_negative_selection[i * 16 + 1 * 3 + 1],
      active_negative_selection[i * 16 + 1 * 3 + 2]);
    vec3 dir1 = vec3(
      active_negative_selection[i * 16 + 2 * 3 + 0],
      active_negative_selection[i * 16 + 2 * 3 + 1],
      active_negative_selection[i * 16 + 2 * 3 + 2]);
    vec3 dir2 = vec3(
      active_negative_selection[i * 16 + 3 * 3 + 0],
      active_negative_selection[i * 16 + 3 * 3 + 1],
      active_negative_selection[i * 16 + 3 * 3 + 2]);
    vec3 dir3 = vec3(
      active_negative_selection[i * 16 + 4 * 3 + 0],
      active_negative_selection[i * 16 + 4 * 3 + 1],
      active_negative_selection[i * 16 + 4 * 3 + 2]);

    vec3 N0 = normalize(cross(normalize(dir0), normalize(dir1)));
    vec3 N1 = normalize(cross(normalize(dir1), normalize(dir2)));
    vec3 N2 = normalize(cross(normalize(dir2), normalize(dir3)));
    vec3 N3 = normalize(cross(normalize(dir3), normalize(dir0)));

    if ((dot(N0, (vertex - orig)) > 0 && dot(N1, (vertex - orig)) > 0 && dot(N2, (vertex - orig)) > 0 && dot(N3, (vertex - orig)) > 0)) {
      last_negative_selection_if_selected = int(active_negative_selection[i * 16 + 15]);
    }
  }

  if (selected == uint(255)) {
    g_color = vec3(0.0f, 0.5f, 0.0f) + color * 1.f / 255.f;
  }
  else if (active_selected &&
      last_selection_if_selected >
      last_negative_selection_if_selected)
    g_color = vec3(0.5f, 0.0f, 0.0f) + color * 1.f / 255.f;
  else g_color = color * 1.f / 255.f;

  g_selected = selected;
  gl_Position = projection * view * model * vec4(vertex, 1.0f);
}
