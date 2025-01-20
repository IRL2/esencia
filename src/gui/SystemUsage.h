#include <pdh.h>
#include <vector>
#include <numeric>
#include <algorithm>

#pragma comment(lib, "pdh.lib")

class SystemUsage {

    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;

public:

    std::vector<int> cpuHistory;
    int maxHistory = 150;
    int currentIndex = 0;
    bool bufferFull = false;

    void setup() {
        PdhOpenQuery(NULL, NULL, &cpuQuery);
        PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
        PdhCollectQueryData(cpuQuery);
        cpuHistory.resize(maxHistory, 0);
        update();
    }

    int getCPUUsage() {
        PDH_FMT_COUNTERVALUE counterVal;
        PdhCollectQueryData(cpuQuery);
        PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
        return (int)counterVal.doubleValue;
    }

    int getSmoothedCPUUsage() {
        int avgWindow = bufferFull ? maxHistory : currentIndex;
        avgWindow = maxHistory;
        return (int) std::accumulate(cpuHistory.begin(), cpuHistory.begin() + maxHistory, 0.0f) / maxHistory;
    }

    void update() {
        int usage = getCPUUsage();
        cpuHistory[currentIndex] = usage;
        currentIndex = (currentIndex + 1) % maxHistory;
    }

    void exit() {
        PdhCloseQuery(cpuQuery);
    }

    SystemUsage() = default;
};