// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <gluniform.hpp>
#include <initializer_list>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_set>

namespace glraii {

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <typename Vec>
inline constexpr bool is_glm_vector = false;

template <glm::length_t L, class T, glm::qualifier Q>
inline constexpr bool is_glm_vector<glm::vec<L, T, Q>> = true;

static_assert(is_glm_vector<glm::vec3>);

// clang-format off
template <class T>
struct gl_format_from;
template <> struct gl_format_from<float>    { static constexpr GLenum type = GL_FLOAT; };
template <> struct gl_format_from<double>   { static constexpr GLenum type = GL_DOUBLE; };
template <> struct gl_format_from<int32_t>  { static constexpr GLenum type = GL_INT; };
template <> struct gl_format_from<int16_t>  { static constexpr GLenum type = GL_SHORT; };
template <> struct gl_format_from<int8_t>   { static constexpr GLenum type = GL_BYTE; };
template <> struct gl_format_from<uint32_t> { static constexpr GLenum type = GL_UNSIGNED_INT; };
template <> struct gl_format_from<uint16_t> { static constexpr GLenum type = GL_UNSIGNED_SHORT; };
template <> struct gl_format_from<uint8_t>  { static constexpr GLenum type = GL_UNSIGNED_BYTE; };
// clang-format on

template <glm::length_t L, class T, glm::qualifier Q>
struct gl_format_from<glm::vec<L, T, Q>> : gl_format_from<T> {};

template <class T>
inline constexpr GLenum gl_format_from_v = gl_format_from<T>::type;

inline void constructVerify([[maybe_unused]] bool condition) { assert(condition); }

class Shader {
public:
    Shader(GLenum type, const std::string_view& source) {
        m_shader = glCreateShader(type);
        constructVerify(m_shader != 0);
        const char* sources[1] = {source.data()};
        const GLint sourceLengths[1] = {GLint(source.size())};
        glShaderSource(m_shader, 1, sources, sourceLengths);
        glCompileShader(m_shader);
        GLint success;
        glGetShaderiv(m_shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            auto log = infoLog();
            release_();
            throw std::runtime_error(log);
        }
    }
    Shader(const Shader& other) = delete;
    Shader(Shader&& other) noexcept
        : m_shader(other.m_shader) {
        other.invalidate_();
    }
    ~Shader() noexcept { release_(); }
    Shader& operator=(const Shader& other) = delete;
    Shader& operator=(Shader&& other) noexcept {
        release_();
        m_shader = other.m_shader;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_shader; }
    std::string infoLog() {
        GLint maxLength = 0;
        glGetShaderiv(m_shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::string result(maxLength, '\0');
        glGetShaderInfoLog(m_shader, maxLength, &maxLength, result.data());
        result.resize(maxLength);
        return result;
    }

private:
    void release_() {
        if (m_shader) {
            glDeleteShader(m_shader);
        }
    };
    void   invalidate_() { m_shader = 0; }
    GLuint m_shader = 0;
};

class Program {
public:
    Program(std::initializer_list<Shader> shaders) {
        m_program = glCreateProgram();
        constructVerify(m_program != 0);
        for (const Shader& shader : shaders) {
            glAttachShader(m_program, shader);
        }
        glLinkProgram(m_program);
        GLint success;
        glGetProgramiv(m_program, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            auto log = infoLog();
            release_();
            throw std::runtime_error(log);
        }
    }
    Program(const Program& other) = delete;
    Program(Program&& other) noexcept
        : m_program(other.m_program) {
        other.invalidate_();
    }
    ~Program() noexcept { release_(); }
    Program& operator=(const Program& other) = delete;
    Program& operator=(Program&& other) noexcept {
        release_();
        m_program = other.m_program;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_program; }
    std::string infoLog() {
        GLint maxLength = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &maxLength);
        std::string result(maxLength, '\0');
        glGetProgramInfoLog(m_program, maxLength, &maxLength, result.data());
        result.resize(maxLength);
        return result;
    }
    GLint uniformLocation(const std::string& name) {
        GLint result = glGetUniformLocation(m_program, name.c_str());
        if (result == -1)
            fprintf(stderr, "Program %u has no uniform '%s'\n", m_program, name.c_str());
        return result;
    }
    template <class T>
    void setUniform(const std::string& name, const T& value) {
        ::setUniform(uniformLocation(name), value);
    }

private:
    void release_() {
        if (m_program) {
            glDeleteProgram(m_program);
        }
    };
    void   invalidate_() { m_program = 0; }
    GLuint m_program = 0;
};

class Texture {
public:
    Texture(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height,
            GLsizei depth) {
        glCreateTextures(target, 1, &m_texture);
        constructVerify(m_texture != 0);
        switch (target) {
        case GL_TEXTURE_1D:
            [[fallthrough]];
        case GL_PROXY_TEXTURE_1D:
            assert(height <= 1);
            assert(depth <= 1);
            glTextureStorage1D(m_texture, levels, internalformat, width);
            break;
        case GL_TEXTURE_3D:
            [[fallthrough]];
        case GL_TEXTURE_2D_ARRAY:
            [[fallthrough]];
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            [[fallthrough]];
        case GL_PROXY_TEXTURE_3D:
            [[fallthrough]];
        case GL_PROXY_TEXTURE_2D_ARRAY:
            [[fallthrough]];
        case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
            glTextureStorage3D(m_texture, levels, internalformat, width, height, depth);
            break;
        default:
            assert(depth <= 1);
            glTextureStorage2D(m_texture, levels, internalformat, width, height);
            break;
        }
    }
    Texture(const Texture& other) = delete;
    Texture(Texture&& other) noexcept
        : m_texture(other.m_texture) {
        other.invalidate_();
    }
    ~Texture() noexcept { release_(); }
    Texture& operator=(const Texture& other) = delete;
    Texture& operator=(Texture&& other) noexcept {
        release_();
        m_texture = other.m_texture;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_texture; }

private:
    void release_() {
        if (m_texture) {
            glDeleteTextures(1, &m_texture);
        }
    };
    void   invalidate_() { m_texture = 0; }
    GLuint m_texture = 0;
};

class Framebuffer {
public:
    Framebuffer() {
        glCreateFramebuffers(1, &m_framebuffer);
        constructVerify(m_framebuffer != 0);
    }
    Framebuffer(const Framebuffer& other) = delete;
    Framebuffer(Framebuffer&& other) noexcept
        : m_framebuffer(other.m_framebuffer) {
        other.invalidate_();
    }
    ~Framebuffer() noexcept { release_(); }
    Framebuffer& operator=(const Framebuffer& other) = delete;
    Framebuffer& operator=(Framebuffer&& other) noexcept {
        release_();
        m_framebuffer = other.m_framebuffer;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_framebuffer; }

private:
    void release_() {
        if (m_framebuffer) {
            glDeleteFramebuffers(1, &m_framebuffer);
        }
    };
    void   invalidate_() { m_framebuffer = 0; }
    GLuint m_framebuffer = 0;
};

class Renderbuffer {
public:
    Renderbuffer() {
        glCreateRenderbuffers(1, &m_renderbuffer);
        constructVerify(m_renderbuffer != 0);
    }
    Renderbuffer(const Renderbuffer& other) = delete;
    Renderbuffer(Renderbuffer&& other) noexcept
        : m_renderbuffer(other.m_renderbuffer) {
        other.invalidate_();
    }
    ~Renderbuffer() noexcept { release_(); }
    Renderbuffer& operator=(const Renderbuffer& other) = delete;
    Renderbuffer& operator=(Renderbuffer&& other) noexcept {
        release_();
        m_renderbuffer = other.m_renderbuffer;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_renderbuffer; }

private:
    void release_() {
        if (m_renderbuffer) {
            glDeleteRenderbuffers(1, &m_renderbuffer);
        }
    };
    void   invalidate_() { m_renderbuffer = 0; }
    GLuint m_renderbuffer = 0;
};

class Buffer {
public:
    Buffer(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_STATIC_DRAW) {
        glCreateBuffers(1, &m_buffer);
        constructVerify(m_buffer != 0);
        glNamedBufferData(m_buffer, size, data, usage);
    }
    template <std::ranges::input_range Range>
        requires std::ranges::contiguous_range<Range>
    Buffer(const Range& range, GLenum usage = GL_STATIC_DRAW)
        : Buffer(std::size(range) * sizeof(std::ranges::range_value_t<Range>), std::data(range),
                 usage) {}
    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other) noexcept
        : m_buffer(other.m_buffer) {
        other.invalidate_();
    }
    ~Buffer() noexcept { release_(); }
    Buffer& operator=(const Buffer& other) = delete;
    Buffer& operator=(Buffer&& other) noexcept {
        release_();
        m_buffer = other.m_buffer;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_buffer; }

private:
    void release_() {
        if (m_buffer) {
            glDeleteBuffers(1, &m_buffer);
        }
    };
    void   invalidate_() { m_buffer = 0; }
    GLuint m_buffer = 0;
};

