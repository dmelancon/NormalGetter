in vec2 vTexCoord0;
uniform sampler2D uSampler;
uniform vec2 u_textureSize;
uniform float bias;
uniform float invertR;
uniform float invertG;
out vec4 fragColor;

  
void main(void) {
	vec2 _step = vec2(1.0, 1.0) / u_textureSize;
	float d0 = abs(texture(uSampler, vTexCoord0.xy + vec2(0.0, 0.0)).r);
	float d1 = abs(texture(uSampler, vTexCoord0.xy + vec2(_step.x, 0.0)).r);
	float d2 = abs(texture(uSampler, vTexCoord0.xy + vec2(-_step.x, 0.0)).r);
	float d3 = abs(texture(uSampler, vTexCoord0.xy + vec2(0.0, _step.y)).r);
	float d4 = abs(texture(uSampler, vTexCoord0.xy + vec2(0.0, -_step.y)).r);
	float dx = ((d2 - d0) + (d0 - d1)) * 0.5;
	float dy = ((d4 - d0) + (d0 - d3)) * 0.5;
	vec4 normal = vec4(normalize(vec3(dx * invertR, dy * invertG, 1.0 - ((bias - 0.1) / 100.0))), 1.0);
  
//    fragColor = texture(uSampler, vTexCoord0);
    	fragColor = vec4(normal.xyz * 0.5 + 0.5, 1.0);
}