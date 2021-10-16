




#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"
#include "rudp/core/sender.hpp"
#include "rudp/core/receiver.hpp"
#include "rudp/core/manager.hpp"


#define MAX_WAIT_MS 1000

namespace rudp {

  namespace core {


    Manager::Manager(rudp::utils::UDPSocket udp_socket) : socket_(udp_socket),
                                                          sender_(rudp::core::SenderManager(udp_socket)),
                                                          receiver_(rudp::core::RecvManager(udp_socket))
    {
      this->start_manager();
    }



    void Manager::sendPacket(rudp::packets::RUDPPacket packet)
    {
      this->sender_.sendPacket(packet);


      auto duration = std::chrono::system_clock::now().time_since_epoch();

      packet_status_t packet_s(packet, std::chrono::duration_cast< std::chrono::milliseconds >( duration ));

      this->window_.push_back(packet_s);
    }

    rudp::packets::RUDPPacket Manager::recvPacket()
    {
      std::unique_lock<std::mutex> lk(this->recv_m_);

      if (this->recv_packet_buffer_.empty()) {
        this->recv_cv_.wait(lk, [=](){ return !recv_packet_buffer_.empty(); });
      }

      rudp::packets::RUDPPacket packet = this->recv_packet_buffer_.front();
      this->recv_packet_buffer_.pop();

      return packet;
    }

    void Manager::flushAndSendAgain()
    {
      std::vector<rudp::core::PacketStatus> temp;
      temp = this->window_;

      this->window_.clear();

      for (auto &item : temp) {
        this->sendPacket(item.packet);
      }
    }


    void Manager::timer_loop()
    {
      while (true)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(MAX_WAIT_MS));

        std::unique_lock<std::mutex> lk(this->m_);
        auto duration = std::chrono::system_clock::now().time_since_epoch();

        auto now_ms = std::chrono::duration_cast< std::chrono::milliseconds >( duration );

        if (this->window_.empty()) continue;

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now_ms - this->window_[0].created_time).count() > MAX_WAIT_MS) {
          this->flushAndSendAgain();
        }

      }
    }

    void Manager::ack_loop()
    {
      while (true) {
        std::unique_lock<std::mutex> lk(this->m_);

        rudp::packets::RUDPPacket ack_packet = this->receiver_.recvAckPacket();

        for (int i = 0; i < this->window_.size(); i++) {

          if (this->window_[i].packet.isSameSeq(ack_packet.header_.seq_number)) {
            this->window_.erase(this->window_.begin() + i);
            break;
          }
        }
      }
    }


    void Manager::send_ack_loop()
    {
      while(true)
      {
        rudp::packets::RUDPPacket recv_packet = this->receiver_.recvPacket();

        std::unique_lock<std::mutex> lk(this->recv_m_);

        this->recv_packet_buffer_.push(recv_packet);

        rudp::packets::rudp_packet_header_t header;

        header.is_ack = 1;
        header.seq_number = recv_packet.header_.seq_number;

        rudp::packets::RUDPPacket ack_packet(
          header,
          std::string("")
        );

        this->sender_.sendPacket(ack_packet);

        lk.unlock();
        this->recv_cv_.notify_all();
      }
    }

    void Manager::start_manager()
    {

      std::thread sender_thread(&rudp::core::SenderManager::startSendLoop, &(this->sender_));
      std::thread recv_thread(&rudp::core::RecvManager::startRecvLoop, &(this->receiver_));
      std::thread timer_thread(&rudp::core::Manager::timer_loop, this);
      std::thread recv_ack_thread(&rudp::core::Manager::ack_loop, this);
      std::thread send_ack_thread(&rudp::core::Manager::send_ack_loop, this);

      sender_thread.detach();
      recv_thread.detach();
      timer_thread.detach();
      recv_ack_thread.detach();
      send_ack_thread.detach();
    }

  };
}