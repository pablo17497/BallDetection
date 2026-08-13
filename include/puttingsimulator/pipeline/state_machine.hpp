#pragma once

#include "puttingsimulator/models/detection.hpp"
#include "puttingsimulator/models/event.hpp"
#include <optional>

namespace puttingsimulator {

    enum class VisionState {
        Calibration,
        SearchingBall,
        BallDetected,
		ShotDetected,
    };

    class StateMachine {

        public:
            std::optional<EventType> update(const Detection& det);

        private:
            VisionState state_ = VisionState::Calibration;
    };
}