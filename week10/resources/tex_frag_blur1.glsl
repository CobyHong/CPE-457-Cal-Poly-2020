#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;

uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

void main(){
   color = vec4(texture(texBuf, texCoord).rgb, 1)*weight[0];

   for (int i=1; i < 3; i++) {
         color += vec4(texture( texBuf, texCoord + vec2(offset[i], 0.0)/640.0 ).rgb, 1)*weight[i];
         color += vec4(texture( texBuf, texCoord - vec2(offset[i], 0.0)/640.0 ).rgb, 1)*weight[i];
   }
}