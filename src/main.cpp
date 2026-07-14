#include "puttingsimulator/pipeline/factory.hpp"
#include "puttingsimulator/pipeline/pipeline.hpp"



int main() {
    
    auto pipeline = buildPipeline("D:\\CppProjects\\PuttingSimulator\\data\\top_down_putt.avi");

    pipeline->start();

    bool ran = pipeline->stop();

    return ran ? 0 : 1;
}