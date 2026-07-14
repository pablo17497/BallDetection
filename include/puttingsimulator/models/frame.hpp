#pragma once

#define _USE_MATH_DEFINES
#include <opencv2/opencv.hpp>
#include <vector>
#include <optional>

namespace puttingsimulator {

    struct Frame {
        cv::Mat image;
        std::vector<long long> timings;
    };
}