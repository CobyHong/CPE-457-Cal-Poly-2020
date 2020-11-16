#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
out vec4 Outcolor;

in vec3 fragNor;
in vec3 l;
in vec3 h;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

uniform float Invert;

void main()
{
	vec4 texColor0 = texture(Texture0, vTexCoord);
	vec3 normal = normalize(fragNor);
  	vec4 light_color = vec4(1, 1, 1, 1);

	//diffuse.
	vec4 diff = texColor0 * max(0, dot(Invert * normal, l)) * light_color;

	//ambient.
	vec4 amb = vec4(MatAmb, 1.0) * light_color;

	//specular.
	vec4 spec = vec4(MatSpec, 1.0) * pow(max(0, dot(normal,h)), shine) * light_color;

	//reflective color.
	vec4 rc = diff + spec + amb;

	Outcolor = rc; //return color.
}
