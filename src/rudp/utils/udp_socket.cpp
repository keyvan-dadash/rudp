


#include <string>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>

#include "rudp/utils/udp_socket.hpp"


#define MAX_PACKET_SIZE 4096


namespace rudp
{

  namespace utils {


    UDPSocket::UDPSocket(
      std::string peer_addr,
      std::string peer_port) : 
                  peer_addr_(peer_addr),                    
                  peer_port_(peer_port)
    {
      this->fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    }


    UDPSocket::UDPSocket(u_int32_t fd) : fd_(fd)
    {

    }


    void UDPSocket::sendPacket(std::string packet)
    {
      struct sockaddr_in peer = this->getAddrIn();
      sendto(this->fd_, packet.c_str(), packet.size(),
        MSG_CONFIRM, (const struct sockaddr *) &peer, 
            sizeof(peer));
    }


    std::string UDPSocket::recvPacket()
    {
      char buffer[MAX_PACKET_SIZE];
      memset(buffer, '\0', MAX_PACKET_SIZE);
      struct sockaddr_in peer = this->getAddrIn();

      unsigned int len = sizeof(peer);

      recvfrom(this->fd_, (char *)buffer, MAX_PACKET_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &peer, &len);

      std::cout << buffer << std::endl;

      return std::string(buffer);
    }

    struct sockaddr_in UDPSocket::getAddrIn()
    {
      struct sockaddr_in peer;

      memset(&peer, 0, sizeof(peer));

      peer.sin_family = AF_INET;
      peer.sin_port = htons(std::stoi(this->peer_port_));

      inet_pton(AF_INET, this->peer_addr_.c_str(), &(peer.sin_addr));

      return peer;
    }

    void UDPSocket::setToListenMode()
    {
      struct sockaddr_in servaddr;

      servaddr.sin_family    = AF_INET;
      servaddr.sin_addr.s_addr = INADDR_ANY;
      servaddr.sin_port = htons(std::stoi(this->peer_port_));


      if ( bind(this->fd_, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
      {
          perror("bind failed");
          exit(-1);
      }
    }


  };

}