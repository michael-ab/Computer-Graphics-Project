#pragma once


GLuint InitShader(const char* vShaderFile, const char* fShaderFile);

static char* readShaderSource(const char* shaderFile);