#pragma once

#include <winsock2.h>
#include <string>
#include "puttingsimulator/interfaces/event_sink.hpp"

namespace puttingsimulator {

    class UdpEventSender : public EventSink {

        public:
			bool initialize() override;
			bool initialize_udp_sender(int port);
            void publish(const PipelineEvent& event) override;
            void sendUdp(std::string message);
			void clean_up() override;
            bool clean_up_udp_sender();

        private:
            SOCKET sock_;
            sockaddr_in unityAddr_{};
    };
}