#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>
#include "puttingsimulator/interfaces/detector.hpp"
#include "puttingsimulator/interfaces/frame_reader.hpp"
#include "puttingsimulator/interfaces/event_sink.hpp"
#include "puttingsimulator/pipeline/stats.hpp"
#include "puttingsimulator/pipeline/state_machine.hpp"

namespace puttingsimulator {

    class PuttingSimPipeline {

        public:
            PuttingSimPipeline(
                std::unique_ptr<FrameReader> reader,
                std::unique_ptr<BallDetector> detector,
                std::unique_ptr<EventSink> eventSink
            );

            void start();
            bool stop();

        private:
            void readerLoop();
            void detectLoop();
            void debugLoop();

        private:
            int maxQueueSize_;
            std::string windowName_;

            std::unique_ptr<FrameReader> reader_;
            std::unique_ptr<BallDetector> detector_;
            std::unique_ptr<EventSink> eventSink_;

            StateMachine stateMachine_;

            std::thread readerThread_;
            std::thread detectThread_;

            std::atomic<bool> running_{ false };

            std::queue<Frame> frameQueue_;
            std::queue<Detection> detectQueue_;

            std::mutex frameMutex_;
            std::mutex detectMutex_;

            std::condition_variable frameCv_;
            std::condition_variable detectCv_;

            std::vector<timings> timing_history_;
    };
}