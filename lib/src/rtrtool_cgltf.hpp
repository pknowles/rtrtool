// Copyright (c) 2024 Pyarelal Knowles, MIT License

#pragma once

#include <cgltf.h>
#include <cstddef>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ranges>
#include <vector>
#include <stdexcept>

namespace rtrtool {

static const char* cgltfErrorString(cgltf_result result, cgltf_data* data) {
    switch (result) {
    case cgltf_result_file_not_found:
        return data ? "resource not found" : "file not found";

    case cgltf_result_io_error:
        return "I/O error";

    case cgltf_result_invalid_json:
        return "invalid JSON";

    case cgltf_result_invalid_gltf:
        return "invalid GLTF";

    case cgltf_result_out_of_memory:
        return "out of memory";

    case cgltf_result_legacy_gltf:
        return "legacy GLTF";

    case cgltf_result_data_too_short:
        return data ? "buffer too short" : "not a GLTF file";

    case cgltf_result_unknown_format:
        return data ? "unknown resource format" : "not a GLTF file";

    default:
        return "unknown error";
    }
}

// Adaptors for cgltf_accessor
constexpr char const* cgltfTypeName(cgltf_component_type component_type, cgltf_type type) {
#define ROW(c)                                                                                     \
    { "vec2_" c, "vec3_" c, "vec4_" c, "mat2_" c, "mat3_" c, "mat4_" c, "scalar_" c }
    constexpr char const* table[8][7] = {ROW("i8"),   ROW("ui8"),  ROW("i16"),
                                         ROW("ui16"), ROW("ui32"), ROW("f32")};

    int typeIndex = 0;
    switch (type) {
    case cgltf_type_vec2:
        typeIndex = 0;
        break;
    case cgltf_type_vec3:
        typeIndex = 1;
        break;
    case cgltf_type_vec4:
        typeIndex = 2;
        break;
    case cgltf_type_mat2:
        typeIndex = 3;
        break;
    case cgltf_type_mat3:
        typeIndex = 4;
        break;
    case cgltf_type_mat4:
        typeIndex = 5;
        break;
    case cgltf_type_scalar:
        typeIndex = 6;
        break;
    default:
        return "unknown";
    }

    switch (component_type) {
    case cgltf_component_type_r_8:
        return table[0][typeIndex];
    case cgltf_component_type_r_8u:
        return table[1][typeIndex];
    case cgltf_component_type_r_16:
        return table[2][typeIndex];
    case cgltf_component_type_r_16u:
        return table[3][typeIndex];
    case cgltf_component_type_r_32u:
        return table[4][typeIndex];
    case cgltf_component_type_r_32f:
        return table[5][typeIndex];
    default:
        return "unknown";
    }
}

// clang-format off
template <class T> struct cgltf_type_traits      { static constexpr cgltf_component_type component_type = cgltf_component_type_invalid; static constexpr cgltf_type type = cgltf_type_invalid; static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<int8_t>     { static constexpr cgltf_component_type component_type = cgltf_component_type_r_8;     static constexpr cgltf_type type = cgltf_type_scalar;  static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<uint8_t>    { static constexpr cgltf_component_type component_type = cgltf_component_type_r_8u;    static constexpr cgltf_type type = cgltf_type_scalar;  static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<int16_t>    { static constexpr cgltf_component_type component_type = cgltf_component_type_r_16;    static constexpr cgltf_type type = cgltf_type_scalar;  static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<uint16_t>   { static constexpr cgltf_component_type component_type = cgltf_component_type_r_16u;   static constexpr cgltf_type type = cgltf_type_scalar;  static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<uint32_t>   { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32u;   static constexpr cgltf_type type = cgltf_type_scalar;  static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<float>      { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32f;   static constexpr cgltf_type type = cgltf_type_scalar;  static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<glm::vec2>  { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32f;   static constexpr cgltf_type type = cgltf_type_vec2;    static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<glm::uvec2> { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32u;   static constexpr cgltf_type type = cgltf_type_vec2;    static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<glm::vec3>  { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32f;   static constexpr cgltf_type type = cgltf_type_vec3;    static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<glm::uvec3> { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32u;   static constexpr cgltf_type type = cgltf_type_vec3;    static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<glm::vec4>  { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32f;   static constexpr cgltf_type type = cgltf_type_vec4;    static constexpr char const* name = cgltfTypeName(component_type, type); };
template <> struct cgltf_type_traits<glm::uvec4> { static constexpr cgltf_component_type component_type = cgltf_component_type_r_32u;   static constexpr cgltf_type type = cgltf_type_vec4;    static constexpr char const* name = cgltfTypeName(component_type, type); };

template <cgltf_component_type component_type, cgltf_type type> struct cgltf_type_traits_inv { using element_type = void; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_8,   cgltf_type_scalar>      { using element_type = int8_t; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_8u,  cgltf_type_scalar>      { using element_type = uint8_t; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_16,  cgltf_type_scalar>      { using element_type = int16_t; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_16u, cgltf_type_scalar>      { using element_type = uint16_t; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32u, cgltf_type_scalar>      { using element_type = uint32_t; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32f, cgltf_type_scalar>      { using element_type = float; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32f, cgltf_type_vec2>        { using element_type = glm::vec2; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32u, cgltf_type_vec2>        { using element_type = glm::uvec2; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32f, cgltf_type_vec3>        { using element_type = glm::vec3; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32u, cgltf_type_vec3>        { using element_type = glm::uvec3; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32f, cgltf_type_vec4>        { using element_type = glm::vec4; };
template <> struct cgltf_type_traits_inv<cgltf_component_type_r_32u, cgltf_type_vec4>        { using element_type = glm::uvec4; };
// clang-format on

inline glm::mat4 cgltfTransform(const cgltf_node& node) {
    glm::mat4 result = glm::identity<glm::mat4>();
    if (node.has_matrix) {
        std::ranges::copy(node.matrix, glm::value_ptr(result));
    } else {
        if (node.has_rotation)
            result =
                glm::mat4_cast(glm::quat{node.rotation[3] /* <- not a typo ffs */, node.rotation[0],
                                         node.rotation[1], node.rotation[2]});
        if (node.has_translation)
            result[3] =
                glm::vec4(node.translation[0], node.translation[1], node.translation[2], 1.0f);
        // TODO: verify scale order?
        if (node.has_scale)
            result = glm::scale(result, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    }
    return result;
}

template <class T>
class strided_iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    strided_iterator(T* data, size_type stride)
        : m_data(data),
          m_stride(stride) {}
    T&                operator*() { return *m_data; }
    strided_iterator& operator++() {
        m_data = &(*this)[1];
        return *this;
    }
    strided_iterator operator++(int) {
        strided_iterator tmp = *this;
        ++*this;
        return tmp;
    }
    T& operator[](difference_type n) const {
        return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(m_data) + (m_stride * n));
    }
    strided_iterator& operator+=(difference_type n) {
        m_data = &(*this)[n];
        return *this;
    }
    strided_iterator operator+(difference_type n) const {
        strided_iterator result(*this);
        result += n;
        return result;
    }
    strided_iterator& operator-=(difference_type n) {
        m_data = &(*this)[-n];
        return *this;
    }
    strided_iterator operator-(difference_type n) const {
        strided_iterator result(*this);
        result -= n;
        return result;
    }
    difference_type operator-(const strided_iterator& other) const {
        return difference_type(other.m_data - m_data) / m_stride;
    }
    bool operator==(const strided_iterator& other) const { return m_data == other.m_data; }
    bool operator!=(const strided_iterator& other) const { return m_data != other.m_data; }

private:
    T*        m_data;
    size_type m_stride;
};

template <class T>
class cgltf_accessor_adapter {
public:
    using iterator = strided_iterator<T>;
    cgltf_accessor_adapter(const cgltf_accessor& accessor)
        : m_data(reinterpret_cast<T*>(
              reinterpret_cast<std::byte*>(accessor.buffer_view->buffer->data) +
              accessor.buffer_view->offset + accessor.offset)),
          m_size(accessor.count),
          m_stride(accessor.buffer_view->stride ? accessor.buffer_view->stride : accessor.stride) {}

    T*       data() const { return m_data; }
    size_t   size() const { return m_size; }
    size_t   stride() const { return m_stride; }
    bool     tight() const { return sizeof(T) == m_stride; }
    iterator begin() const { return iterator(m_data, m_stride); }
    iterator end() const { return iterator(m_data, m_stride) + m_size; }

private:
    T*     m_data;
    size_t m_size;
    size_t m_stride;
};

template<cgltf_component_type component_type, cgltf_type type, class U, std::ranges::output_range<U> Range>
void convertCTT(const cgltf_accessor& accessor, Range& result)
{
    using T = typename cgltf_type_traits_inv<component_type, type>::element_type;
    if constexpr(std::is_same_v<T, void>)
    {
        throw std::runtime_error("Unsupported cgltf accessor component_type and type combination");
    }
    else if  constexpr(!std::is_convertible_v<T, U>)
    {
        std::string msg = std::string("Cannot convert cgltf ") + cgltf_type_traits<T>::name + " to " + cgltf_type_traits<U>::name;
        throw std::runtime_error(msg);
    }
    else
    {
        cgltf_accessor_adapter<T> adapter(accessor);
        auto out = result.begin();
        for(const T& v : adapter)
            *out++ = static_cast<U>(v);
        //std::ranges::transform(adapter, result.begin(), [](const T& v){ return static_cast<U>(v); });
    }
}

template<cgltf_component_type component_type, class U, std::ranges::output_range<U> Range>
void convertCT(const cgltf_accessor& accessor, Range& result)
{
    // clang-format off
    switch(accessor.type)
    {
    case cgltf_type_scalar: convertCTT<component_type, cgltf_type_scalar, U, Range>(accessor, result); break;
    case cgltf_type_vec2:   convertCTT<component_type, cgltf_type_vec2,   U, Range>(accessor, result); break;
    case cgltf_type_vec3:   convertCTT<component_type, cgltf_type_vec3,   U, Range>(accessor, result); break;
    case cgltf_type_vec4:   convertCTT<component_type, cgltf_type_vec4,   U, Range>(accessor, result); break;
    case cgltf_type_mat2:   convertCTT<component_type, cgltf_type_mat2,   U, Range>(accessor, result); break;
    case cgltf_type_mat3:   convertCTT<component_type, cgltf_type_mat3,   U, Range>(accessor, result); break;
    case cgltf_type_mat4:   convertCTT<component_type, cgltf_type_mat4,   U, Range>(accessor, result); break;
    default: throw std::runtime_error("Invalid cgltf accessor type");
    }
    // clang-format on
}

template <class T, class U = T>
    requires(alignof(T) == alignof(U))
std::span<U> convert(const cgltf_accessor& accessor, std::vector<std::remove_cv_t<U>>& temporary) {
    static_assert(std::max(sizeof(T), sizeof(U)) % std::min(sizeof(T), sizeof(U)) == 0);
    std::span<U>              result;
    cgltf_accessor_adapter<T> adapter(accessor);

    // Check for happy path
    if (cgltf_type_traits<T>::component_type == accessor.component_type &&
        cgltf_type_traits<T>::type == accessor.type && adapter.tight()) {
        std::span<T> spanT = {adapter.data(), adapter.size()};
        result = {reinterpret_cast<U*>(spanT.data()), (spanT.size() * sizeof(T)) / sizeof(U)};
    } else {
        // Convert using 'temporary' storage
        temporary.resize((adapter.size() * sizeof(T)) / sizeof(U));
        std::span<T> spanT = {reinterpret_cast<T*>(temporary.data()), adapter.size()};
        // clang-format off
        switch (accessor.component_type) {
        case cgltf_component_type_r_8:   convertCT<cgltf_component_type_r_8,   T>(accessor, spanT); break;
        case cgltf_component_type_r_8u:  convertCT<cgltf_component_type_r_8u,  T>(accessor, spanT); break;
        case cgltf_component_type_r_16:  convertCT<cgltf_component_type_r_16,  T>(accessor, spanT); break;
        case cgltf_component_type_r_16u: convertCT<cgltf_component_type_r_16u, T>(accessor, spanT); break;
        case cgltf_component_type_r_32u: convertCT<cgltf_component_type_r_32u, T>(accessor, spanT); break;
        case cgltf_component_type_r_32f: convertCT<cgltf_component_type_r_32f, T>(accessor, spanT); break;
        default: throw std::runtime_error("Invalid cgltf accessor component_type");
        }
        // clang-format on
        result = temporary;
    }
    return result;
}

} // namespace rtrtool
