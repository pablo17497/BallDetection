#include "puttingsimulator/pipeline/pipeline.hpp"
#include <chrono>

namespace puttingsimulator {
    
    PuttingSimPipeline::PuttingSimPipeline(std::unique_ptr<FrameReader> reader, std::unique_ptr<BallDetector> detector, std::unique_ptr<EventSink> eventSink)
    : reader_(std::move(reader)), detector_(std::move(detector)), eventSink_(std::move(eventSink)){
        maxQueueSize_ = 5;
        windowName_ = "Detection";
        cv::namedWindow(windowName_, cv::WINDOW_NORMAL);
    }

    void PuttingSimPipeline::start() {
        if (!eventSink_->initialize())
        {
            std::cerr << "Failed to initialize event sink\n";
            return;
        }
        running_ = true;
        reader_->start();
        readerThread_ = std::thread(&PuttingSimPipeline::readerLoop, this);
        detectThread_ = std::thread(&PuttingSimPipeline::detectLoop, this);
        debugLoop();
    }

    bool PuttingSimPipeline::stop() {
        {
            running_ = false;
        }
        frameCv_.notify_all();
        detectCv_.notify_all();
        if (readerThread_.joinable()) {
            readerThread_.join();
        }
        if (detectThread_.joinable()) {
            detectThread_.join();
        }
        eventSink_->clean_up();

        auto readStats = computeStats(timing_history_, [](const timings& t) { return t.read; });
        auto setupStats = computeStats(timing_history_, [](const timings& t) { return t.setup; });
        auto cvtColorStats = computeStats(timing_history_, [](const timings& t) { return t.cvtColor; });
        auto blurStats = computeStats(timing_history_, [](const timings& t) { return t.blur; });
        auto ThresholdStats = computeStats(timing_history_, [](const timings& t) { return t.threshold; });
        auto contoursStats = computeStats(timing_history_, [](const timings& t) { return t.contours; });
        auto contoursAnalysisStats = computeStats(timing_history_, [](const timings& t) { return t.contourAnalysis; });
        auto drawBallStats = computeStats(timing_history_, [](const timings& t) { return t.drawBall; });
        auto kalmanStats = computeStats(timing_history_, [](const timings& t) { return t.kalman; });
        auto drawPredictionStats = computeStats(timing_history_, [](const timings& t) { return t.drawPrediction; });
        auto processNoiseStats = computeStats(timing_history_, [](const timings& t) { return t.processNoise; });
        auto speedCalcStats = computeStats(timing_history_, [](const timings& t) { return t.speedCalc; });
        auto regressionStats = computeStats(timing_history_, [](const timings& t) { return t.regression; });
        auto totalStats = computeStats(timing_history_, [](const timings& t) { return t.total; });

        std::cout << "Read:\n" << "  Mean:   " << readStats.mean << " us\n" << "  StdDev: " << readStats.stddev << " us\n" << "  Max:    " << readStats.max << " us\n";
        std::cout << "Setup:\n" << "  Mean:   " << setupStats.mean << " us\n" << "  StdDev: " << setupStats.stddev << " us\n" << "  Max:    " << setupStats.max << " us\n";
        std::cout << "CvtColor:\n" << "  Mean:   " << cvtColorStats.mean << " us\n" << "  StdDev: " << cvtColorStats.stddev << " us\n" << "  Max:    " << cvtColorStats.max << " us\n";
        std::cout << "Blur:\n" << "  Mean:   " << blurStats.mean << " us\n" << "  StdDev: " << blurStats.stddev << " us\n" << "  Max:    " << blurStats.max << " us\n";
        std::cout << "Threshold:\n" << "  Mean:   " << ThresholdStats.mean << " us\n" << "  StdDev: " << ThresholdStats.stddev << " us\n" << "  Max:    " << ThresholdStats.max << " us\n";
        std::cout << "Contours:\n" << "  Mean:   " << contoursStats.mean << " us\n" << "  StdDev: " << contoursStats.stddev << " us\n" << "  Max:    " << contoursStats.max << " us\n";
        std::cout << "Contours Analysis:\n" << "  Mean:   " << contoursAnalysisStats.mean << " us\n" << "  StdDev: " << contoursAnalysisStats.stddev << " us\n" << "  Max:    " << contoursAnalysisStats.max << " us\n";
        std::cout << "Draw Ball:\n" << "  Mean:   " << drawBallStats.mean << " us\n" << "  StdDev: " << drawBallStats.stddev << " us\n" << "  Max:    " << drawBallStats.max << " us\n";
        std::cout << "Kalman:\n" << "  Mean:   " << kalmanStats.mean << " us\n" << "  StdDev: " << kalmanStats.stddev << " us\n" << "  Max:    " << kalmanStats.max << " us\n";
        std::cout << "Draw Prediction:\n" << "  Mean:   " << drawPredictionStats.mean << " us\n" << "  StdDev: " << drawPredictionStats.stddev << " us\n" << "  Max:    " << drawPredictionStats.max << " us\n";
        std::cout << "Process Noise:\n" << "  Mean:   " << processNoiseStats.mean << " us\n" << "  StdDev: " << processNoiseStats.stddev << " us\n" << "  Max:    " << processNoiseStats.max << " us\n";
        std::cout << "Speed Calc:\n" << "  Mean:   " << speedCalcStats.mean << " us\n" << "  StdDev: " << speedCalcStats.stddev << " us\n" << "  Max:    " << speedCalcStats.max << " us\n";
        std::cout << "Regression:\n" << "  Mean:   " << regressionStats.mean << " us\n" << "  StdDev: " << regressionStats.stddev << " us\n" << "  Max:    " << regressionStats.max << " us\n";
        std::cout << "Total Detection:\n" << "  Mean:   " << totalStats.mean << " us\n" << "  StdDev: " << totalStats.stddev << " us\n" << "  Max:    " << totalStats.max << " us\n";

        return true;
    }

