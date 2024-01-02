#version 330 core
layout (location = 0) in vec4 vPos;
layout (location = 1) in vec4 vCol;

uniform float uSize;
uniform mat4 uModel;
uniform mat4 uProjView;
out vec4 col;
void main()
{
    gl_Position = uProjView * uModel * vPos;
    gl_PointSize = uSize;
}