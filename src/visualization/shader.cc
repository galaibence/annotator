#include "visualization/shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "gl_headers.h"

namespace visualization {

void Shader::compileShaders() {
  program_ = glCreateProgram();
  for (auto shader : stages_) glAttachShader(program_, shader);
  glLinkProgram(program_);

  GLint success;
  GLchar info_log[512];
  glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program_, 512, nullptr, info_log);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << info_log << std::endl;
  }

  for (auto shader : stages_) {
    glDeleteShader(shader);
  }
}
void Shader::loadShader(uint32_t stage, std::string path) {
  std::ifstream shader_file;
  shader_file.open(path.c_str());
  if (!shader_file.is_open()) {
    std::cout << "Cannot load shader at " << path << std::endl;
    exit(-1);
  }

  std::stringstream shader_stream;
  shader_stream << shader_file.rdbuf();
  shader_file.close();
  std::string shader_code = shader_stream.str();
  const char* shader_code_cstr = shader_code.c_str();

  GLint success;
  GLchar info_log[512];
  GLuint shader = glCreateShader(stage);
  glShaderSource(shader, 1, &shader_code_cstr, nullptr);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    std::cout << "ERROR::SHADER::STAGE" << stage << "::COMPILATION_FAILED\n"
              << info_log << std::endl;
  }

  stages_.push_back(shader);
}

uint32_t Shader::getProgram() { return program_; }

void Shader::use() { glUseProgram(this->program_); }
}
