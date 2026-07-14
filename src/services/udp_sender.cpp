#include <ws2tcpip.h>
#include <iostream>
#include "puttingsimulator/services/udp_sender.hpp"

#pragma comment(lib, "ws2_32.lib")

namespace puttingsimulator {

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
        
        switch (event.type) {
            case EventType::ShotStarted:
                sendUdp("SHOT_STARTED");
                break;

            case EventType::ShotFinished:
                sendUdp("SHOT_FINISHED");
                break;

            default:
                break;
        }
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
        std::cout << "Packet sent\n";
    }

    bool UdpEventSender::clean_up_udp_sender() {
        
        closesocket(sock_);
        WSACleanup();

        return true;
    }
}