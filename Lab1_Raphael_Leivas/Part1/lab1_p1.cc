#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/names.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/random-variable-stream.h"

#define MAX_CLIENTS 5
#define STOP_TIME 20.0
#define SERVER_PORT 15
#define PACKET_SIZE 1024
#define NETWORK_BANDWIDTH "5Mbps"
#define NETWORK_PROPAGATION_DELAY "2ms"

using namespace ns3;

int
main(int argc, char* argv[])
{
    Ptr<UniformRandomVariable> randVar = CreateObject<UniformRandomVariable> (); 

    uint32_t nPackets = 1;
    uint32_t nClients = 1;

    CommandLine cmd;
    cmd.AddValue("nPackets", "Numero de pacotes para enviar", nPackets);
    cmd.AddValue("nClients", "Numero de clientes", nClients);
    cmd.Parse(argc, argv);

    if (nClients > MAX_CLIENTS)
    {
        nClients = MAX_CLIENTS;
    }

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // cria nClients nos para os clientes
    NodeContainer clients;
    clients.Create(nClients);

    // cira um no para o servidor
    NodeContainer server;
    server.Create(1);

    // stack internet para todo mundo
    InternetStackHelper stack;
    stack.Install(clients);
    stack.Install(server);

    // Rotula os nos para facilitar
    Names::Add("n0 (servidor)", server.Get(0));

    for (uint32_t i = 0; i < nClients; ++i)
    {
        Names::Add("n" + std::to_string(i + 1), clients.Get(i));
    }

    // faz a ponto a ponto
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(NETWORK_BANDWIDTH));
    pointToPoint.SetChannelAttribute("Delay", StringValue(NETWORK_PROPAGATION_DELAY));

    // endereçamento ipv4
    Ipv4AddressHelper address;
    NetDeviceContainer devices[nClients];
    Ipv4InterfaceContainer interfaces[nClients];

    for (uint32_t i = 0; i < nClients; ++i)
    {
        NodeContainer pair(clients.Get(i), server.Get(0));
        devices[i] = pointToPoint.Install(pair);

        std::ostringstream subnet;
        subnet << "10.1." << i + 1 << ".0";
        address.SetBase(Ipv4Address(subnet.str().c_str()), "255.255.255.0");
        interfaces[i] = address.Assign(devices[i]);
    }

    // porta do servidor
    UdpEchoServerHelper echoServer(SERVER_PORT);
    ApplicationContainer serverApp = echoServer.Install(server.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(STOP_TIME));

    // envia para a porta do servidor
    for (uint32_t i = 0; i < nClients; ++i)
    {
        UdpEchoClientHelper echoClient(interfaces[i].GetAddress(1), SERVER_PORT);
        echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(PACKET_SIZE));

        ApplicationContainer clientApp = echoClient.Install(clients.Get(i));
        clientApp.Start(Seconds(static_cast<int>(randVar->GetInteger (2, 7)))); // inteiro aleatorio entre 2 e 7
        clientApp.Stop(Seconds(STOP_TIME));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // define o XML para onde vai a animação
    AnimationInterface anim("lab1_p1.xml");

    // Posiciona os clientes em linha em y = 10
    for (uint32_t i = 0; i < nClients; ++i)
    {
        anim.SetConstantPosition(clients.Get(i), 10.0 + 10.0 * i, 10.0);
    }

    // posiciona o servidor na media aritmetica deles, embaixo
    anim.SetConstantPosition(server.Get(0), (10.0 + nClients * 10.0) / 2, 30.0);

    Simulator::Stop(Seconds(STOP_TIME));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
