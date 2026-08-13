#pragma once

#include "puttingsimulator/models/message.hpp"

#include <string>

namespace puttingsimulator {

    class MessageSerializer {
        public:
            static std::string toJson(const Message& msg);
            static std::string toString(MessageType type);
            static std::string toString(Status status);
    };

}