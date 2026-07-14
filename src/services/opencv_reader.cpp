#include "puttingsimulator/services/opencv_reader.hpp"

#include <chrono>
#include <stdexcept>

namespace puttingsimulator {
    OpenCVReader::OpenCVReader(const std::string& source) : cap_(source) {
        if (!cap_.isOpened()) {
            throw std::runtime_error("Cannot open video source");
        }
    }

    void OpenCVReader::start() {
        running_ = true;
    }

    void OpenCVReader::stop() {
        running_ = false;
        cap_.release();
    }

    std::optional<Frame> OpenCVReader::read() {
        if (!running_) {
            return std::nullopt;
        }

        auto t0 = std::chrono::steady_clock::now();

        cv::Mat frame;

        if (!cap_.read(frame)) {
            return std::nullopt;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count();

        Frame result{
            frame,
            {elapsed}
        };

        return result;
    }
}