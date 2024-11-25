#version 430

layout(local_size_x = 256) in;

struct Particle {
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
uniform float targetTemperature;
uniform float coupling;
uniform bool applyThermostat;
uniform float depthFieldScale;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index >= particles.length()) return;

    Particle p = particles[index];

    // Sample depth field
    vec2 texCoord = clamp(p.position / worldSize, vec2(0.0), vec2(1.0));
    float depth = texture(depthField, texCoord).r;

    // Calculate gradient with correct force direction
    vec2 texelSize = 1.0 / worldSize;
    float dx = (texture(depthField, clamp(texCoord + vec2(texelSize.x, 0), vec2(0), vec2(1))).r -
        texture(depthField, clamp(texCoord - vec2(texelSize.x, 0), vec2(0), vec2(1))).r) * 0.5;
    float dy = (texture(depthField, clamp(texCoord + vec2(0, texelSize.y), vec2(0), vec2(1))).r -
        texture(depthField, clamp(texCoord - vec2(0, texelSize.y), vec2(0), vec2(1))).r) * 0.5;

    // Force points downhill (towards darker regions)
    vec2 depthForce = depthFieldScale * vec2(dx, dy);

    // Combined force and acceleration
    vec2 acceleration = depthForce / p.mass;

    // Update velocity with damping for stability
    p.velocity = p.velocity * 0.99 + acceleration * deltaTime;

    // Apply thermostat if enabled
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
    if (p.position.x < 0.0 || p.position.x > worldSize.x) {
        p.velocity.x *= -0.9;  // Added damping on collision
        p.position.x = clamp(p.position.x, 0.0, worldSize.x);
    }
    if (p.position.y < 0.0 || p.position.y > worldSize.y) {
        p.velocity.y *= -0.9;  // Added damping on collision
        p.position.y = clamp(p.position.y, 0.0, worldSize.y);
    }

    particles[index] = p;
}