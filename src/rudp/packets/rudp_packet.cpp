


#include <string>

#include <sys/types.h>


#include "rudp/packets/rudp_packet.hpp"


namespace rudp {


  namespace packets {


      RUDPPacket::RUDPPacket(
        rudp_packet_header_t header,
        std::string payload
      ) : header_(header),
          payload_(payload)
      {
        this->whole_packet_ = this->convertHeaderToString() + this->payload_;
      }

      RUDPPacket::RUDPPacket(
        std::string packet
      ) : whole_packet_(packet)
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
        this->header_.is_ack = static_cast<u_int8_t>(bytes[0]);
        this->header_.seq_number = 1;

        for (int i = 0; i < 4; i++) {
          this->header_.seq_number = this->header_.seq_number << 8 | static_cast<unsigned char>(bytes[i + 1]);
        }

        this->payload_ = std::string(bytes + 6);
      }

      bool RUDPPacket::isAckPacket()
      {
        return (this->header_.is_ack & 1);
      }

      bool RUDPPacket::isSameSeq(u_int32_t seqNumber)
      {
        return (this->header_.seq_number == seqNumber);
      }

      std::string RUDPPacket::convertHeaderToString()
      {
        char ack_buff[1];
        char seq_buff[4];

        std::sprintf(ack_buff, "%d", this->header_.is_ack);

        seq_buff[0] = (this->header_.seq_number >> 24) & 0xFF;
        seq_buff[1] = (this->header_.seq_number >> 16) & 0xFF;
        seq_buff[2] = (this->header_.seq_number >> 8) & 0xFF;
        seq_buff[3] = this->header_.seq_number & 0xFF;


        std::sprintf(seq_buff, "%d%d%d%d", seq_buff[3], seq_buff[2], seq_buff[1], seq_buff[0]);


        char whole_buff[5];

        whole_buff[0] = ack_buff[0];

        whole_buff[1] = seq_buff[0];
        whole_buff[2] = seq_buff[1];
        whole_buff[3] = seq_buff[2];
        whole_buff[4] = seq_buff[3];


        return std::string(whole_buff);
      }

  };

}