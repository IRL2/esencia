#version 430

layout(local_size_x = 256) in;

struct Particle {
    //int index;
    vec2 position;
    vec2 velocity;
    float radius;
    float mass;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout(binding = 0) uniform sampler2D depthField;

uniform float deltaTime;
uniform vec2 worldSize;
uniform vec2 videoOffset;
uniform vec2 videoScale;
uniform vec2 sourceSize;
uniform float targetTemperature;
uniform float coupling;
uniform bool applyThermostat;
uniform float depthFieldScale;

// Lennard-Jones parameters
uniform float ljEpsilon;   // Depth of potential well
uniform float ljCutoff;    // Cutoff for interaction distance
uniform float maxForce;    // Max allowable force magnitude
uniform float sigma;       // Currently unused, kept for potential scaling

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index >= particles.length()) return;

    Particle p = particles[index];
    vec2 totalLJForce = vec2(0.0);

    // Interaction loop
    for (uint i = 0; i < particles.length(); i++) {
        if (i == index) continue;

        Particle other = particles[i];
        vec2 diff = p.position - other.position;
        float dist = length(diff);
        float minDist = p.radius + other.radius; // Minimum interaction distance

        // Lennard-Jones potential
        if (dist > 0.0 && dist < ljCutoff) {
            // Standard LJ
            float invDist = minDist / dist;  
            float invDist6 = pow(invDist, 6.0);
            float invDist12 = invDist6 * invDist6;
            float ljForceMag = 24.0 * ljEpsilon * (2.0 * invDist12 - invDist6) / dist;

            // Optionally clamp
            ljForceMag = clamp(ljForceMag, -maxForce, maxForce);

            // Direction from other -> p
            vec2 dir = normalize(diff); // diff = p - other
            totalLJForce += dir * ljForceMag;
        }
    }

    // Apply Lennard-Jones forces to acceleration
    vec2 acceleration = totalLJForce / p.mass;

    // Add additional forces like depth-based gradient
    vec2 adjustedPos = (p.position - videoOffset) / videoScale;
    vec2 texCoord = adjustedPos / sourceSize;
    texCoord = clamp(texCoord, vec2(0.0), vec2(1.0));

    // Sample depth field
    float depth = texture(depthField, texCoord).r;

    // Calculate depth gradient
    vec2 texelSize = 1.0 / sourceSize;
    float dx = (texture(depthField, clamp(texCoord + vec2(texelSize.x, 0), vec2(0), vec2(1))).r -
        texture(depthField, clamp(texCoord - vec2(texelSize.x, 0), vec2(0), vec2(1))).r) * 0.5;
    float dy = (texture(depthField, clamp(texCoord + vec2(0, texelSize.y), vec2(0), vec2(1))).r -
        texture(depthField, clamp(texCoord - vec2(0, texelSize.y), vec2(0), vec2(1))).r) * 0.5;

    vec2 depthForce = depthFieldScale * vec2(dx, dy) * videoScale;

    // Combine all forces
    vec2 totalForce = depthForce + totalLJForce;
    acceleration += totalForce / p.mass;

    // Update velocity with damping
    p.velocity = p.velocity * 0.99 + acceleration * deltaTime;

    // Apply thermostat
    if (applyThermostat) {
        float kineticEnergy = 0.5 * p.mass * dot(p.velocity, p.velocity);
        float currentTemperature = kineticEnergy / 3.0;
        float scaleFactor = sqrt(targetTemperature / (coupling * currentTemperature));
        scaleFactor = clamp(scaleFactor, 0.95, 1.05);
        p.velocity *= scaleFactor;
    }

    // Update position
    p.position += p.velocity * deltaTime;

    // Boundary conditions
    vec2 minBound = videoOffset;
    vec2 maxBound = videoOffset + sourceSize * videoScale;

    if (p.position.x < minBound.x || p.position.x > maxBound.x) {
        p.velocity.x *= -0.9;
        p.position.x = clamp(p.position.x, minBound.x, maxBound.x);
    }
    if (p.position.y < minBound.y || p.position.y > maxBound.y) {
        p.velocity.y *= -0.9;
        p.position.y = clamp(p.position.y, minBound.y, maxBound.y);
    }

    particles[index] = p;
}
