// Shader code: particlesComputeShader.glsl
#version 430

layout(local_size_x = 256) in;

struct Particle {
    vec2 position;
    vec2 velocity;
    float mass;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

uniform float deltaTime;
uniform vec2 worldSize;
uniform float targetTemperature;
uniform float coupling;
uniform bool applyThermostat;

// Gravity or other global forces can be declared here.
const vec2 globalForce = vec2(0.0, -9.8);

void main() {
    uint index = gl_GlobalInvocationID.x;
    
    if (index >= particles.length()) {
        return;
    }

    Particle p = particles[index];

    // Compute acceleration from forces
    vec2 force = globalForce; // You can add other forces here
    vec2 acceleration = force / p.mass;

    // Update velocity using Euler integration
    p.velocity += deltaTime * acceleration;

    // Apply thermostat if enabled
    if (applyThermostat) {
        float kineticEnergy = 0.5 * p.mass * dot(p.velocity, p.velocity);
        float currentTemperature = kineticEnergy / 3.0;
        float scaleFactor = sqrt(targetTemperature / (coupling * currentTemperature));
        scaleFactor = clamp(scaleFactor, 0.95, 1.05);
        p.velocity *= scaleFactor;
    }

    // Update position using Euler integration
    p.position += deltaTime * p.velocity;

    // Boundary conditions (bouncing off walls)
    if (p.position.x < 0.0 || p.position.x > worldSize.x) {
        p.velocity.x *= -1.0;
        p.position.x = clamp(p.position.x, 0.0, worldSize.x);
    }
    if (p.position.y < 0.0 || p.position.y > worldSize.y) {
        p.velocity.y *= -1.0;
        p.position.y = clamp(p.position.y, 0.0, worldSize.y);
    }

    particles[index] = p;
}
