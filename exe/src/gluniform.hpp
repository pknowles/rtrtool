// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <span>

// clang-format off
inline void setUniform(GLuint location, const float& v)      { glUniform1f(location, v); }
inline void setUniform(GLuint location, const glm::vec2& v)  { glUniform2f(location, v.x, v.y); }
inline void setUniform(GLuint location, const glm::vec3& v)  { glUniform3f(location, v.x, v.y, v.z); }
inline void setUniform(GLuint location, const glm::vec4& v)  { glUniform4f(location, v.x, v.y, v.z, v.w); }
inline void setUniform(GLuint location, const int32_t& v)    { glUniform1i(location, v); }
inline void setUniform(GLuint location, const glm::ivec2& v) { glUniform2i(location, v.x, v.y); }
inline void setUniform(GLuint location, const glm::ivec3& v) { glUniform3i(location, v.x, v.y, v.z); }
inline void setUniform(GLuint location, const glm::ivec4& v) { glUniform4i(location, v.x, v.y, v.z, v.w); }
inline void setUniform(GLuint location, const uint32_t& v)   { glUniform1ui(location, v); }
inline void setUniform(GLuint location, const glm::uvec2& v) { glUniform2ui(location, v.x, v.y); }
inline void setUniform(GLuint location, const glm::uvec3& v) { glUniform3ui(location, v.x, v.y, v.z); }
inline void setUniform(GLuint location, const glm::uvec4& v) { glUniform4ui(location, v.x, v.y, v.z, v.w); }
inline void setUniform(GLuint location, const glm::mat2& v)    { glUniformMatrix2fv(location,   1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat3& v)    { glUniformMatrix3fv(location,   1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat4& v)    { glUniformMatrix4fv(location,   1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat2x3& v)  { glUniformMatrix2x3fv(location, 1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat3x2& v)  { glUniformMatrix3x2fv(location, 1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat2x4& v)  { glUniformMatrix2x4fv(location, 1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat4x2& v)  { glUniformMatrix4x2fv(location, 1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat3x4& v)  { glUniformMatrix3x4fv(location, 1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const glm::mat4x3& v)  { glUniformMatrix4x3fv(location, 1, GL_FALSE, glm::value_ptr(v)); }
inline void setUniform(GLuint location, const std::span<const float>& v)       { glUniform1fv(location,  GLsizei(v.size()), reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::vec2>& v)   { glUniform2fv(location,  GLsizei(v.size()), reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::vec3>& v)   { glUniform3fv(location,  GLsizei(v.size()), reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::vec4>& v)   { glUniform4fv(location,  GLsizei(v.size()), reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const int32_t>& v)     { glUniform1iv(location,  GLsizei(v.size()), reinterpret_cast<const int32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::ivec2>& v)  { glUniform2iv(location,  GLsizei(v.size()), reinterpret_cast<const int32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::ivec3>& v)  { glUniform3iv(location,  GLsizei(v.size()), reinterpret_cast<const int32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::ivec4>& v)  { glUniform4iv(location,  GLsizei(v.size()), reinterpret_cast<const int32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const uint32_t>& v)    { glUniform1uiv(location, GLsizei(v.size()), reinterpret_cast<const uint32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::uvec2>& v)  { glUniform2uiv(location, GLsizei(v.size()), reinterpret_cast<const uint32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::uvec3>& v)  { glUniform3uiv(location, GLsizei(v.size()), reinterpret_cast<const uint32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::uvec4>& v)  { glUniform4uiv(location, GLsizei(v.size()), reinterpret_cast<const uint32_t*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat2>& v)   { glUniformMatrix2fv(location,   GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat3>& v)   { glUniformMatrix3fv(location,   GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat4>& v)   { glUniformMatrix4fv(location,   GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat2x3>& v) { glUniformMatrix2x3fv(location, GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat3x2>& v) { glUniformMatrix3x2fv(location, GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat2x4>& v) { glUniformMatrix2x4fv(location, GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat4x2>& v) { glUniformMatrix4x2fv(location, GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat3x4>& v) { glUniformMatrix3x4fv(location, GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
inline void setUniform(GLuint location, const std::span<const glm::mat4x3>& v) { glUniformMatrix4x3fv(location, GLsizei(v.size()), GL_FALSE, reinterpret_cast<const float*>(v.data())); }
// clang-format on
