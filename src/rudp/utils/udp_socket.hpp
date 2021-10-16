

#pragma once

#include <atomic>
#include <string>
#include <mutex>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



namespace rudp
{

  namespace utils {


    typedef struct RawPacket {
      std::string buff;
      struct sockaddr_in client_peer_;
    } raw_packet_t;


    class UDPSocket
    {

      public:

        explicit UDPSocket(
          std::string peer_addr,
          std::string peer_port);


        explicit UDPSocket(u_int32_t fd);


        void sendPacket(raw_packet_t packet);


        raw_packet_t recvPacket();

        void setToListenMode();

      private:

        u_int32_t fd_;
        int is_server;

        std::string peer_addr_;
        std::string peer_port_;

        struct sockaddr_in getAddrIn();


    };


  };

}