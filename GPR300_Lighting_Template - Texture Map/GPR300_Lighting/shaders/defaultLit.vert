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
    vec3 ViewSpaceNormal;
    vec3 Eye;
    vec2 Uv;
}v_out;

uniform bool Scrolling;
uniform float Time;

void main(){    
    v_out.WorldPosition = vec3(_Model * vec4(vPos,1));

    //Quincy
    //*********************
    v_out.ViewSpaceNormal = normalize(mat3(_View) * vNormal); //viewSapce Normal
    vec3 viewSpacePos = vec3(_View * vec4(vPos,1)); //viewSpace Position
    v_out.Eye = normalize(-viewSpacePos);//vector towards cam/eye
    //********************************

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
