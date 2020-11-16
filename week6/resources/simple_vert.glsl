#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

out vec3 fragNor;
out vec3 kd;
out vec3 ka;
out vec3 ks;
out float s;
out vec3 l;
out vec3 h;


void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (M * vec4(vertNor, 0.0)).xyz;

	//initial setup.
	vec3 light_source = vec3(0, 1, 1);
	vec4 WPos = (M * vertPos);
	l = normalize(light_source - WPos.xyz);

	//all to get h.
	vec3 ViewDir = -WPos.xyz;
	h = normalize(l + ViewDir);

	kd = MatDif;
	ka = MatAmb;
	ks = MatSpec;
	s = shine;
}
