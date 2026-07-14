#pragma once

#include "puttingsimulator/models/frame.hpp"
#include "puttingsimulator/models/detection.hpp"

namespace puttingsimulator {

    class BallDetector {

        public:
            virtual ~BallDetector() = default;

            virtual Detection detect(const Frame& captured) = 0;
    };
}