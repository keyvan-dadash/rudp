


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



void print_ipv4(struct sockaddr_in *saddr)
{
  printf("Numeric: %s\n", inet_ntoa(saddr->sin_addr));
  printf("sin_port: %d\n", saddr->sin_port);
  printf("saddr = %d, %s: %d\n", saddr->sin_family, inet_ntoa(saddr->sin_addr), saddr->sin_port);
  printf("s_addr: %d\n", ntohl(saddr->sin_addr.s_addr));
}


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
      this->is_server = 0;
    }


    UDPSocket::UDPSocket(u_int32_t fd) : fd_(fd)
    {
      this->is_server = 0;
    }


    void UDPSocket::sendPacket(raw_packet_t packet)
    {
      std::cout << "send" << std::endl;
      std::cout << packet.buff << std::endl;


      // peer = this->is_server >= 1 ? this->client_peer_ : this->getAddrIn();

      // print_ipv4(&(this->client_peer_));
      print_ipv4(&(packet.client_peer_));
      sendto(this->fd_, packet.buff.c_str(), packet.buff.size(),
        MSG_CONFIRM, (const struct sockaddr *) &(packet.client_peer_), 
            sizeof(packet.client_peer_));
    }


    raw_packet_t UDPSocket::recvPacket()
    {
      std::cout << "recvvv" << std::endl;
      char buffer[MAX_PACKET_SIZE];
      memset(buffer, '\0', MAX_PACKET_SIZE);
      struct sockaddr_in peer = this->getAddrIn();

      unsigned int len = sizeof(peer);

      recvfrom(this->fd_, (char *)buffer, MAX_PACKET_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &peer, &len);
      
      // if (this->is_server != 2 && this->is_server >= 1) {
      //   memset(&(this->client_peer_), 0, sizeof(this->client_peer_));
      //   this->client_peer_ = peer;
        print_ipv4(&(peer));
      //   std::cout << "i am here" << std::endl;
      //   this->is_server = 2;
      // }

      std::cout << buffer << std::endl;

      raw_packet_t packet;

      packet.buff = buffer;
      packet.client_peer_ = peer;

      return packet;
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