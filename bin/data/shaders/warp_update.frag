#version 150

uniform sampler2D currentDisplacement;  // Previous frame's displacement
uniform sampler2D externalInput;        // Current video frame
uniform float warpVariance;
uniform float warpPropagation;
uniform float warpPropagationPersistence;
uniform float warpSpreadX;
uniform float warpSpreadY;
uniform float warpDetail;

in vec2 vTexCoord;
out vec2 fragColor;  // Output displacement as RG

void main() {
    vec2 scaledCoord = vTexCoord * warpDetail;
    vec2 texelSize = 1.0 / textureSize(externalInput, 0);
    
    vec2 left = vec2(scaledCoord.x - warpSpreadX * texelSize.x, scaledCoord.y);
    vec2 right = vec2(scaledCoord.x + warpSpreadX * texelSize.x, scaledCoord.y);
    vec2 top = vec2(scaledCoord.x, scaledCoord.y - warpSpreadY * texelSize.y);
    vec2 bottom = vec2(scaledCoord.x, scaledCoord.y + warpSpreadY * texelSize.y);

    //  current displacement from previous frame
    vec2 currentDisp = texture(currentDisplacement, scaledCoord).rg;
    vec2 currentDisp_Left = texture(currentDisplacement, left).rg;
    vec2 currentDisp_Right = texture(currentDisplacement, right).rg;
    vec2 currentDisp_Top = texture(currentDisplacement, top).rg;
    vec2 currentDisp_Bottom = texture(currentDisplacement, bottom).rg;
    
    // sample external input (video frame)
    float external_Center = texture(externalInput, scaledCoord).r;
    float external_Right = texture(externalInput, right).r;
    float external_Bottom = texture(externalInput, bottom).r;

    // calculate new displacement from gradients
    vec2 newDisplacement = vec2(
        (external_Center - external_Right) * warpVariance,
        (external_Center - external_Bottom) * warpVariance
    );

    // propagate existing displacement
    vec2 propDisplacement = vec2(
        (currentDisp.x * warpPropagationPersistence) + 
        ((currentDisp_Left.x + currentDisp_Right.x + currentDisp_Top.x + currentDisp_Bottom.x) * warpPropagation),
        (currentDisp.y * warpPropagationPersistence) + 
        ((currentDisp_Left.y + currentDisp_Right.y + currentDisp_Top.y + currentDisp_Bottom.y) * warpPropagation)
    );

    // combine and clamp..
    vec2 finalDisplacement = newDisplacement + propDisplacement;
    finalDisplacement = clamp(finalDisplacement, vec2(-10.0), vec2(10.0));

    fragColor = finalDisplacement;
}