#pragma once

#define _USE_MATH_DEFINES
#include <opencv2/opencv.hpp>
#include <optional>
#include <vector>

namespace puttingsimulator {

    struct Detection {
        std::optional<cv::Point2f> center;
        std::optional<float> radius;
        std::optional<float> speed;
        std::optional<cv::Point2f> aim;
		std::optional<bool> BallDetected;
        std::optional<bool> ShotDetected;

        cv::Mat frame;
        std::vector<long long> timings;
    };
}