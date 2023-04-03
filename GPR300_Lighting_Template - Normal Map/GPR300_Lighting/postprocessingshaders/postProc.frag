#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec2 Uv;
}v_out;

uniform sampler2D _FrameBuffer;

void main(){
    vec3 col = texture(_FrameBuffer, v_out.Uv).rgb;

    //inverted colors
    //col = vec3(1) - col;

    //blue scale
    //col.x = 0;
    //col.y = 0;

    //vignette
    float xDis = abs(v_out.Uv.x - 0.5);
    float yDis = abs(v_out.Uv.y - 0.5);
    float distanceFromCenter = sqrt(xDis * xDis + yDis * yDis);
    col -= distanceFromCenter * distanceFromCenter * distanceFromCenter;

    //normalize colors
    //normalize(col);


    FragColor = vec4(col,1.0f);
}
