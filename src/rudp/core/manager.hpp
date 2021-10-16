


#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"
#include "rudp/core/sender.hpp"
#include "rudp/core/receiver.hpp"

namespace rudp {

  namespace core {



    typedef struct PacketStatus
    {
      PacketStatus(rudp::packets::RUDPPacket packet,
                    std::chrono::milliseconds time) : packet(packet), created_time(time)
      {

      }
      rudp::packets::RUDPPacket packet;
      std::chrono::milliseconds created_time;
    } packet_status_t;


    class Manager
    {

      public:

        explicit Manager(rudp::utils::UDPSocket udp_socket);



        void sendPacket(rudp::packets::RUDPPacket packet, bool take_mutex = true);

        rudp::packets::RUDPPacket recvPacket();

        void flushAndSendAgain();


      private:

        std::vector<rudp::core::PacketStatus> window_;
        std::queue<rudp::packets::RUDPPacket> recv_packet_buffer_;
        rudp::core::SenderManager sender_;
        rudp::core::RecvManager receiver_;
        std::atomic_int32_t base;
        rudp::utils::UDPSocket socket_;

        std::mutex m_;
        std::condition_variable cv_;

        std::mutex recv_m_;
        std::condition_variable recv_cv_;


        void timer_loop();

        void ack_loop();

        void send_ack_loop();

        void start_manager();

    };

  };
}