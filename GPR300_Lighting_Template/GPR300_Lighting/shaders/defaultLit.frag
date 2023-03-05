#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

uniform vec3 _CameraPos;

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
    float falloffCurve;
};

#define MAX_LIGHTS 8
uniform DirLight _DirLight[MAX_LIGHTS];
uniform PtLight _PtLight[MAX_LIGHTS];
uniform SpLight _SpLight[MAX_LIGHTS];
uniform int numDirLights, numPtLights, numSpLights;

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
    ambient = _Material.ambientK * _Material.color;

    diffuse = vec3(0);
    specular = vec3(0);

    //Directional Lights
    for(int i = 0; i < numDirLights; i++) {
        vec3 l = normalize(_DirLight[i].direction * -1);

        diffuse += _Material.diffuseK * max(dot(l, v_out.WorldNormal), 0) * (_DirLight[i].intensity * _DirLight[i].color);

        vec3 v = _CameraPos - v_out.WorldPosition;
        vec3 h = normalize(v + l);

        specular += _Material.specularK * pow(dot(v_out.WorldNormal, h), _Material.shininess) * (_DirLight[i].intensity * _DirLight[i].color);
    }

    //Point Lights
    for(int i = 0; i < numPtLights; i++) {
        float linearAtt = length(_PtLight[i].position - v_out.WorldPosition) / _PtLight[i].linearAtt;
        linearAtt = 1 - pow(linearAtt, 4);
        linearAtt = min(max(linearAtt, 0), 1);
        linearAtt = pow(linearAtt, 2);

        vec3 l = normalize(_PtLight[i].position - v_out.WorldPosition);

        diffuse += _Material.diffuseK * max(dot(l, v_out.WorldNormal), 0) * (_PtLight[i].intensity * linearAtt * _PtLight[i].color);

        vec3 v = _CameraPos - v_out.WorldPosition;
        vec3 h = normalize(v + l);

        specular += _Material.specularK * pow(dot(v_out.WorldNormal, h), _Material.shininess) * (_PtLight[i].intensity * linearAtt * _PtLight[i].color);
    }

    //Spot Lights
    for(int i = 0; i < numSpLights; i++) {

        vec3 dtofrag = normalize(v_out.WorldPosition - _SpLight[i].position);
        float theta = dot(dtofrag, _SpLight[i].direction);
        theta = min(max(theta, 0), 1);

        float angularAtt = (theta - _SpLight[i].maxAngle) / (_SpLight[i].minAngle - _SpLight[i].maxAngle);
        angularAtt = min(max(angularAtt, 0), 1);
        angularAtt = pow(angularAtt ,_SpLight[i].falloffCurve);

        float linearAtt = length(_PtLight[i].position - v_out.WorldPosition) / _PtLight[i].linearAtt;
        linearAtt = 1 - pow(linearAtt, 4);
        linearAtt = min(max(linearAtt, 0), 1);
        linearAtt = pow(linearAtt, 2);

        vec3 l = normalize(_SpLight[i].position - v_out.WorldPosition);

        diffuse += _Material.diffuseK * max(dot(l, v_out.WorldNormal), 0) * (_SpLight[i].intensity * angularAtt * linearAtt * _SpLight[i].color);

        vec3 v = _CameraPos - v_out.WorldPosition;
        vec3 h = normalize(v + l);

        specular += _Material.specularK * pow(dot(v_out.WorldNormal, h), _Material.shininess) * (_SpLight[i].intensity * angularAtt * linearAtt * _SpLight[i].color);
    }

    vec3 lightCol = ambient + diffuse + specular;
    vec3 col = _Material.color * lightCol;
    FragColor = vec4(col,1.0f);
}
