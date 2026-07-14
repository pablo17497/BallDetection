#pragma once

#include <optional>

#include "puttingsimulator/models/frame.hpp"

namespace puttingsimulator {

    class FrameReader {

        public:
            virtual ~FrameReader() = default;

            virtual void start() = 0;

            virtual std::optional<Frame> read() = 0;

            virtual void stop() = 0;
    };
}