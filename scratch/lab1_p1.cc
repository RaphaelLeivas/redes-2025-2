#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/names.h"

#define MAX_CLIENTS 5
#define STOP_TIME 20.0
#define SERVER_PORT 15

using namespace ns3;

int main (int argc, char *argv[])
{
  uint32_t nPackets = 1;
  uint32_t nClients = 1;

  CommandLine cmd;
  cmd.AddValue("nPackets", "Number of packets to echo", nPackets);
  cmd.AddValue("nClients", "Number of clients to the server", nClients);
  cmd.Parse (argc, argv);

  if (nClients > MAX_CLIENTS) {
    nClients = MAX_CLIENTS;
  }

  // Create 4 nodes: 3 clients + 1 server
  NodeContainer clients;
  clients.Create (nClients);
  NodeContainer server;
  server.Create (1);

  // Install Internet stack on all nodes
  InternetStackHelper stack;
  stack.Install (clients);
  stack.Install (server);

  // Give names to nodes
  Names::Add ("n0 (servidor)", server.Get (0));

  for (uint32_t i = 0; i < nClients; ++i) {
    Names::Add ("n" + std::to_string(i + 1), clients.Get (i));
  }

  // Helper objects
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  Ipv4AddressHelper address;
  NetDeviceContainer devices[nClients];
  Ipv4InterfaceContainer interfaces[nClients];

  for (uint32_t i = 0; i < nClients; ++i)
  {
    // Create link between client[i] and server
    NodeContainer pair (clients.Get(i), server.Get(0));
    devices[i] = p2p.Install (pair);

    std::ostringstream subnet;
    subnet << "10.1." << i+1 << ".0";
    address.SetBase (Ipv4Address (subnet.str().c_str()), "255.255.255.0");
    interfaces[i] = address.Assign (devices[i]);
  }

  // Server: listens on all interfaces
  UdpEchoServerHelper echoServer (SERVER_PORT);
  ApplicationContainer serverApp = echoServer.Install (server.Get(0));
  serverApp.Start (Seconds (1.0));
  serverApp.Stop (Seconds (STOP_TIME));

  // Clients: each sends a packet to the server’s corresponding interface
  for (uint32_t i = 0; i < nClients; ++i)
  {
    UdpEchoClientHelper echoClient (interfaces[i].GetAddress (1), SERVER_PORT);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (nPackets));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApp = echoClient.Install (clients.Get(i));
    clientApp.Start (Seconds (2 + std::rand() % 6)); // aleatorio entre 2 e 7
    clientApp.Stop (Seconds (STOP_TIME));
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    AnimationInterface anim("lab1_p1.xml");

    // Posiciona os clientes em linha em y = 10
    for (uint32_t i = 0; i < nClients; ++i)
    {
      anim.SetConstantPosition(clients.Get(i), 10.0 + 10.0 * i, 10.0);
    }

    // posiciona o servidor na media aritmetica deles, embaixo
    anim.SetConstantPosition(server.Get(0), (10.0 + nClients * 10.0) / 2, 30.0);

  Simulator::Stop (Seconds (STOP_TIME));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
