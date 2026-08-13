#include "puttingsimulator/pipeline/state_machine.hpp"
#include <iostream>

namespace puttingsimulator {

    std::optional<EventType> StateMachine::update(const Detection& det) {
        switch (state_) {
            case VisionState::Calibration:
                if (det.BallDetected == true) {
                    state_ = VisionState::BallDetected;
                    return EventType::BallDetected;
                }
				else {
					state_ = VisionState::SearchingBall;
                    return EventType::CalibrationFinished;
				}
				std::cout << "Calibration\n" ;
                break;
            case VisionState::SearchingBall:
                if (det.BallDetected == true) {
                    state_ = VisionState::BallDetected;
                    return EventType::BallDetected;
                }
                std::cout << "Searching for ball\n";
                break;
            case VisionState::BallDetected:
                if (det.ShotDetected == true && det.speed.has_value() && det.aim.has_value()) {
                    state_ = VisionState::ShotDetected;
                    return EventType::ShotDetected;
                }
				if (det.BallDetected == false) {
					state_ = VisionState::SearchingBall;
					return EventType::BallLost;
				}
                std::cout << "Ball detected\n";
                break;
            case VisionState::ShotDetected:
                if (det.BallDetected == true) {
                    state_ = VisionState::BallDetected;
                    return EventType::BallDetected;
                }
                else {
                    state_ = VisionState::SearchingBall;
                    return EventType::BallLost;
                }
                std::cout << "Shot detected\n";
                break;
        }
        return std::nullopt;
    }
}