#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 uv;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

out struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
    vec3 Eye;
    vec2 Uv;
}v_out;

uniform bool Scrolling;
uniform float Time;

void main(){    
    v_out.WorldPosition = vec3(_Model * vec4(vPos,1));

    vec3 viewPosition = vec3((_View * _Model) * vec4(vPos,1));

    v_out.Eye = normalize(-viewPosition);//Quincy Code

    v_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);

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
