#pragma once

#include <chrono>
#include <string>
#include <variant>

namespace puttingsimulator {

    enum class MessageType {
        Calibration,
        BallLost,
        BallDetected,
        ShotDetected,
		EventNotRecognized
    };

    enum class Status {
        Started,
        Finished,
		Searching,
        BallFound,
        DataMissing,
		EventNotRecognized
    };

    struct ShotPayload
    {
        float velocity;
        float xAim;
        float yAim;
    };

    struct StatusPayload
    {
        Status status;
    };

    using Payload = std::variant<ShotPayload, StatusPayload>;

    struct Message {
        uint32_t version = 1;

        MessageType type;

        std::chrono::system_clock::time_point timestamp;

        Payload payload;
    };

}