#version 460 core
layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord0;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec4 vertexTangent;
out vec3 interpVertexPosition;
out vec2 interpVertexTexCoord0;
out vec3 interpVertexNormal;
out vec4 interpVertexTangent;
void main()
{
    interpVertexPosition = vertexPosition.xyz;
    interpVertexTexCoord0 = vertexTexCoord0;
    interpVertexNormal = vertexNormal;
    interpVertexTangent = vertexTangent;
    gl_Position = vertexPosition;
}
