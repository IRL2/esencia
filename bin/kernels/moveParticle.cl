struct Particle {
    float x, y;
    float vx, vy;
};

kernel void moveParticles(global struct Particle* particles) {
    int id = get_global_id(0);
    particles[id].x += particles[id].vx;
    particles[id].y += particles[id].vy;
    if (particles[id].x < 0 || particles[id].x > 1024) particles[id].vx *= -1;
    if (particles[id].y < 0 || particles[id].y > 768 ) particles[id].vy *= -1;
}
