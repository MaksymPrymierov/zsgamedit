#version 420 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out vec3 FragPos;
out vec3 InNormal;
out vec2 UVCoord;
out mat3 TBN;
out vec3 _id;

layout (std140, binding = 0) uniform CamMatrices{
uniform mat4 cam_projection;
uniform mat4 cam_view;
uniform mat4 object_transform;
};

void main(){
	_id = position;
	UVCoord = uv;
	InNormal = normal;
	
	FragPos = (object_transform * vec4(position, 1.0)).xyz;
	
	gl_Position =  cam_projection * cam_view * vec4(FragPos, 1.0);
	
}
