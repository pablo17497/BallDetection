#pragma once

#include <chrono>
#include <opencv2/core.hpp>
#include <optional>

namespace puttingsimulator {

    enum class EventType {
        CalibrationStarted,
        CalibrationFinished,
        BallDetected,
        BallLost,
        ShotDetected,
    };

    struct DetectionData {
        float speed;
        cv::Point2f aim;
    };

    struct PipelineEvent {
        EventType type;
        std::chrono::system_clock::time_point timestamp;
        std::optional<DetectionData> data;
    };
}