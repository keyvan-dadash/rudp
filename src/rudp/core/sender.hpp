


#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>


#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"

namespace rudp {

  namespace core {


    class SenderManager
    {

      public:
      
        SenderManager(rudp::utils::UDPSocket udp_socket);


        void sendPacket(rudp::packets::RUDPPacket packet);

        void startSendLoop();

      private:

        std::queue<rudp::packets::RUDPPacket> sender_buffer_;
        std::mutex m_;
        std::condition_variable cv_;
        rudp::utils::UDPSocket socket_;

    };

  }


}