#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 17) out;
in vec4 gTangent[];
in vec4 nor[];
uniform float thickness = 0.01;
uniform float offset = 0.001;
uniform mat4 uModel;
uniform mat4 uProjView;
uniform vec4 uFieldColor = vec4(0,0,1,1);
uniform float uFieldScale = 10;
uniform float uFieldThickness = 0.01;
out vec4 col;

void main()
{
    vec4 p = gl_in[0].gl_Position;
    vec4 normal = nor[0];
    vec4 tangent = gTangent[0];
    vec4 cotangent = vec4(normalize(cross(vec3(normal), vec3(tangent))),0);
    vec4 coords[17];
    float thickness = uFieldThickness;
    coords[0] = p + tangent*uFieldScale+offset*normal;
    coords[1] = p + offset*normal;
    coords[2] = p + tangent*thickness - cotangent*thickness+offset*normal;
    coords[3] = p + offset*normal;
    coords[4] = p - cotangent*uFieldScale+offset*normal;
    coords[5] = p + offset*normal;
    coords[6] = p - tangent*thickness - cotangent*thickness+offset*normal;
    coords[7] = p + offset*normal;
    coords[8] = p - tangent*uFieldScale+offset*normal;
    coords[9] = p + offset*normal;
    coords[10] = p - tangent*thickness + cotangent*thickness+offset*normal;
    coords[11] = p + offset*normal;
    coords[12] = p + cotangent*uFieldScale+offset*normal;
    coords[13] = p + offset*normal;
    coords[14] = p + tangent*thickness + cotangent*thickness+offset*normal;
    coords[15] = p + offset*normal;
    coords[16] = p + tangent*uFieldScale+offset*normal;
    for(int i = 0;i<17;i++) {
        gl_Position = uProjView * uModel * coords[i ];
    col = uFieldColor;
    EmitVertex();
    }
    EndPrimitive();
}

// #version 330

// layout (points) in;
// layout (triangle_strip, max_vertices = 8) out;

// in vec4 gTangent[];
// in vec4 nor[];

// uniform float thickness = 0.0001;
// uniform float offset = 0.001;
// uniform mat4 uModel;
// uniform mat4 uProjView;
// uniform float uFieldScale = 10;

// out vec4 col;

// void main()
// {
//     vec4 p = gl_in[0].gl_Position;
//     vec4 normal = nor[0];
//     vec4 tangent = gTangent[0];
//     vec4 cotangent = vec4(normalize(cross(vec3(normal), vec3(tangent))),0);

//     vec4 coords[8];
//     float thickness = 0.1;
//     float offset = 1000;
//     coords[0] = p + tangent*uFieldScale+offset*normal;
//     coords[1] = p + tangent*thickness - cotangent*thickness+offset*normal;
    
//     coords[2] = p - cotangent*uFieldScale+offset*normal;
//     coords[3] = p - tangent*thickness - cotangent*thickness+offset*normal;
    
//     coords[4] = p - tangent*uFieldScale+offset*normal;
//     coords[5] = p - tangent*thickness + cotangent*thickness+offset*normal;
    
//     coords[6] = p + cotangent*uFieldScale+offset*normal;
//     coords[7] = p + tangent*thickness + cotangent*thickness+offset*normal;

//     gl_Position = uProjView * uModel * coords[0];
//     col = uFieldColor;
//     EmitVertex();
//     gl_Position = uProjView * uModel * coords[1];
//     col = uFieldColor;
//     EmitVertex();
//     gl_Position = uProjView * uModel * coords[2];
//     col = uFieldColor;
//     EmitVertex();
//     gl_Position = uProjView * uModel * coords[3];
//     col = uFieldColor;
//     EmitVertex();
//     gl_Position = uProjView * uModel * coords[4];
//     col = uFieldColor;
//     EmitVertex();
//     gl_Position = uProjView * uModel * coords[5];
//     col = uFieldColor;
//     EmitVertex();
//     gl_Position = uProjView * uModel * coords[6];
//     col = uFieldColor;
//     EmitVertex();
//     gl_Position = uProjView * uModel * coords[7];
//     col = uFieldColor;
//     EmitVertex();
//     EndPrimitive();
// }