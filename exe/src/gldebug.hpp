// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <glad/gl.h>

inline std::string getGLDebugSourceStr(GLenum source) {
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "WINDOW SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "SHADER COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "THIRD PARTY";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "APPLICATION";
    case GL_DEBUG_SOURCE_OTHER:
        return "OTHER";
    default:
        return "UNKNOWN:" + std::to_string(source);
    }
}

inline std::string getGLDebugTypeStr(GLenum type) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        return "ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "DEPRECATED BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "UDEFINED BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "PERFORMANCE";
    case GL_DEBUG_TYPE_MARKER:
        return "MARKER";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "PUSH_GROUP";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "POP_GROUP";
    case GL_DEBUG_TYPE_OTHER:
        return "OTHER";
    default:
        return "UNKNOWN:" + std::to_string(type);
    }
}

inline std::string getGLDebugSeverityStr(GLenum severity) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        return "HIGH";
    case GL_DEBUG_SEVERITY_MEDIUM:
        return "MEDIUM";
    case GL_DEBUG_SEVERITY_LOW:
        return "LOW";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "NOTIFICATION";
    default:
        return "UNKNOWN:" + std::to_string(severity);
    }
}

inline void defaultDebugCallbackPrintStderr(GLenum source, GLenum type, GLuint id, GLenum severity,
                                            GLsizei length, const GLchar* message,
                                            const void* userParam) {
    (void)length;
    (void)userParam;
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;
    fprintf(stderr, "GL Message: source %s type %s id %x severity %s '%s'\n",
            getGLDebugSourceStr(source).c_str(), getGLDebugTypeStr(type).c_str(), id,
            getGLDebugSeverityStr(severity).c_str(), message);
}
