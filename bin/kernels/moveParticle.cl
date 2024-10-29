typedef struct  {
    float width;
    float height;
    float dt;
    int totalParticles;
} SimulationConfig;

typedef struct {
    // WARNING!
    // Be sure that this particle structure match to the one passed from the simulator GPURunner
    // if not you must transform it when pushing in-out to-from the kernel
    // otherwhise the buffer wont match and the calculations will be stored in different memory addresses
    float x, y;
    float vx, vy;
    float radius;
} Particle;

__kernel void moveParticles(
    __global Particle* particles,
    __global SimulationConfig* config
) {
    const int gid = get_global_id(0);
    const int size = get_global_size(0);

    const float radius = 2.0f;
    const float damping = 0.6f;

    // Early exit for padding threads
    if (gid >= size) return;

    // Load particle data into private memory
    Particle p = particles[gid];

    // Update position with velocity * dt
    p.x += p.vx ;
    p.y += p.vy ;

    particles[gid] = p;

    return;
    
    // Boundary collisions with elastic reflection
    if (p.x < p.radius || p.x > config->width - p.radius) {
        p.vx *= -damping;
        p.x = clamp(p.x, p.radius, config->width - p.radius);
    }

    if (p.y < p.radius || p.y > config->height - p.radius) {
        p.vy *= -damping;
        p.y = clamp(p.y, p.radius, config->height - p.radius);
    }


    // Particle-particle collisions
    for (int i = gid + 1; i < config->totalParticles; i++) {
        Particle other = particles[i];
        float dx = p.x - other.x;
        float dy = p.y - other.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= 2.0 * radius) {
            // Resolve collision
            float overlap = 2.0 * radius - distance;
            p.x += dx / distance * overlap * 0.5f;
            p.y += dy / distance * overlap * 0.5f;
            other.x -= dx / distance * overlap * 0.5f;
            other.y -= dy / distance * overlap * 0.5f;

            // Update velocities with damping
            p.vx = (p.vx - other.vx * damping) * damping;
            p.vy = (p.vy - other.vy * damping) * damping;
            other.vx = (other.vx - p.vx * damping) * damping;
            other.vy = (other.vy - p.vy * damping) * damping;

            // Write back modified particle
            particles[i] = other;
        }
    }

    // Write back to global memory
    particles[gid] = p;
}

