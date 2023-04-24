#version 450

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

uniform mat4 _Model;
uniform mat4 _Translation;
uniform mat4 _View;
uniform mat4 _Projection;
uniform float _Outlining;

void main(){    
	vec3 crntPos = vec3(_Model * _Outlining * vec4(vPos , 1.0f));
	crntPos = vec3(_Translation * vec4(crntPos, 1.0f));
	gl_Position = _Projection * _View * vec4(crntPos, 1.0f);
}