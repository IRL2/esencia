#include "GPURunner.h"
#include "Particle.h"

class GPUMoveParticles:public GPURunner {
public:
    void setup(std::vector<Particle>& particles);
    void run(std::vector<Particle>& particles);
    void setBufferSize(std::vector<Particle>& particles);
    //void updateParticlesAmmount(int ammount);
    //void updateSimulationBoundaries(float width, float height);
    //void updateDeltatime(float dt);
    int numParticles;

private:
    void setConfigBuffer();

    static constexpr size_t WORKGROUP_SIZE = 256; // Optimal for most GPUs

    cl::Buffer particleBuffer;
    cl::Buffer configBuffer;

    struct SimulationConfig {
        float width;    // world width
        float height;   // world height
        float dt;       // Time step
        float padding;  // For memory alignment
        int totalParticles;
    };
    SimulationConfig config;
};


void GPUMoveParticles::setup(std::vector<Particle>& particles) {
    setupKernel("kernels\\moveParticle.cl", "moveParticles");
    GPUMoveParticles::setBufferSize(particles);
}

void GPUMoveParticles::setConfigBuffer() {
    config = SimulationConfig { (float)ofGetWidth(), (float)ofGetHeight(), 0.8f, 0.0f, numParticles };

    configBuffer = cl::Buffer(
        context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(SimulationConfig),
        &config
    );
}

void GPUMoveParticles::setBufferSize(std::vector<Particle>& particles) {
    if (numParticles == particles.size()) return;

    numParticles = particles.size();

    // todo: needs to update the config.totalSize value too, but do this now creates race conditions and other unwanted behavoirs
    // must use shared data to the config value or block execution until buffers are correctly set
    
    if (config.totalParticles != numParticles) {
        config.totalParticles = numParticles;
        //GPUMoveParticles::setConfigBuffer();  // 
    }

    ofLogNotice("GPUMoveParticles::setup():Number of particles allocated in the particleBuffer: ") << numParticles;

    // Use pinned memory for faster transfers
    cl_mem_flags flags = CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR;
    particleBuffer = cl::Buffer(context, flags, sizeof(Particle) * numParticles);

    // Initial data transfer
    void* mappedPtr = queue.enqueueMapBuffer(
        particleBuffer,
        CL_TRUE,    // blocking
        CL_MAP_WRITE,
        0,
        sizeof(Particle) * numParticles
    );
    memcpy(mappedPtr, particles.data(), sizeof(Particle) * numParticles);
    queue.enqueueUnmapMemObject(particleBuffer, mappedPtr);
}

void GPUMoveParticles::run(std::vector<Particle>& particles) {
    if (particles.size() != numParticles) setBufferSize(particles);

    try {
        // Set kernel arguments
        kernel.setArg(0, particleBuffer);
        kernel.setArg(1, configBuffer);

        // Calculate work group size
        const size_t globalSize = ((numParticles + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE) * WORKGROUP_SIZE;

        // Enqueue kernel with local workgroup size
        cl::NDRange global(globalSize); // global work size (total number of working items
        cl::NDRange local(WORKGROUP_SIZE); // the number of work items per work group // GPU dependant
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

        // Asynchronous read the result to the host
        queue.enqueueReadBuffer(
            particleBuffer,
            CL_FALSE,  // non-blocking
            0,
            sizeof(Particle) * numParticles,
            particles.data()
        );

        //queue.finish();
    }
    catch (std::exception& ex) {
        ofLogFatalError("GPUMoveParticles::run():Exception") << ex.what();
    }
}

