#include "puttingsimulator/services/message_serializer.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace puttingsimulator {

    std::string MessageSerializer::toJson(const Message& msg) {
        json j;

        j["version"] = msg.version;
        j["type"] = toString(msg.type);

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                msg.timestamp.time_since_epoch()).count();

        j["timestamp"] = ms;

        json payload;

        switch (msg.type) {
            case MessageType::Calibration:
            case MessageType::BallLost:
            case MessageType::BallDetected:
            case MessageType::EventNotRecognized:
            {
                const auto& status = std::get<StatusPayload>(msg.payload);
                payload["status"] = toString(status.status);
                break;
            }

            case MessageType::ShotDetected:
            {
                if (std::holds_alternative<ShotPayload>(msg.payload)) {
                    const auto& shot = std::get<ShotPayload>(msg.payload);
                    payload["velocity"] = shot.velocity;
                    payload["x_aim"] = shot.xAim;
                    payload["y_aim"] = shot.yAim;
                }
                else {
                    const auto& status = std::get<StatusPayload>(msg.payload);
                    payload["status"] = toString(status.status);
                }
                break;
            }

            default:
                break;
        }

        j["payload"] = payload;

        return j.dump();
    }

    std::string MessageSerializer::toString(MessageType type)
    {
        switch (type) {
            case MessageType::Calibration:
                return "Calibration";

            case MessageType::BallLost:
                return "BallLost";

            case MessageType::BallDetected:
                return "BallDetected";

            case MessageType::ShotDetected:
                return "ShotDetected";

            case MessageType::EventNotRecognized:
                return "EventNotRecognized";
        }

        return "Unknown";
    }

    std::string MessageSerializer::toString(Status status)
    {
        switch (status) {
            case Status::Started:
                return "Started";

            case Status::Finished:
                return "Finished";

            case Status::Searching:
                return "Searching";

            case Status::BallFound:
                return "BallFound";

            case Status::DataMissing:
                return "DataMissing";

            case Status::EventNotRecognized:
                return "EventNotRecognized";
        }

        return "Unknown";
    }
}