class EmptyVertexArray {
public:
    EmptyVertexArray() {
        glCreateVertexArrays(1, &m_vertexArray);
        constructVerify(m_vertexArray != 0);
    }
    EmptyVertexArray(const EmptyVertexArray& other) = delete;
    EmptyVertexArray(EmptyVertexArray&& other) noexcept
        : m_vertexArray(other.m_vertexArray) {
        other.invalidate_();
    }
    ~EmptyVertexArray() noexcept { release_(); }
    EmptyVertexArray& operator=(const EmptyVertexArray& other) = delete;
    EmptyVertexArray& operator=(EmptyVertexArray&& other) noexcept {
        release_();
        m_vertexArray = other.m_vertexArray;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_vertexArray; }

private:
    void release_() {
        if (m_vertexArray) {
            glDeleteVertexArrays(1, &m_vertexArray);
        }
    };
    void   invalidate_() { m_vertexArray = 0; }
    GLuint m_vertexArray = 0;
};

class VertexArray : public EmptyVertexArray {
public:
    struct Format {
        GLint     size = 0;
        GLenum    type = 0;
        GLboolean normalized = GL_FALSE;
        GLuint    relativeoffset = 0;
    };
    struct IntegerFormat {
        GLint  size = 0;
        GLenum type = 0;
        GLuint relativeoffset = 0;
    };
    struct DoubleFormat {
        GLint  size = 0;
        GLuint relativeoffset = 0;
    };
    struct Attrib {
        GLuint                                            bindingIndex = 0;
        const Buffer&                                     buffer;
        GLintptr                                          offset = 0;
        GLsizei                                           stride = 0;
        std::variant<Format, IntegerFormat, DoubleFormat> format;

