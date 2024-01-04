#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;
in vec4 gCol[];
in vec4 nor[];
uniform float thickness = 0.01;
uniform float offset = 0.001;
uniform mat4 uModel;
uniform mat4 uProjView;
out vec4 col;

void main()
{
    vec4 p1 = gl_in[0].gl_Position;
    //vec4 normal = nor[0];
    //vec4 tangent = vec4(normalize(cross(vec3(normal), vec3(p2-p1))),0);
    //vec4 direction = vec4(normalize(vec3(p2-p1)), 0);
    vec4 normal = nor[0];
    vec4 tangent = vec4(normalize(cross(vec3(normal),vec3(3,2,1))),0);
    vec4 direction = vec4(normalize(cross(vec3(normal),vec3(tangent))),0);
    vec4 coords[4];
    coords[0] = p1 - tangent  *thickness + normal * offset*0.5;
    coords[1] = p1 + direction*thickness + normal * offset*0.5;
    coords[2] = p1 + tangent  *thickness + normal * offset*0.5;
    coords[3] = p1 - direction*thickness + normal * offset*0.5;
    gl_Position = uProjView * uModel * coords[0];
    col = gCol[0];
    EmitVertex();
    gl_Position = uProjView * uModel * coords[1];
    col = gCol[0];
    EmitVertex();
    gl_Position = uProjView * uModel * coords[2];
    col = gCol[0];
    EmitVertex();
    gl_Position = uProjView * uModel * coords[3];
    col = gCol[0];
    EmitVertex();
    gl_Position = uProjView * uModel * coords[0];
    col = gCol[0];
    EmitVertex();
    EndPrimitive();
}