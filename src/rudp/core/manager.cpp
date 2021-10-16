




#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <iostream>

#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"
#include "rudp/core/sender.hpp"
#include "rudp/core/receiver.hpp"
#include "rudp/core/manager.hpp"


#define MAX_WAIT_MS 1000
#define MAX_WINDOW_SIZE 5

namespace rudp {

  namespace core {


    Manager::Manager(rudp::utils::UDPSocket udp_socket) : socket_(udp_socket),
                                                          sender_(rudp::core::SenderManager(udp_socket)),
                                                          receiver_(rudp::core::RecvManager(udp_socket))
    {
      this->base.store(INT32_MAX, std::memory_order_seq_cst);

      //threads start here
      this->start_manager();
    }


    //send packet
    //this is user api's
    void Manager::sendPacket(rudp::packets::RUDPPacket packet, bool take_mutex)
    {
      //check if window is already full
      if (this->window_.size() >= MAX_WINDOW_SIZE) {
        std::cout << "WARNING: Cannot send more packet because window is full" << std::endl;
        return;
      }

      // std::cout << packet.payload_ << std::endl;

      this->sender_.sendPacket(packet);

      if (this->base.load(std::memory_order_seq_cst) > packet.header_.seq_number) {
        this->base.store(packet.header_.seq_number, std::memory_order_seq_cst);
      }

      //we use this if we want flush and send again whole windows
      if (take_mutex)
        std::unique_lock<std::mutex> lk(this->m_);

      auto duration = std::chrono::system_clock::now().time_since_epoch();

      packet_status_t packet_s(packet, std::chrono::duration_cast< std::chrono::milliseconds >( duration ));

      this->window_.push_back(packet_s);
    }

    //recv packet
    //user api recv packet
    //it will wait until there is packet for recv
    //it will get packet from queue not directly from socket
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

    //when timer is up
    //we should send all packet in then windows
    //becuase we couldn't recv ack
    void Manager::flushAndSendAgain()
    {
      std::vector<rudp::core::PacketStatus> temp;
      temp = this->window_;

      this->window_.clear();

      for (auto &item : temp) {
        this->sendPacket(item.packet, false);
      }
    }


    //timer for chekcing if packet ack not recved
    void Manager::timer_loop()
    {
      while (true)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(MAX_WAIT_MS));

        std::unique_lock<std::mutex> lk(this->m_);
        auto duration = std::chrono::system_clock::now().time_since_epoch();

        auto now_ms = std::chrono::duration_cast< std::chrono::milliseconds >( duration );

        // std::cout << this->window_.size() << std::endl;

        if (this->window_.empty()) continue;

        //first packet inside windows is older one
        //so for simplity we only compare older packet with timer
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now_ms - this->window_[0].created_time).count() > MAX_WAIT_MS) {
          this->flushAndSendAgain();
        }

      }
    }

    void Manager::ack_loop()
    {
      while (true) {

        rudp::packets::RUDPPacket ack_packet = this->receiver_.recvAckPacket();

        std::unique_lock<std::mutex> lk(this->m_);

        // std::cout << this->window_[0].packet.header_.seq_number << std::endl;

        if (this->window_[0].packet.isSameSeq(ack_packet.header_.seq_number)) {
          this->base.store(ack_packet.header_.seq_number, std::memory_order_seq_cst);
          this->window_.erase(this->window_.begin());
        }



        // for (int i = 0; i < this->window_.size(); i++) {

        //   if (this->window_[i].packet.isSameSeq(ack_packet.header_.seq_number)) {
        //     this->window_.erase(this->window_.begin() + i);
        //     break;
        //   }
        // }
      }
    }


    //recv packets
    //and send acks
    void Manager::send_ack_loop()
    {
      while(true)
      {
        rudp::packets::RUDPPacket recv_packet = this->receiver_.recvPacket();

        std::unique_lock<std::mutex> lk(this->recv_m_);

        this->recv_packet_buffer_.push(recv_packet);

        rudp::packets::rudp_packet_header_t header;

        header.is_ack = 1;

        //for test
        // if (recv_packet.header_.seq_number % 2 == 0) {
        //   lk.unlock();
        //   this->recv_cv_.notify_all();
        //   continue;
        // }


        if (this->base.load(std::memory_order_seq_cst) >= recv_packet.header_.seq_number)
          header.seq_number = recv_packet.header_.seq_number;
        else
          header.seq_number = this->base.load(std::memory_order_seq_cst);

        rudp::packets::RUDPPacket ack_packet(
          recv_packet.getAdd(),
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

      //sender thread for sending packet
      std::thread sender_thread(&rudp::core::SenderManager::startSendLoop, &(this->sender_));

      //recv thread for recving packets
      std::thread recv_thread(&rudp::core::RecvManager::startRecvLoop, &(this->receiver_));

      //timer thread for resend if ack not recved in time
      std::thread timer_thread(&rudp::core::Manager::timer_loop, this);

      //recv ack thread
      std::thread recv_ack_thread(&rudp::core::Manager::ack_loop, this);

      //send ack thread
      std::thread send_ack_thread(&rudp::core::Manager::send_ack_loop, this);

      sender_thread.detach();
      recv_thread.detach();
      timer_thread.detach();
      recv_ack_thread.detach();
      send_ack_thread.detach();
    }

  };
}