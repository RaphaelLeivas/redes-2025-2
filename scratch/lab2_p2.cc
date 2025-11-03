#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    NodeContainer nodes;
    nodes.Create(4); // 0: Client, 1: Router, 2: Server A, 3: Server B

    // Links from Router to Client/Servers
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NodeContainer n0n1 = NodeContainer(nodes.Get(0), nodes.Get(1));
    NodeContainer n1n2 = NodeContainer(nodes.Get(1), nodes.Get(2));
    NodeContainer n1n3 = NodeContainer(nodes.Get(1), nodes.Get(3));

    NetDeviceContainer d0d1 = p2p.Install(n0n1);
    NetDeviceContainer d1d2 = p2p.Install(n1n2);
    NetDeviceContainer d1d3 = p2p.Install(n1n3);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer if0if1 = address.Assign(d0d1);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer if1if2 = address.Assign(d1d2);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer if1if3 = address.Assign(d1d3);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t portA = 5000;
    uint16_t portB = 5001;

    // Server A
    PacketSinkHelper sinkA("ns3::TcpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), portA));
    ApplicationContainer serverA = sinkA.Install(nodes.Get(2));
    serverA.Start(Seconds(0.0));
    serverA.Stop(Seconds(10.0));

    // Server B
    PacketSinkHelper sinkB("ns3::TcpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), portB));
    ApplicationContainer serverB = sinkB.Install(nodes.Get(3));
    serverB.Start(Seconds(0.0));
    serverB.Stop(Seconds(10.0));

    // Client sends to Server A
    BulkSendHelper sourceA("ns3::TcpSocketFactory",
                           InetSocketAddress(if1if2.GetAddress(1), portA));
    sourceA.SetAttribute("MaxBytes", UintegerValue(5)); // unlimited
    ApplicationContainer clientA = sourceA.Install(nodes.Get(0));
    clientA.Start(Seconds(1.0));
    clientA.Stop(Seconds(10.0));

    // Client sends to Server B
    BulkSendHelper sourceB("ns3::TcpSocketFactory",
                           InetSocketAddress(if1if3.GetAddress(1), portB));
    sourceB.SetAttribute("MaxBytes", UintegerValue(5)); // unlimited
    ApplicationContainer clientB = sourceB.Install(nodes.Get(0));
    clientB.Start(Seconds(1.0));
    clientB.Stop(Seconds(10.0));

    // define o XML para onde vai a animação
    AnimationInterface anim("lab2_p2.xml");

    // // Posiciona os clientes em linha em y = 10
    // for (uint32_t i = 0; i < nClients; ++i)
    // {
    //     anim.SetConstantPosition(clients.Get(i), 10.0 + 10.0 * i, 10.0);
    // }

    // // posiciona o servidor na media aritmetica deles, embaixo
    // anim.SetConstantPosition(server.Get(0), (10.0 + nClients * 10.0) / 2, 30.0);

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
