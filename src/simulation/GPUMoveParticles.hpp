#include "GPURunner.h"
#include "Particle.h"

class GPUMoveParticles:public GPURunner {
public:
    void setup(std::vector<Particle>& particles);
    void run(std::vector<Particle>& particles);
    void setBufferSize(std::vector<Particle>& particles);

    //std::vector<Particle2> particles;
    cl::Buffer particleBuffer;
    int numParticles;
};


void GPUMoveParticles::setup(std::vector<Particle>& particles) {
    setupKernel("kernels\\moveParticle.cl", "moveParticles");

    GPUMoveParticles::setBufferSize(particles);
}

void GPUMoveParticles::setBufferSize(std::vector<Particle>& particles) {
    numParticles = particles.size();
    std::cout << "GPUMoveParticles::setup():Number of particles allocated in the particleBuffer: " << numParticles;

    // Create buffer for particles
    particleBuffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Particle) * numParticles, particles.data());
}

void GPUMoveParticles::run(std::vector<Particle>& particles) {
    try {
        // Set kernel arguments
        kernel.setArg(0, particleBuffer);

        // Enqueue kernel execution
        // todo: recalculate number of particles
        cl::NDRange global(numParticles);
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
        queue.finish();

        // Read the result back to host
        queue.enqueueReadBuffer(particleBuffer, CL_FALSE, 0, sizeof(Particle) * numParticles, particles.data());
    }
    catch (std::exception& ex) {
        std::cout << "GPUMoveParticles::run():Exception: " << ex.what();
    }
}

