#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 vTangent;

out struct Vertex{
    vec2 Uv;
}v_out;

void main(){    
    v_out.Uv = uv;
    gl_Position = vec4(vPos, 1);
}
