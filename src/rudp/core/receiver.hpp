


#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>


#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"

namespace rudp {

  namespace core {


    class RecvManager
    {

      public:

        RecvManager(rudp::utils::UDPSocket udp_socket);


        rudp::packets::RUDPPacket recvPacket();

        rudp::packets::RUDPPacket recvAckPacket();

        void startRecvLoop();

      private:

        std::queue<rudp::packets::RUDPPacket> recv_packet_buffer_;
        std::queue<rudp::packets::RUDPPacket> recv_ack_packet_buffer_;
        std::mutex m_;
        std::condition_variable ack_cv_;
        std::condition_variable pac_cv_;
        rudp::utils::UDPSocket socket_;

    };

  }


}