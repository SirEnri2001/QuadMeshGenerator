#version 330 core
in vec3 nor;
in vec3 lightDir;
in vec4 col;

layout(location = 0) out vec4 fCol;

uniform vec3 uColor = vec3(0.7, 0.7, 0.6);
uniform float uAlpha = 1.0;

void main()
{
    float diffuseTerm = dot(normalize(nor), normalize(lightDir));
    diffuseTerm = clamp(diffuseTerm, 0, 1);
    float ambientTerm = 0.1;

    fCol = vec4(vec3(col) * (diffuseTerm), uAlpha);
    //fCol = vec4(0.5 * (nor + 1), 1.0);
    //fCol = col;
} 