    void PuttingSimPipeline::readerLoop() {
        while (true) {
            auto frameOpt = reader_->read();
            if (!frameOpt.has_value()) {
                running_ = false;
                frameCv_.notify_all();
                detectCv_.notify_all();
                return;
            }
            {
                std::unique_lock<std::mutex> lock(frameMutex_);
                if (frameQueue_.size() >= maxQueueSize_) {
                    frameQueue_.pop();
                }
                frameQueue_.push(std::move(*frameOpt));
            }
            frameCv_.notify_one();
        }
    }

    void PuttingSimPipeline::detectLoop() {
        while (true) {
            Frame frame;
            {
                std::unique_lock<std::mutex> lock(frameMutex_);
                frameCv_.wait(lock, [&] {
                    return !frameQueue_.empty() || !running_;
                });
                if (!running_ && frameQueue_.empty()) {
                    return;
                }
                frame = std::move(frameQueue_.front());
                frameQueue_.pop();
            }
            Detection det = detector_->detect(frame);
            auto eventType = stateMachine_.update(det);
            if (eventType.has_value()) {
                PipelineEvent event;
				event.type = *eventType;
				event.timestamp = std::chrono::system_clock::now();
                if (eventType == EventType::ShotDetected) {
                    if (det.speed && det.aim) {
                        event.data = DetectionData {
                            *det.speed,
                            *det.aim
                        };
                    }
                }
                eventSink_->publish(event);
                std::cout << "Event published\n";
            }
            {
                std::unique_lock<std::mutex> lock(detectMutex_);
                if (detectQueue_.size() >= maxQueueSize_) {
                    detectQueue_.pop();
                }
                detectQueue_.push(std::move(det));
            }
            detectCv_.notify_one();
        }
    }

    void PuttingSimPipeline::debugLoop() {
        while (true) {
            Detection det;
            {
                std::unique_lock<std::mutex> lock(detectMutex_);
                detectCv_.wait(lock, [&] {
                    return !detectQueue_.empty() || !running_;
                });
                if (!running_ && detectQueue_.empty()) {
                    return;
                }
                det = std::move(detectQueue_.front());
                detectQueue_.pop();
            }
            timings timingSample;
            timingSample.read = det.timings[READ];
            timingSample.setup = det.timings[SETUP];
            timingSample.cvtColor = det.timings[CVT_COLOR];
            timingSample.blur = det.timings[BLUR];
            timingSample.threshold = det.timings[THRESHOLD];
            timingSample.contours = det.timings[CONTOURS];
            timingSample.contourAnalysis = det.timings[CONTOUR_ANALYSIS];
            timingSample.drawBall = det.timings[DRAW_BALL];
            timingSample.kalman = det.timings[KALMAN];
            timingSample.drawPrediction = det.timings[DRAW_PREDICTION];
            timingSample.processNoise = det.timings[PROCESS_NOISE];
            timingSample.speedCalc = det.timings[SPEED_CALC];
            timingSample.regression = det.timings[REGRESSION];
            timingSample.total = det.timings[TOTAL];
            timing_history_.push_back(timingSample);
            cv::imshow(windowName_, det.frame);
            cv::waitKey(1);
        }
    }
}