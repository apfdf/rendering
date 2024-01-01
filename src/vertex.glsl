
#version 330 core

layout (location = 0) in vec3 aPos;

out vec2 frag_pos;

void main() {
	frag_pos = vec2(aPos.x, aPos.y);
   	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
