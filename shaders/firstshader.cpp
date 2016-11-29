/**
* @file firstshader.cpp
* @author Erik Sandrén
* @date 24-11-2016
* @brief [Description Goes Here]
*/

#include <cstring>
#include <ai.h>
AI_SHADER_NODE_EXPORT_METHODS(SimpleMethods);

namespace {
  enum simpleParams { p_color};
};

node_parameters {
  AiParameterRGB("constantColor", 0.5f, 0.5f, 0.5f);
}

node_initialize { }

node_update { }

node_finish { }

shader_evaluate {
  AtColor color = AiShaderEvalParamRGB(p_color);
  sg->out.RGB = color;
}

node_loader {
  if(i > 0)
    return false;

  node->methods = SimpleMethods;
  node->output_type = AI_TYPE_RGB;
  node->name = "simple";
  node->node_type = AI_NODE_SHADER;

  strcpy(node->version, AI_VERSION);

  return true;
}
