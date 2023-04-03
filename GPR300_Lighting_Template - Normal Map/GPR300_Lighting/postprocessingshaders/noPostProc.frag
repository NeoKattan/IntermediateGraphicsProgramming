#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec2 Uv;
}v_out;

uniform sampler2D _FrameBuffer;

void main(){
    vec3 col = texture(_FrameBuffer, v_out.Uv).rgb;
    FragColor = vec4(col,1.0f);
}
