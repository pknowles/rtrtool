// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

class OrbitCamera {
public:
    OrbitCamera(float distance = 1.0f, float yaw = glm::radians(45.0f),
                float pitch = -glm::radians(45.0f))
        : m_orbitDistance(distance),
          m_orbitAngles(pitch, 0.0f, yaw) {}
    glm::mat4 worldToEye() const {
        glm::mat4 result = glm::identity<glm::mat4>();
        result = glm::translate(result, m_origin);
        result *= glm::orientate4(m_orbitAngles);
        result = glm::translate(result, glm::vec3(0.0f, 0.0f, m_orbitDistance));
        return glm::inverse(result);
    }
    glm::vec3 forward() const {
        return glm::mat3(glm::orientate4(m_orbitAngles)) * glm::vec3(0.0f, 0.0f, -1.0f);
    }
    glm::vec3 right() const {
        return glm::mat3(glm::orientate4(m_orbitAngles)) * glm::vec3(1.0f, 0.0f, 0.0f);
    }
    glm::vec3 up() const {
        return glm::mat3(glm::orientate4(m_orbitAngles)) * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    void yaw(float v) {
        m_orbitAngles.z += v * rotateSpeed();
        m_orbitAngles.z -= glm::two_pi<float>() * floorf(m_orbitAngles.z / glm::two_pi<float>());
    }
    void pitch(float v) {
        m_orbitAngles.x += v * rotateSpeed();
        float limit = glm::half_pi<float>() + glm::epsilon<float>();
        m_orbitAngles.x = glm::clamp(m_orbitAngles.x, -limit, limit);
    }
    void roll(float v) {
        m_orbitAngles.y += v * rotateSpeed();
        float limit = glm::half_pi<float>() + glm::epsilon<float>();
        m_orbitAngles.y = glm::clamp(m_orbitAngles.y, -limit, limit);
    }
    void pan(float x, float y) {
        m_origin +=
            glm::mat3(glm::orientate4(m_orbitAngles)) * (glm::vec3(x, y, 0.0f) * moveSpeed());
    }
    void fly(float x, float y) {
        m_origin +=
            glm::mat3(glm::orientate4(m_orbitAngles)) * (glm::vec3(x, 0.0f, -y) * moveSpeed());
    }
    void zoom(float distance) {
        // TODO: speed() is m_orbitDistance dependent. Should be integrating
        m_orbitDistance = glm::max(m_orbitDistance + distance * moveSpeed(), glm::epsilon<float>());
    }

private:
    float moveSpeed() const { return m_orbitDistance; }
    float rotateSpeed() const { return 1.0f; }

    glm::vec3 m_origin = glm::vec3(0.0f);
    float     m_orbitDistance = 0.0f;
    glm::vec3 m_orbitAngles = glm::vec3(0.0f);
};

struct PerspectiveProjection {
    float     fovY = glm::radians(80.0f);
    float     aspect = 1.0f;
    float     near = 0.1f;
    float     far = 100.0f;
    glm::mat4 matrix() const { return glm::perspective(fovY, aspect, near, far); }
};
