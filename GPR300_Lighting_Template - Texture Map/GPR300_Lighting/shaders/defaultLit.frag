#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
    vec3 Eye;
    vec2 Uv;
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
vec3 RimColor;//Quincy Code

uniform struct Material{
    vec3 color;
    float ambientK;
    float diffuseK;
    float specularK; 
    float shininess; 
}_Material;

uniform sampler2D first, second;

//Quincy Code: Toon Shading
//***************************
uniform int toon_color_levels = 4;
const float toon_scale_factor = 1.0f / toon_color_levels;
uniform float _RimLightPower = 4.0f;
uniform bool CellShadingEnabled = false;
uniform bool floorFuncEnabled = false;
uniform bool RimLightingEnabled = false;

float CalcRimLightingFactor(vec3 Eye, vec3 normal)
{
    float RimFactor = dot(Eye, normal);
    RimFactor = 1.0 - RimFactor; //want it to increase as diffuse light decreases
    RimFactor = max(0.0, RimFactor);
    RimFactor = pow(RimFactor, _RimLightPower);

    return RimFactor;
}
//***************************

void main(){      
    ambient = _Material.ambientK * texture(first, v_out.Uv).rgb;

    diffuse = vec3(0);
    specular = vec3(0);
    RimColor = vec3(0);

    //Directional Lights
    for(int i = 0; i < numDirLights; i++) {
        vec3 l = normalize(_DirLight[i].direction * -1);

        //Quincy Code
        //************************************
        //https://www.youtube.com/watch?v=h15kTY3aWaY
        float diffuseFactor = dot(v_out.WorldNormal, l);

        if (diffuseFactor > 0)
        {
            //might need to use local positions instead
            vec3 v = _CameraPos - v_out.WorldPosition;
            vec3 h = normalize(v + l);

            //Cell Shading
            if (CellShadingEnabled)
            {
                if(floorFuncEnabled)
                {
                    //results are darker
                    diffuseFactor = floor(diffuseFactor * toon_color_levels) * toon_scale_factor;
                }
                else
                {
                    //results are brighter
                    diffuseFactor = ceil(diffuseFactor * toon_color_levels) * toon_scale_factor;
                }
            }
            if(!CellShadingEnabled && !RimLightingEnabled) //dont do specular if CellShading is enabled
            {
                 specular += _Material.specularK * pow(dot(v_out.WorldNormal, h), _Material.shininess) * (_DirLight[i].intensity * _DirLight[i].color);
            }

            //Quincy Edit: put diffuseFactor in Max func
            diffuse += _Material.diffuseK * max(diffuseFactor, 0) * (_DirLight[i].intensity * _DirLight[i].color);

            //Rim Lighting
            if(RimLightingEnabled)
            {
                float RimFactor = CalcRimLightingFactor(v_out.Eye, v_out.WorldNormal);
                RimColor = diffuse * RimFactor;
            }

        }
        //*****************************************
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

    vec3 lightCol = ambient + diffuse + specular + RimColor; //added RimColor: Quincy
    vec3 col = _Material.color * lightCol;
    FragColor = vec4(col,1.0f);
}
