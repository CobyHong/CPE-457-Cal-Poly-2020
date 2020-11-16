#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

uniform vec3 light_source;
uniform vec3 eye;

out vec2 vTexCoord;
out vec3 fragNor;
out vec3 l;
out vec3 h;

void main() {
  vec4 vPosition;

  /* First model transforms */
  gl_Position = P * V *M * vec4(vertPos.xyz, 1.0);
  fragNor = (M * vec4(vertNor, 0.0)).xyz;

  vec3 WPos = (M * vec4(vertPos, 0.0)).xyz;
  l = normalize(light_source - WPos);

	vec3 ViewDir = eye - WPos.xyz;
	h = normalize(l + ViewDir);

  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
}
