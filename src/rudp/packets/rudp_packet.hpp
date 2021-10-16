

#pragma once

#include <string>


#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace rudp {


  namespace packets {


  
    typedef struct RUDPPacket_Header 
    {
      u_int8_t  is_ack;
      u_int32_t seq_number;
    } rudp_packet_header_t;

    class RUDPPacket
    {

      public:

        explicit RUDPPacket(
          struct sockaddr_in addr,
          rudp_packet_header_t header,
          std::string payload
        );

        explicit RUDPPacket(
          struct sockaddr_in addr,
          std::string packet
        );


        void setPacketHeader(rudp_packet_header_t header);

        void setPacketPayload(std::string payload);
        
        const char* serializePacket();

        void marshalPacket();

        bool isAckPacket();

        bool isSameSeq(u_int32_t seqNumber);

        struct sockaddr_in getAdd();

        rudp_packet_header_t header_;

        std::string payload_;
      private:

        std::string whole_packet_;

        struct sockaddr_in addr;

        std::string convertHeaderToString();

    };

  };

}