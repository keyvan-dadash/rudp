

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "rudp/core/receiver.hpp"
#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"

namespace rudp {

  namespace core {

    RecvManager::RecvManager(rudp::utils::UDPSocket udp_socket) : socket_(udp_socket)
    {

    }

    rudp::packets::RUDPPacket RecvManager::recvPacket()
    {
      std::unique_lock<std::mutex> lk(this->m_);
      
      if (this->recv_packet_buffer_.empty()) {
        this->pac_cv_.wait(lk, [=](){ return !recv_packet_buffer_.empty();});
      }

      rudp::packets::RUDPPacket packet = this->recv_packet_buffer_.front();
      this->recv_packet_buffer_.pop();

      lk.unlock();

      return packet;
    }

    rudp::packets::RUDPPacket RecvManager::recvAckPacket()
    {
      std::unique_lock<std::mutex> lk(this->m_);

      if (this->recv_ack_packet_buffer_.empty()) {
        this->ack_cv_.wait(lk, [=](){ return !recv_ack_packet_buffer_.empty();});
      }

      rudp::packets::RUDPPacket packet = this->recv_ack_packet_buffer_.front();
      this->recv_ack_packet_buffer_.pop();

      lk.unlock();

      return packet;
    }

    void RecvManager::startRecvLoop()
    {
      while (true) 
      {
        std::string packet = this->socket_.recvPacket();
        rudp::packets::RUDPPacket rudp_packet(packet);
        rudp_packet.marshalPacket();

        std::unique_lock<std::mutex> lk(this->m_);

        if (rudp_packet.isAckPacket()) {
          this->recv_ack_packet_buffer_.push(rudp_packet);
          lk.unlock();
          this->ack_cv_.notify_all();
          continue;
        } 


        this->recv_packet_buffer_.push(rudp_packet);
        lk.unlock();
        this->pac_cv_.notify_all();
      }
    }

  };


}