#include "puttingsimulator/services/ball_detector.hpp"

#include <chrono>
#define _USE_MATH_DEFINES
#include <cmath>
#include <Eigen/Dense>
#include <vector>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

namespace puttingsimulator {
    Detector::Detector() : kf_(4, 2) {
        kf_.transitionMatrix =
            (
                cv::Mat_<float>(4, 4)
                << 1, 0, 1, 0,
                0, 1, 0, 1,
                0, 0, 1, 0,
                0, 0, 0, 1
                );
        kf_.measurementMatrix =
            (
                cv::Mat_<float>(2, 4)
                << 1, 0, 0, 0,
                0, 1, 0, 0
                );
        kf_.processNoiseCov =
            (
                cv::Mat_<float>(4, 4)
                << 0.05f, 0, 0, 0,
                0, 0.05f, 0, 0,
                0, 0, 0.5f, 0,
                0, 0, 0, 0.5f
                );
        kf_.measurementNoiseCov =
            (
                cv::Mat_<float>(2, 2)
                << 1.5f, 0,
                0, 1.5f
                );
        kf_.errorCovPost =
            (
                cv::Mat_<float>(4, 4)
                << 10, 0, 0, 0,
                0, 10, 0, 0,
                0, 0, 10, 0,
                0, 0, 0, 10
                );
    }

