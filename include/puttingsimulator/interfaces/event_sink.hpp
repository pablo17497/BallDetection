#pragma once

#include "puttingsimulator/models/event.hpp"

namespace puttingsimulator {

    class EventSink {

        public:
            virtual ~EventSink() = default;

            virtual void publish(const PipelineEvent& event) = 0;
    };
}