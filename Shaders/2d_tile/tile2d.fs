#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) out vec3 tDiffuse;
layout (location = 2) out vec3 tPos;

#define LIGHTSOURCE_NONE 0
#define LIGHTSOURCE_DIR 1
#define LIGHTSOURCE_POINT 2
#define LIGHTSOURCE_SPOT 3

out vec4 FragColor;

in vec3 FragPos;
in vec2 UVCoord;

//textures
uniform sampler2D diffuse;

uniform bool hasDiffuseMap;

uniform vec3 amb_color;

void main(){

	vec2 uv = UVCoord;
	
	vec3 result = vec3(1.0, 0.078, 0.574); //Default value
	if(hasDiffuseMap)
		result = texture(diffuse, uv).xyz ;
		
	FragColor = vec4(result, 1);
	tDiffuse = FragColor.rgb;
	tPos = FragPos;
}
