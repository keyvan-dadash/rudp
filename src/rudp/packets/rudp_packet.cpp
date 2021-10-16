


#include <string>
#include <iostream>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rudp/packets/rudp_packet.hpp"


namespace rudp {


  namespace packets {


      RUDPPacket::RUDPPacket(
        struct sockaddr_in addr,
        rudp_packet_header_t header,
        std::string payload
      ) : header_(header),
          payload_(payload),
          addr(addr)
      {
        // std::cout << this->header_.seq_number << std::endl;

        this->whole_packet_ = this->convertHeaderToString() + this->payload_;

        // std::cout << this->whole_packet_ << std::endl;
      }

      RUDPPacket::RUDPPacket(
        struct sockaddr_in addr,
        std::string packet
      ) : whole_packet_(packet),
          addr(addr)
      {

      }


      void RUDPPacket::setPacketHeader(rudp_packet_header_t header)
      {
        this->header_ = header;
      }

      void RUDPPacket::setPacketPayload(std::string payload)
      {
        this->payload_ = payload;
      }
      
      const char* RUDPPacket::serializePacket()
      {
        return this->whole_packet_.c_str();
      }

      void RUDPPacket::marshalPacket()
      {
        const char *bytes = this->whole_packet_.c_str();

        this->header_.is_ack = static_cast<u_int8_t>(bytes[0]);

        char seq[4];
        for (int i = 0; i < 4; i++) {
          seq[i]= bytes[i + 1];
        }

        this->header_.seq_number = std::atoi(seq);

        this->payload_ = std::string(bytes + 5);
      }

      bool RUDPPacket::isAckPacket()
      {
        return (this->header_.is_ack & 1);
      }

      bool RUDPPacket::isSameSeq(u_int32_t seqNumber)
      {
        return (this->header_.seq_number == seqNumber);
      }

      struct sockaddr_in RUDPPacket::getAdd()
      {
        return this->addr;
      }

      std::string RUDPPacket::convertHeaderToString()
      {
        return (std::string(std::to_string(this->header_.is_ack)) + std::string(std::to_string(this->header_.seq_number)));
      }

  };

}