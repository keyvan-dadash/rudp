


#include <iostream>


#include "rudp/packets/rudp_packet.hpp"
#include "rudp/utils/udp_socket.hpp"
#include "rudp/core/sender.hpp"
#include "rudp/core/receiver.hpp"
#include "rudp/core/manager.hpp"


void do_client(rudp::core::Manager& manager)
{
  int init_seq = 2423;

  std::string msg;
  while (true) 
  {
    msg.clear();
    std::cin >> msg;

    rudp::packets::rudp_packet_header_t header;

    header.is_ack = 0;
    header.seq_number = init_seq;

    rudp::packets::RUDPPacket packet(
      header,
      msg
    );
    init_seq++;

    manager.sendPacket(packet);
  }
}

void do_server(rudp::core::Manager& manager)
{
  while (true) 
  {
    rudp::packets::RUDPPacket packet = manager.recvPacket();

    std::cout << packet.payload_ << std::endl;
  }
}

int main(int argc, char *argv[]) 
{
  bool is_server = false;
  std::string addr = std::string(argv[1]);
  std::string port = std::string(argv[2]);

  rudp::utils::UDPSocket udp_socket(addr, port);

  if (std::stoi(addr) == -1) {
    udp_socket.setToListenMode();
    is_server = true;
  }


  rudp::core::Manager manager(udp_socket);

  if (is_server) {
    do_server(manager);
  } else {
    do_client(manager);
  }


}