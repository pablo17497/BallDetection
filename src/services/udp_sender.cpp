#include <ws2tcpip.h>
#include <iostream>
#include "puttingsimulator/services/udp_sender.hpp"
#include "puttingsimulator/services/message_serializer.hpp"

#pragma comment(lib, "ws2_32.lib")

namespace puttingsimulator {

	bool UdpEventSender::initialize() {
		int port = 5005; // Unity listens on this port
		return initialize_udp_sender(port);
	}

    bool UdpEventSender::initialize_udp_sender(int port) {
        
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

        if (result != 0) {
            std::cout << "WSAStartup failed\n";
            return false;
        }

        sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (sock_ == INVALID_SOCKET) {
            std::cout << "Socket creation failed\n";
            WSACleanup();
            return false;
        }

        unityAddr_.sin_family = AF_INET;
        unityAddr_.sin_port = htons(port); // Unity listens on this port

        inet_pton(AF_INET, "127.0.0.1", &unityAddr_.sin_addr);
        return true;
    }
    
    void UdpEventSender::publish(const PipelineEvent& event) {
        
        Message msg;

        msg.timestamp = event.timestamp;

        switch (event.type) {
        case EventType::CalibrationStarted:
            msg.type = MessageType::Calibration;
            msg.payload = StatusPayload{ Status::Started };
            break;
        case EventType::CalibrationFinished:
            msg.type = MessageType::Calibration;
            msg.payload = StatusPayload{ Status::Finished };
            break;
        case EventType::BallDetected:
            msg.type = MessageType::BallDetected;
            msg.payload = StatusPayload{ Status::BallFound };
            break;
        case EventType::BallLost:
            msg.type = MessageType::BallLost;
            msg.payload = StatusPayload{ Status::Searching };
            break;
        case EventType::ShotDetected:
            if (event.data) {
                msg.type = MessageType::ShotDetected;
                msg.payload = ShotPayload{
                    event.data->speed,
                    event.data->aim.x,
                    event.data->aim.y
                };
            }
			else {
				msg.type = MessageType::ShotDetected;
				msg.payload = StatusPayload{ Status::DataMissing };
			}
            break;
        default:
            msg.type = MessageType::EventNotRecognized;
            msg.payload = StatusPayload{ Status::EventNotRecognized };
            break;
        }

        sendUdp(MessageSerializer::toJson(msg));
    }

    void UdpEventSender::sendUdp(std::string message) {
        
        sendto(
            sock_,
            message.c_str(),
            (int)message.size(),
            0,
            (sockaddr*)&unityAddr_,
            sizeof(unityAddr_)
        );
        std::cout << "Packet sent\n" << message << "\n";
    }

	void UdpEventSender::clean_up() {
		clean_up_udp_sender();
	}

    bool UdpEventSender::clean_up_udp_sender() {
        
        closesocket(sock_);
        WSACleanup();

        return true;
    }
}