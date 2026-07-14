#pragma once

#include <chrono>

namespace puttingsimulator {

    enum class EventType {
        BallDetected,
        BallLost,
        ShotStarted,
        ShotFinished
    };

    struct PipelineEvent {
        EventType type;
        std::chrono::steady_clock::time_point timestamp;
    };
}