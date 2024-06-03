#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
uniform mat4  modelView;
uniform mat4  modelViewProjection;
uniform mat3  normalMatrix;
in vec3 interpVertexPosition[];
in vec2 interpVertexTexCoord0[];
in vec3 interpVertexNormal[];
in vec4 interpVertexTangent[];
out vec3 interpPosition;
out vec2 interpTexCoord0;
out vec3 interpNormal;
out vec4 interpTangent;
flat out vec3 triangleNormal;
void          main() {
    vec3 e0 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 e1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 faceNormal = normalize(normalMatrix * cross(e1, e0));
    for (int i = 0; i < 3; ++i) {
        triangleNormal = faceNormal;
        // TODO: multiply in vertex shader?
        interpPosition = vec3(modelView * vec4(interpVertexPosition[i], 1.0));
        interpTexCoord0 = interpVertexTexCoord0[i];
        interpNormal = normalMatrix * interpVertexNormal[i];
        interpTangent = vec4(normalMatrix * interpVertexTangent[i].xyz, interpVertexTangent[i].w);
        gl_Position = modelViewProjection * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
