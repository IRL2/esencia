#include "GPURunner.h"


std::string GPURunner::loadKernel(std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open kernel file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void GPURunner::setupKernel(std::string_view kernelFilename, std::string_view kernelProgram) {
    std::string kernelFilenameStr(kernelFilename);
    std::string kernelProgramStr(kernelProgram);

    try {
        // Get all platforms (drivers), e.g. NVIDIA, Intel, AMD
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.empty()) {
            throw std::runtime_error("No OpenCL platforms found.");
        }

        // Get default device of the first platform
        cl_context_properties properties[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(),
            0
        };
        context = cl::Context(CL_DEVICE_TYPE_GPU, properties);

        // Get a list of devices on this platform
        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        if (devices.empty()) {
            throw std::runtime_error("No OpenCL devices found.");
        }

        // Create a command queue and use the first device
        queue = cl::CommandQueue(context, devices[0]);

        // Load kernel code from file
        std::string kernelCode = loadKernel(kernelFilenameStr);

        // Build the kernel code
        program = cl::Program(context, kernelCode);
        program.build(devices);

        // Create kernel
        kernel = cl::Kernel(program, kernelProgramStr.c_str());

        // Log build info
        std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        std::cout << "GPURunner::setupKernel()::Build Log: " << buildLog;
    }
    catch (std::exception& ex) {
        std::cerr << "GPURunner::setupKernel()::Exception: " << ex.what() << std::endl;
    }
}

