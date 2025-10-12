#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#define MAX_PACKETS 20
#define SERVER_PORT 9

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HybridNetworkExample");

int
main(int argc, char* argv[])
{
    CommandLine cmd;

    uint32_t nPackets = 1;
    uint32_t nCsma = 1;

    cmd.AddValue("nPackets", "Numero de pacotes para enviar", nPackets);
    cmd.AddValue("nCsma", "Numero de clientes", nCsma);

    cmd.Parse(argc, argv);

    if (nPackets > MAX_PACKETS) {
        nPackets = MAX_PACKETS;
    }

    // Create nodes
    NodeContainer first_p2p_nodes;
    first_p2p_nodes.Create(2); // n0 e n1

    NodeContainer csmaNodes;
    csmaNodes.Add(first_p2p_nodes.Get(1)); // n1 faz parte dos CSMA
    csmaNodes.Create(nCsma); // add os outros CSMA do nCsma

    NodeContainer second_p2p_nodes;
    second_p2p_nodes.Add(csmaNodes.Get(csmaNodes.GetN() - 1)); // ultimo no CSMA
    second_p2p_nodes.Create(1);                                // servidor

    // stack internet para todo mundo
    InternetStackHelper stack;
    stack.InstallAll();

    // primeiro ponto a ponto entre n0 e n1
    PointToPointHelper first_p2p;
    first_p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    first_p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer first_p2p_net = first_p2p.Install(first_p2p_nodes);

    // sequencia de nós CSMA
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("1ms"));
    NetDeviceContainer csmaDevices = csma.Install(csmaNodes);

    // segundo ponto a ponto entre o ultimo CSMA e o servidor final
    PointToPointHelper second_p2p;
    second_p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    second_p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer second_p2p_net = second_p2p.Install(second_p2p_nodes);

    // endereço IP
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer first_p2p_interface = address.Assign(first_p2p_net);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces = address.Assign(csmaDevices);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer second_p2p_interface = address.Assign(second_p2p_net);

    // add servidor no final dos CSMA
    UdpEchoServerHelper echoServer(SERVER_PORT);
    ApplicationContainer serverApp = echoServer.Install(second_p2p_nodes.Get(1));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // add cliente que chama o servidor no inicio dos CSMA
    UdpEchoClientHelper echoClient(second_p2p_interface.GetAddress(1), SERVER_PORT);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = echoClient.Install(first_p2p_nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // define o XML para onde vai a animação
    AnimationInterface anim("lab1_p2.xml");

    // Posiciona o cliente inicial  em y = 10
    anim.SetConstantPosition(first_p2p_nodes.Get(0), 10.0, 10.0);

    // Posiciona os CSMA em linha em y = 20
    for (uint32_t i = 0; i < nCsma + 1; ++i)
    {
        anim.SetConstantPosition(csmaNodes.Get(i), 20.0 + 10.0 * i, 20.0);
    }

    // Posiciona o servidor final em y = 10 ao final
    anim.SetConstantPosition(second_p2p_nodes.Get(1), 20.0 + 10.0 * (nCsma + 1), 10.0);

    //  roda o simulador
    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
