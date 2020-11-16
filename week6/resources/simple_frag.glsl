#version 330 core 
in vec3 fragNor;
out vec4 color;
in vec3 kd;
in vec3 ka;
in vec3 ks;
in vec3 l;
in vec3 h;
in float s;

void main()
{
	//pre-setup (rest of setup done in vert shader).
	vec3 normal = normalize(fragNor); //getting normal.
	vec3 light_color = vec3(1, 1, 1);

	//diffuse.
	vec3 diff = kd * max(0, dot(normal, l)) * light_color;

	//ambient.
	vec3 amb = ka * light_color;

	//specular.
	vec3 spec = ks * pow(max(0, dot(normal,h)), s) * light_color;

	//reflective color.
	vec3 rc = diff + spec + amb;

	color = vec4(rc, 1.0); //return color.
}
