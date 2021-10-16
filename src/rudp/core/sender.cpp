


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


    void SenderManager::sendPacket(rudp::packets::RUDPPacket packet)
    {
      std::unique_lock<std::mutex> lk(this->m_);

      this->sender_buffer_.push(packet);

      lk.unlock();
      this->cv_.notify_one();
    }

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

        this->socket_.sendPacket(std::string(packet.serializePacket()));
      }
    }

  }


}