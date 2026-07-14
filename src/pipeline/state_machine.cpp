#include "puttingsimulator/pipeline/state_machine.hpp"

namespace puttingsimulator {

    std::optional<EventType> StateMachine::update(const Detection& det) {
        switch (state_) {
            case VisionState::WaitingForBall:
                if (det.ballDetected) {
                    state_ = VisionState::BallReady;
                    return EventType::BallDetected;
                }
                break;
            case VisionState::BallReady:
                if (det.speed > 0.1)
                {
                    state_ = VisionState::TrackingShot;
                    return EventType::ShotStarted;
                }
                break;
            case VisionState::TrackingShot:
                if (det.speed < 0.01)
                {
                    state_ = VisionState::BallReady;
                    return EventType::ShotFinished;
                }
                break;
        }
        return std::nullopt;
    }
}