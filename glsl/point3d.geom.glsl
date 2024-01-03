#version 330

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 gCol[];
in vec4 nor[];

uniform float thickness = 0.0001;
uniform float offset = 0.001;
uniform mat4 uModel;
uniform mat4 uProjView;

out vec4 col;

void main()
{
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;
    vec4 normal = (nor[0]+nor[1])/2.0;
    vec4 tangent = vec4(normalize(cross(vec3(normal), vec3(p2-p1))),0);

    vec4 coords[4];
    coords[0] = p1 + tangent*thickness+offset*normal;
    coords[1] = p1 - tangent*thickness+offset*normal;
    coords[2] = p2 + tangent*thickness+offset*normal;
    coords[3] = p2 - tangent*thickness+offset*normal;

    gl_Position = uProjView * uModel * coords[0];
    col = gCol[0];
    EmitVertex();
    gl_Position = uProjView * uModel * coords[1];
    col = gCol[0];
    EmitVertex();
    gl_Position = uProjView * uModel * coords[2];
    col = gCol[1];
    EmitVertex();
    gl_Position = uProjView * uModel * coords[3];
    col = gCol[1];
    EmitVertex();
    EndPrimitive();
}