        template <class T>
        static Attrib contiguous(const Buffer& buffer, GLuint bindingIndex) {
            GLint size = 1;
            if constexpr (is_glm_vector<std::decay_t<T>>)
                size = std::decay_t<T>::length();
            return Attrib{.bindingIndex = bindingIndex,
                          .buffer = buffer,
                          .offset = 0,
                          .stride = sizeof(std::decay_t<T>),
                          .format = Format{.size = size,
                                           .type = gl_format_from_v<std::decay_t<T>>,
                                           .normalized = GL_FALSE,
                                           .relativeoffset = 0}};
        }
    };
    VertexArray(const Buffer& elementBuffer, std::initializer_list<Attrib> attribs) {
#if !NDEBUG
        std::unordered_set<GLuint> bindingIndices;
        for (const auto& attrib : attribs) {
            assert(bindingIndices.count(attrib.bindingIndex) == 0);
            bindingIndices.insert(attrib.bindingIndex);
        }
#endif
        glVertexArrayElementBuffer(*this, elementBuffer);
        GLuint indexNext = 0;
        for (const auto& attrib : attribs) {
            GLuint index = indexNext++;
            glEnableVertexArrayAttrib(*this, index);
            glVertexArrayAttribBinding(*this, index, attrib.bindingIndex);
            glVertexArrayVertexBuffer(*this, attrib.bindingIndex, attrib.buffer, attrib.offset,
                                      attrib.stride);
            std::visit(
                overloaded{[this, &index](const Format& format) {
                               glVertexArrayAttribFormat(*this, index, format.size, format.type,
                                                         format.normalized, format.relativeoffset);
                           },
                           [this, &index](const IntegerFormat& format) {
                               glVertexArrayAttribIFormat(*this, index, format.size, format.type,
                                                          format.relativeoffset);
                           },
                           [this, &index](const DoubleFormat& format) {
                               glVertexArrayAttribLFormat(*this, index, format.size, GL_DOUBLE,
                                                          format.relativeoffset);
                           }},
                attrib.format);
        }
    }
};

