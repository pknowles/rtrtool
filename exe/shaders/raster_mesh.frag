#version 460 core

// Face normal from geometry shader
flat in vec3 triangleNormal;

// Material values
uniform vec4  color;
uniform float metallic;
uniform float roughness;
uniform int       hasColorTexture;
uniform int       hasMetallicTexture;
uniform int       hasRoughnessTexture;
uniform int       hasNormalTexture;
uniform sampler2D colorTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalTexture;

uniform vec3 lightDir;

// Vertex attributes
in vec3  interpPosition;
in vec2  interpTexCoord0;
in vec3  interpNormal;
in vec4  interpTangent;
out vec4 fragColor;

vec2 LightingFuncGGX_FV(float dotLH, float roughness) {
    float alpha = roughness * roughness;

    // F
    float F_a, F_b;
    float dotLH5 = pow(1.0f - dotLH, 5);
    F_a = 1.0f;
    F_b = dotLH5;

    // V
    float vis;
    float k = alpha / 2.0f;
    float k2 = k * k;
    float invK2 = 1.0f - k2;
    vis = 1.0f / (dotLH * dotLH * invK2 + k2);

    return vec2(F_a * vis, F_b * vis);
}

float LightingFuncGGX_D(float dotNH, float roughness) {
    float alpha = roughness * roughness;
    float alphaSqr = alpha * alpha;
    float pi = 3.14159f;
    float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0f;

    float D = alphaSqr / (pi * denom * denom);
    return D;
}

// http://filmicworlds.com/blog/optimizing-ggx-shaders-with-dotlh/ - John Hable
float LightingFuncGGX_OPT3(vec3 N, vec3 V, vec3 L, float roughness, float F0) {
    vec3 H = normalize(V + L);

    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

    float D = LightingFuncGGX_D(dotNH, roughness);
    vec2  FV_helper = LightingFuncGGX_FV(dotLH, roughness);
    float FV = F0 * FV_helper.x + (1.0f - F0) * FV_helper.y;
    float specular = dotNL * D * FV;

    return specular;
}

void main() {
    vec4  colorSample = color;
    float  metallicSample = metallic;
    float  roughnessSample = roughness;
    vec3  normalSample = vec3(0, 0, 1);
    if(hasColorTexture != 0) colorSample *= texture(colorTexture, interpTexCoord0);
    if(hasMetallicTexture != 0) metallicSample *= texture(metallicTexture, interpTexCoord0).x;
    if(hasRoughnessTexture != 0) roughnessSample *= texture(roughnessTexture, interpTexCoord0).x;
    if(hasNormalTexture != 0) normalSample = texture(normalTexture, interpTexCoord0).xyz * 2.0 - 1.0;
    mat3 TBN = mat3(interpTangent.xyz, cross(interpNormal, interpTangent.xyz) * interpTangent.w, interpNormal);
    vec3 L = normalize(lightDir);
    vec3 N = normalize(TBN * normalSample);
    vec3 V = normalize(-interpPosition);
    float F0 = mix(0.02, 1.0, metallicSample);
    float specular = LightingFuncGGX_OPT3(N, V, L, roughnessSample, F0);
    float diffuse = dot(N, L) * (1.0 - metallicSample);  // ???
    vec3  albedo = colorSample.xyz;
    fragColor = vec4(albedo * (diffuse + specular), colorSample.w);
}
