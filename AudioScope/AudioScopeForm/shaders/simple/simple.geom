#version 330

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 5) out;

in vec3 color[];

out vec3 fColor;

void main() {    
   fColor = color[0]; // gs_in[0] since there's only one input vertex
   gl_Position = gl_in[0].gl_Position;
   EmitVertex();
   gl_Position = gl_in[1].gl_Position;
   EmitVertex();
   gl_Position = gl_in[2].gl_Position;
   EmitVertex();
   EndPrimitive();
}  
