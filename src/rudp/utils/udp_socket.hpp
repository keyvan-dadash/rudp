

#pragma once


#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



namespace rudp
{

  namespace utils {



    class UDPSocket
    {

      public:

        explicit UDPSocket(
          std::string peer_addr,
          std::string peer_port);


        explicit UDPSocket(u_int32_t fd);


        void sendPacket(std::string packet);


        std::string recvPacket();

        void setToListenMode();

      private:

        u_int32_t fd_;

        std::string peer_addr_;
        std::string peer_port_;

        struct sockaddr_in getAddrIn();


    };


  };

}