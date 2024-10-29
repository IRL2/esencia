#pragma once

#include <CL/cl.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "ofMain.h"


class GPURunner {
public:
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
    cl::Kernel kernel;

    std::string loadKernel(std::string& filename);
    void setupKernel(std::string_view kernelFilename, std::string_view kernelProgram);
};

