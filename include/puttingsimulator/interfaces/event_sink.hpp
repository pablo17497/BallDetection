#pragma once

#include "puttingsimulator/models/event.hpp"

namespace puttingsimulator {

    class EventSink {

        public:
            virtual ~EventSink() = default;
			virtual bool initialize() = 0;
            virtual void publish(const PipelineEvent& event) = 0;
			virtual void clean_up() = 0;
    };
}