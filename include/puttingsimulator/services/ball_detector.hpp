#pragma once

#include <opencv2/core.hpp>
#include <opencv2/video/tracking.hpp>
#include "puttingsimulator/interfaces/detector.hpp"

namespace puttingsimulator {

    class Detector : public BallDetector {

        public:
            Detector();

            Detection detect(const Frame& captured) override;

        private:
            int min_radius_ = 27;
            int max_radius_ = 35;
            int threshold_value_ = 210;
            float meters_per_pixel_ = 0.042f / 66.0f;
            float fps_ = 60;
            float impact_speed_threshold_ = 6.0f;

            cv::KalmanFilter kf_;

            bool kalman_initialized_ = false;
            bool has_iterated_ = false;
            bool impact_detected_ = false;
        
            int frame_since_impact_ = 0;

            std::optional<cv::Point> ball_center_;
            std::optional<cv::Point> previous_ball_center_;
            std::optional<cv::Point> ball_pos_start_;
            std::optional<cv::Point> ball_pos_end_;
            std::optional<int> ball_radius_;

            std::vector<float> speed_list_;
    };
}