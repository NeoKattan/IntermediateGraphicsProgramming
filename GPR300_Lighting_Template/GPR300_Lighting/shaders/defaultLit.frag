#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

struct DirLight{
    vec3 color;
    vec3 direction;
    float intensity;
};

struct PtLight{
    vec3 color;
    vec3 position;
    float intensity;
    float linearAtt;
};

struct SpLight{
    vec3 color;
    vec3 position;
    vec3 direction;
    float intensity;
    float linearAtt;
    float minAngle;
    float maxAngle;
};

#define MAX_LIGHTS 8
uniform DirLight _DirLight[MAX_LIGHTS];
uniform PtLight _PtLight[MAX_LIGHTS];
uniform SpLight _SpLight[MAX_LIGHTS];

vec3 ambient;
vec3 diffuse;
vec3 specular;

uniform struct Material{
    vec3 color;
    float ambientK;
    float diffuseK;
    float specularK; 
    float shininess; 
}_Material;

void main(){      
    

    vec3 col = ambient + diffuse + specular;
    col *= _Material.color;
    FragColor = vec4(col,1.0f);
}
