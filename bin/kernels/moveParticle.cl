struct Particle {
    // WARNING!
    // Be sure that this particle structure match to the one passed from the simulator GPURunner
    // if not you must transform it when pushing in-out to-from the kernel
    // otherwhise the buffer wont match and the calculations will be stored in different memory addresses

    float x, y;
    float vx, vy;

    float mass;
    float radius;

    //glm::vec2 position;
    //std::vector<float> minimumDistance;
};

kernel void moveParticles(global struct Particle* particles) {
    int id = get_global_id(0);
    particles[id].x += particles[id].vx;
    particles[id].y += particles[id].vy;
    if (particles[id].x < 0 || particles[id].x > 1024) particles[id].vx *= -1;
    if (particles[id].y < 0 || particles[id].y > 768 ) particles[id].vy *= -1;
}
