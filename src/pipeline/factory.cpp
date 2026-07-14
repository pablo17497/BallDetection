#include "puttingsimulator/pipeline/factory.hpp"

#include "puttingsimulator/pipeline/pipeline.hpp"
#include "puttingsimulator/services/opencv_reader.hpp"
#include "puttingsimulator/services/ball_detector.hpp"
#include "puttingsimulator/services/udp_sender.hpp"

using namespace puttingsimulator;

std::unique_ptr<PuttingSimPipeline> buildPipeline(const std::string& source) {
    auto reader = std::make_unique<OpenCVReader>(source);
    auto detector = std::make_unique<Detector>();
    auto sender = std::make_unique<UdpEventSender>();

    return std::make_unique<PuttingSimPipeline>(
        std::move(reader),
        std::move(detector),
        std::move(sender)
    );
}