class Query {
public:
    Query(GLenum type) {
        glCreateQueries(type, 1, &m_query);
        constructVerify(m_query != 0);
    }
    Query(const Query& other) = delete;
    Query(Query&& other) noexcept
        : m_query(other.m_query) {
        other.invalidate_();
    }
    ~Query() noexcept { release_(); }
    Query& operator=(const Query& other) = delete;
    Query& operator=(Query&& other) noexcept {
        release_();
        m_query = other.m_query;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_query; }

private:
    void release_() {
        if (m_query) {
            glDeleteQueries(1, &m_query);
        }
    };
    void   invalidate_() { m_query = 0; }
    GLuint m_query = 0;
};

class Sampler {
public:
    Sampler() {
        glCreateSamplers(1, &m_sampler);
        constructVerify(m_sampler != 0);
    }
    Sampler(const Sampler& other) = delete;
    Sampler(Sampler&& other) noexcept
        : m_sampler(other.m_sampler) {
        other.invalidate_();
    }
    ~Sampler() noexcept { release_(); }
    Sampler& operator=(const Sampler& other) = delete;
    Sampler& operator=(Sampler&& other) noexcept {
        release_();
        m_sampler = other.m_sampler;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_sampler; }

private:
    void release_() {
        if (m_sampler) {
            glDeleteSamplers(1, &m_sampler);
        }
    };
    void   invalidate_() { m_sampler = 0; }
    GLuint m_sampler = 0;
};

class TransformFeedback {
public:
    TransformFeedback() {
        glCreateTransformFeedbacks(1, &m_transformFeedback);
        constructVerify(m_transformFeedback != 0);
    }
    TransformFeedback(const TransformFeedback& other) = delete;
    TransformFeedback(TransformFeedback&& other) noexcept
        : m_transformFeedback(other.m_transformFeedback) {
        other.invalidate_();
    }
    ~TransformFeedback() noexcept { release_(); }
    TransformFeedback& operator=(const TransformFeedback& other) = delete;
    TransformFeedback& operator=(TransformFeedback&& other) noexcept {
        release_();
        m_transformFeedback = other.m_transformFeedback;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_transformFeedback; }

private:
    void release_() {
        if (m_transformFeedback) {
            glDeleteTransformFeedbacks(1, &m_transformFeedback);
        }
    };
    void   invalidate_() { m_transformFeedback = 0; }
    GLuint m_transformFeedback = 0;
};

class Sync {
public:
    Sync(GLenum condition, GLbitfield flags) {
        m_sync = glFenceSync(condition, flags);
        constructVerify(m_sync != 0);
    }
    Sync(const Sync& other) = delete;
    Sync(Sync&& other) noexcept
        : m_sync(other.m_sync) {
        other.invalidate_();
    }
    ~Sync() noexcept { release_(); }
    Sync& operator=(const Sync& other) = delete;
    Sync& operator=(Sync&& other) noexcept {
        release_();
        m_sync = other.m_sync;
        other.invalidate_();
        return *this;
    }
    operator const GLsync&() const { return m_sync; }

private:
    void release_() {
        if (m_sync) {
            glDeleteSync(m_sync);
        }
    };
    void   invalidate_() { m_sync = nullptr; }
    GLsync m_sync = nullptr;
};

class ProgramPipeline {
public:
    ProgramPipeline() {
        glCreateProgramPipelines(1, &m_programPipeline);
        constructVerify(m_programPipeline != 0);
    }
    ProgramPipeline(const ProgramPipeline& other) = delete;
    ProgramPipeline(ProgramPipeline&& other) noexcept
        : m_programPipeline(other.m_programPipeline) {
        other.invalidate_();
    }
    ~ProgramPipeline() noexcept { release_(); }
    ProgramPipeline& operator=(const ProgramPipeline& other) = delete;
    ProgramPipeline& operator=(ProgramPipeline&& other) noexcept {
        release_();
        m_programPipeline = other.m_programPipeline;
        other.invalidate_();
        return *this;
    }
    operator const GLuint&() const { return m_programPipeline; }

private:
    void release_() {
        if (m_programPipeline) {
            glDeleteProgramPipelines(1, &m_programPipeline);
        }
    };
    void   invalidate_() { m_programPipeline = 0; }
    GLuint m_programPipeline = 0;
};

}; // namespace glraii