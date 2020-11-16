#version 330 core 
out vec4 color;
in vec3 fragNor;

in vec3 l;
in vec3 h;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

void main()
{
	//pre-setup (rest of setup done in vert shader).
	vec3 normal = normalize(fragNor); //getting normal.
	vec3 light_color = vec3(1, 1, 1);

	//diffuse.
	vec3 diff = MatDif * max(0, dot(normal, l)) * light_color;

	//ambient.
	vec3 amb = MatAmb * light_color;

	//specular.
	vec3 spec = MatSpec * pow(max(0, dot(normal,h)), shine) * light_color;

	//reflective color.
	vec3 rc = diff + amb;

	color = vec4(rc, 1.0); //return color.
}
