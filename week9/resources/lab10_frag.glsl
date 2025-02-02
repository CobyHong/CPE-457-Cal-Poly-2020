#version 330 core

uniform sampler2D alphaTexture;

in vec3 partCol;
in float a;

out vec4 outColor;


void main()
{
	float alpha = texture(alphaTexture, gl_PointCoord).r;

	outColor = vec4(partCol, alpha * a);
}
