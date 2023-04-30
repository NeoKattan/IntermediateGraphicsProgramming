#version 450

out vec4 FragColor;

uniform vec3 _Color;
uniform bool _OtlnShader;

void main(){
    float a = 1.0f;
    vec3 color = vec3(0);

    if(_OtlnShader)
    {
       color = vec3(0);
    }
    else
    {
       color = _Color;
    }

    FragColor = vec4(color, a);
}
