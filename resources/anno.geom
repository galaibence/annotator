#version 330

in vec3 g_color[];
in uint g_selected[];

out vec3 f_color;

layout (points) in;
layout (points, max_vertices = 1) out;

void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 3.f;
    f_color = g_color[0];

    if(g_selected[0] != uint(255)) {
        EmitVertex();
        EndPrimitive();
    }
}
