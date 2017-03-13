#version 330

in vec3 g_color[];
in int g_selected[];

out vec3 f_color;

layout (points) in;
layout (points, max_vertices = 1) out;

void main() {
  if (g_selected[0] == 0) {
    gl_PointSize = 3.f;
    gl_Position = gl_in[0].gl_Position;
    f_color = g_color[0];
    EmitVertex();
    EndPrimitive();
  }
}