#pragma once

#include <opencv2/opencv.hpp>
#include <optional>
#include <string>

#include "puttingsimulator/interfaces/frame_reader.hpp"

namespace puttingsimulator {

    class OpenCVReader : public FrameReader {

        public:
            explicit OpenCVReader(const std::string& source);

            void start() override;
            void stop() override;
            std::optional<Frame> read() override;

        private:
            cv::VideoCapture cap_;
            bool running_ = false;
    };
}