    Detection Detector::detect(const Frame& captured) {

        auto t0 = std::chrono::steady_clock::now();
        std::vector<long long> detectTimings = captured.timings;

        ball_center_.reset();
        ball_radius_.reset();

		if (shot_calculated_) {
			shot_calculated_ = false;
			impact_detected_ = false;
			frame_since_impact_ = 0;
			speed_list_.clear();
		}

        cv::Mat frame = captured.image;

        auto t1 = std::chrono::steady_clock::now();

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        auto t2 = std::chrono::steady_clock::now();
        
        cv::Mat blurred;
        cv::GaussianBlur(gray, blurred, cv::Size(9, 9), 0);

        auto t3 = std::chrono::steady_clock::now();
        
        cv::Mat thresh;
        cv::threshold(blurred, thresh, threshold_value_, 255, cv::THRESH_BINARY);

        auto t4 = std::chrono::steady_clock::now();
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        auto t5 = std::chrono::steady_clock::now();

        for (const auto& cnt : contours) {
            double area = cv::contourArea(cnt);
            double perimeter = cv::arcLength(cnt, true);
            if (area < min_radius_ * min_radius_ / 4.0f || perimeter == 0) {
                continue;
            }
            double pi = M_PI;
            double circularity = 4.0 * pi * area / (perimeter * perimeter);
            cv::Point2f center;
            float radius;
            cv::minEnclosingCircle(cnt, center, radius);
            if (min_radius_ < radius && radius < max_radius_ && circularity > 0.7 && 660 < center.y && center.y < 1000) {
                ball_center_ = cv::Point(cvRound(center.x), cvRound(center.y));
                ball_radius_ = static_cast<int>(radius);
                break;
            }
        }

        auto t6 = std::chrono::steady_clock::now();

        if (ball_center_) {
            cv::circle(frame, ball_center_.value(), ball_radius_.value(), {0, 255, 0}, 2);
            cv::circle(frame, ball_center_.value(), 3, {0, 0, 255}, -1);
        }

        auto t7 = std::chrono::steady_clock::now();
        
        cv::Point predictionCenter;

        if (kalman_initialized_) {
            cv::Mat prediction = kf_.predict();
            predictionCenter = cv::Point(cvRound(prediction.at<float>(0)), cvRound(prediction.at<float>(1)));
        }

        if (ball_center_) {
            cv::Mat measurement = (cv::Mat_<float>(2, 1) << ball_center_->x, ball_center_->y);
            if (!kalman_initialized_) {
                kf_.statePost = (cv::Mat_<float>(4, 1) << ball_center_->x, ball_center_->y, 0, 0);
                kalman_initialized_ = true;
            }
            else {
                kf_.correct(measurement);
            }
        }

        auto t8 = std::chrono::steady_clock::now();

        if (kalman_initialized_) {
            cv::circle(frame, predictionCenter, 5, { 255, 0, 0 }, -1);
        }

        auto t9 = std::chrono::steady_clock::now();

        if (kalman_initialized_ && ball_center_) {
            double innovation = std::sqrt(std::pow(ball_center_->x - predictionCenter.x, 2) + std::pow(ball_center_->y - predictionCenter.y, 2));
            if (innovation > 10) { //pixels
                kf_.processNoiseCov.at<float>(0, 0) = std::fmin(kf_.processNoiseCov.at<float>(0, 0) * 3.0f, 1.0f);
                kf_.processNoiseCov.at<float>(1, 1) = std::fmin(kf_.processNoiseCov.at<float>(1, 1) * 3.0f, 1.0f);
                kf_.processNoiseCov.at<float>(2, 2) = std::fmin(kf_.processNoiseCov.at<float>(2, 2) * 3.0f, 10.0f);
                kf_.processNoiseCov.at<float>(3, 3) = std::fmin(kf_.processNoiseCov.at<float>(3, 3) * 3.0f, 10.0f);
            }
            else {
                kf_.processNoiseCov.at<float>(0, 0) = std::fmax(kf_.processNoiseCov.at<float>(0, 0) * 0.8f, 0.05f);
                kf_.processNoiseCov.at<float>(1, 1) = std::fmax(kf_.processNoiseCov.at<float>(1, 1) * 0.8f, 0.05f);
                kf_.processNoiseCov.at<float>(2, 2) = std::fmax(kf_.processNoiseCov.at<float>(2, 2) * 0.8f, 0.5f);
                kf_.processNoiseCov.at<float>(3, 3) = std::fmax(kf_.processNoiseCov.at<float>(3, 3) * 0.8f, 0.5f);
            }
        }

        auto t10 = std::chrono::steady_clock::now();

        float v_x = kf_.statePost.at<float>(2, 0);
        float v_y = kf_.statePost.at<float>(3, 0);
        float kalman_speed = std::sqrt(v_x * v_x + v_y * v_y);

        float speed = 0.0f;
        if (has_iterated_) {
            if (ball_center_ && previous_ball_center_) {
                float delta_pos_x = ball_center_->x - previous_ball_center_->x;
                float delta_pos_y = ball_center_->y - previous_ball_center_->y;
                speed = std::sqrt(delta_pos_x * delta_pos_x + delta_pos_y * delta_pos_y);
            }
            previous_ball_center_ = ball_center_;
        }
        else {
            has_iterated_ = true;
            previous_ball_center_ = ball_center_;
        }

        if (kalman_speed > impact_speed_threshold_ && !impact_detected_ && ball_center_) {
            impact_detected_ = true;
            ball_pos_start_ = ball_center_;
        }

        if (impact_detected_) {
            frame_since_impact_ += 1;
        }

        std::optional<float> v0;
        std::optional<cv::Point2f> aim;

        if (1 < frame_since_impact_ && frame_since_impact_ < 9) {
            speed_list_.push_back(speed * meters_per_pixel_ * fps_);
        }

        auto t11 = std::chrono::steady_clock::now();

        if (frame_since_impact_ == 9 && ball_center_ && speed_list_.size() >= 7) {
            ball_pos_end_ = ball_center_;
            Eigen::VectorXd t(7);
            for (int i = 0; i < 7; ++i) { t(i) = (i + 2.0f) / fps_; }
            Eigen::VectorXd v(7);
            for (int i = 0; i < 7; ++i) { v(i) = speed_list_[i]; }
            Eigen::MatrixXd A(7, 2);
            A.col(0) = t;
            A.col(1) = Eigen::VectorXd::Ones(7);
            Eigen::Vector2d coeffs = A.colPivHouseholderQr().solve(v);
            v0 = coeffs(1);
            float norm = std::sqrt(std::pow(ball_pos_end_->x - ball_pos_start_->x, 2) + std::pow(ball_pos_end_->y - ball_pos_start_->y, 2));
            cv::Point2f direction((ball_pos_end_->x - ball_pos_start_->x) / norm, (ball_pos_end_->y - ball_pos_start_->y) / norm);
            aim = direction;
			shot_calculated_ = true;
        }

        auto t12 = std::chrono::steady_clock::now();

        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t6 - t5).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t7 - t6).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t8 - t7).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t9 - t8).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t10 - t9).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t11 - t10).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t12 - t11).count());
        detectTimings.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t12 - t0).count());

        Detection result;
        result.center = ball_center_;
        result.radius = ball_radius_;
		if (shot_calculated_) {
			result.speed = v0;
			result.aim = aim;
			result.ShotDetected = true;
		}
		else {
			result.speed = std::nullopt;
			result.aim = std::nullopt;
			result.ShotDetected = false;
		}
        result.BallDetected = ball_center_.has_value();
        result.frame = frame;
        result.timings = detectTimings;
		std::cout << "Ball center: " << ball_center_.value_or(cv::Point2f(0, 0)) << " Ball detected: " << ball_center_.has_value() << "\n";

        return result;
    }
}