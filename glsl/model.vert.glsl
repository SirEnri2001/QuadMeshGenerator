#version 330 core
layout (location = 0) in vec4 vPos;
layout (location = 1) in vec4 vNor;

uniform mat4 uModel;
uniform mat3 uModelInvTr; // The inverse transpose of the model matrix.
uniform mat4 uProjView;
uniform vec3 uLightPos;

out vec3 nor;
out vec3 lightDir;

void main()
{
    vec4 modelPos = uModel * vPos;
    lightDir = uLightPos - vec3(modelPos);

    
    nor = normalize(uModelInvTr * vec3(vNor));

    gl_Position = uProjView * modelPos;
}