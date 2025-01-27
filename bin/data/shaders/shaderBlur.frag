OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;

uniform float blurAmnt;
uniform float texwidth;
uniform float texheight;

in vec2 texCoordVarying;
out vec4 fragColor;

uniform int pattPeriod = 5;
uniform float pattSize = 2.0;

// Gaussian weights from http://dev.theomader.com/gaussian-kernel-calculator/

void main()
{
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

	// horizontal
	color += 0.000229 * texture(tex0, texCoordVarying + vec2(blurAmnt * -4.0/texwidth, 0.0));
	color += 0.005977 * texture(tex0, texCoordVarying + vec2(blurAmnt * -3.0/texwidth, 0.0));
	color += 0.060598 * texture(tex0, texCoordVarying + vec2(blurAmnt * -2.0/texwidth, 0.0));
	color += 0.241732 * texture(tex0, texCoordVarying + vec2(blurAmnt * -1.0/texwidth, 0.0));

	color += 0.382928 * texture(tex0, texCoordVarying + vec2(0.0, 0));

	color += 0.241732 * texture(tex0, texCoordVarying + vec2(blurAmnt * 1.0/texwidth, 0.0));
	color += 0.060598 * texture(tex0, texCoordVarying + vec2(blurAmnt * 2.0/texwidth, 0.0));
	color += 0.005977 * texture(tex0, texCoordVarying + vec2(blurAmnt * 3.0/texwidth, 0.0));
	color += 0.000229 * texture(tex0, texCoordVarying + vec2(blurAmnt * 4.0/texwidth, 0.0));


	// vertical
	color += 0.000229 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * 4.0/texheight));
	color += 0.005977 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * 3.0/texheight));
	color += 0.060598 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * 2.0/texheight));
	color += 0.241732 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * 1.0/texheight));

	color += 0.382928 * texture(tex0, texCoordVarying + vec2(0.0, 0.0));

	color += 0.241732 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * -1.0/texheight));
	color += 0.060598 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * -2.0/texheight));
	color += 0.005977 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * -3.0/texheight));
	color += 0.000229 * texture(tex0, texCoordVarying + vec2(0.0, blurAmnt * -4.0/texheight));

    //fragColor = color;

    vec3 colorr = texture(tex0, texCoordVarying).rgb;
    ivec2 pixelPos = ivec2(gl_FragCoord.xy);
	vec3 finalColor;


	// finalColor = mix(color.rgb, colorr, -3.0);
	// finalColor = mix(finalColor, colorr, 3.1);
	
	finalColor = 1.0 - (1.0 - color.rgb) * (1.0 - colorr);
	// finalColor = abs(color.rgb - texture(texCoordVarying);
    colorr = mix(color.rgb, finalColor, 0.5);

    // float edge = smoothstep(0.5- 0.05, 0.5 + 0.05, color.a);
    // vec3 thresholdedColor = mix(vec3(0.0), colorr.rgb, edge); // Black & color transition

    if ((pixelPos.x % pattPeriod == pattSize) || (pixelPos.y % pattPeriod == pattSize)) {
		finalColor = color.rgb * 0.8;
		// discard;
    }

	finalColor = mix(2.0 * color.rgb * finalColor, 1.0 - 2.0 * (1.0 - color.rgb) * (1.0 - finalColor), step(0.5, color.rgb));

	fragColor = vec4(finalColor, color.a);
}
