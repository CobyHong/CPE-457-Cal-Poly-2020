#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
in float dCo;
out vec4 Outcolor;

void main()
{
	vec4 texColor0 = texture(Texture0, vTexCoord);

  	vec4 light_color = vec4(1, 1, 1, 1);
	Outcolor = texColor0 * dCo * light_color;

	if(texColor0.z > texColor0.x && texColor0.z > texColor0.y)
	{
		discard;
	}
  
	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);

}

