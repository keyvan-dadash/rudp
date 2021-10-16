


#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>


#include "rudp/core/sender.hpp"
#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"

namespace rudp {

  namespace core {


    SenderManager::SenderManager(rudp::utils::UDPSocket udp_socket) : socket_(udp_socket)
    {

    }

    //push packet to sender buffer
    void SenderManager::sendPacket(rudp::packets::RUDPPacket packet)
    {
      std::unique_lock<std::mutex> lk(this->m_);

      this->sender_buffer_.push(packet);

      lk.unlock();
      this->cv_.notify_one();
    }

    //loop forever
    //we should dequeue then send packet after that 
    void SenderManager::startSendLoop()
    {

      while (true) 
      {
        std::unique_lock<std::mutex> lk(this->m_);
        if (this->sender_buffer_.empty()) {
          this->cv_.wait(lk, [=](){ return !sender_buffer_.empty();});
        }

        rudp::packets::RUDPPacket packet = this->sender_buffer_.front();
        this->sender_buffer_.pop();

        rudp::utils::raw_packet_t raw_packet;

        raw_packet.buff = packet.serializePacket();
        raw_packet.client_peer_ = packet.getAdd();

        this->socket_.sendPacket(raw_packet);
      }
    }

  }


}