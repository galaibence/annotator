#ifndef VISUALIZATION_SHADER_H
#define VISUALIZATION_SHADER_H

#include <cstdint>
#include <vector>

namespace visualization {

class Shader {
 private:
  uint32_t program_;
  std::vector<uint32_t> stages_;

 public:
  Shader() = default;

  void compileShaders();
  void loadShader(uint32_t stage, std::string path);

  uint32_t getProgram();
  void use();
};
}

#endif
