#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 light_source;
uniform vec3 eye;

out vec3 fragNor;
out vec3 l;
out vec3 h;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (M * vec4(vertNor, 0.0)).xyz;

	//initial setup.
	vec4 WPos = (M * V * vertPos);
	l = normalize(light_source - WPos.xyz);

	//all to get h.
	vec3 ViewDir = eye - WPos.xyz;
	h = normalize(l + ViewDir);

}
