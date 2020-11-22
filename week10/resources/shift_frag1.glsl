#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;

void main(){

	vec3 texColor = texture( texBuf, texCoord ).rgb;

	if( mod(gl_FragCoord.x, 20) >= 10 )
	{
		vec2 newCoord = vec2(texCoord.x, texCoord.y + 0.01);
		texColor = texture( texBuf, newCoord ).rgb;
	}

	color = vec4(texColor, 1.0);
}
