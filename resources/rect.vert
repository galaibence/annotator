#version 330

layout (location = 0) in vec2 vertex;

uniform int height;
uniform int width;

void main() {

  float x = vertex.x / (width * 0.5f) - 1.f;
  float y = -(vertex.y / (height * 0.5f) - 1.f);

  gl_Position = vec4(x, y, 0.f, 1.f);

}

