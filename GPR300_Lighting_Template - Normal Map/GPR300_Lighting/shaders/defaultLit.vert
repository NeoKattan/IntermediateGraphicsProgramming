#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 vTangent;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

out struct Vertex{
    vec3 WorldPosition;
    vec2 Uv;
    mat3 TBN;
}v_out;

uniform bool Scrolling;
uniform float Time;

void main(){    
    v_out.WorldPosition = vec3(_Model * vec4(vPos,1));
    vec3 worldNormal = transpose(inverse(mat3(_Model))) * vNormal;
    vec3 worldTangent = transpose(inverse(mat3(_Model))) * vTangent;
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);

    v_out.TBN = mat3(worldTangent, cross(worldTangent, worldNormal), worldNormal);

    if(Scrolling) {
        vec2 temp = uv;
        temp.y += mod(Time,1);
        if(temp.y > 1) {
            temp.y--;
        }
        v_out.Uv = temp;
    }
    else {
        v_out.Uv = uv;
    }
}
