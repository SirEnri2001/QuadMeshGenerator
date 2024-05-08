#version 330 core
layout (location = 0) in vec4 vPos;
layout (location = 1) in vec4 vTangent;
layout (location = 2) in vec4 vNor;

uniform float uSize;
uniform mat4 uModel;
uniform mat4 uProjView;

out vec4 gTangent;
out vec4 nor;


void main()
{
    gl_Position = vPos;
    gl_PointSize = uSize;
    gTangent = vTangent;
    nor = vNor;
}