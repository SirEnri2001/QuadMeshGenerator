#version 330 core
in vec4 col;
out vec4 fColor;

uniform vec3 uColor;
uniform int markCount;
uniform int maxDisplay;

void main()
{
    //float coundIndex = min(col[3], markCount);
    //fColor = mix(vec4(vec3(col),1), vec4(0,0,0,1),1.0-coundIndex/markCount);
    fColor = vec4(vec3(col),1);
} 