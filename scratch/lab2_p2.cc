#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

#define N_NODES 5
#define MAX_FLOWS 20

int
main(int argc, char* argv[])
{
    RngSeedManager::SetSeed(time(NULL));

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    std::string dataRate = "1Mbs";
    std::string delay = "20ms";
    double errorRate = 0.00001;
    uint32_t nFlows = 6;
    std::string transport_prot = "ns3::TcpCubic"; // ou ns3::TcpNewReno
    double simTime = 10.0;

    CommandLine cmd;
    cmd.AddValue("dataRate", "Taxa de envio de dados", dataRate);
    cmd.AddValue("delay", "Atraso no envio de dados", delay);
    cmd.AddValue("errorRate", "Taxa de erros", errorRate);
    cmd.AddValue("nFlows", "Numero de fluxos", nFlows);
    cmd.AddValue("transport_prot", "Protocolo", transport_prot);
    cmd.Parse(argc, argv);

    if (nFlows > MAX_FLOWS)
    {
        nFlows = MAX_FLOWS;
    }

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(transport_prot));

    NodeContainer nodes;
    nodes.Create(N_NODES); // 0: dest, 1: bottleneck1 b1,  2: bottleneck1 b2, 3: Servidor A, 4: Servidor B

    /*  --------- Links entre os nos -----------   */
    // entre dest e b1
    PointToPointHelper p2p_db1;
    p2p_db1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p_db1.SetChannelAttribute("Delay", StringValue("0.01ms"));

    // entre b1 e b2 - bottleneck
    PointToPointHelper p2p_b1b2;
    p2p_b1b2.SetDeviceAttribute("DataRate", StringValue(dataRate));
    p2p_b1b2.SetChannelAttribute("Delay", StringValue(delay));
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (errorRate));
    p2p_b1b2.SetDeviceAttribute("ReceiveErrorModel", PointerValue(em));

    // entre b2 e SA
    PointToPointHelper p2p_b2sa;
    p2p_b2sa.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p_b2sa.SetChannelAttribute("Delay", StringValue("0.01ms"));

    // entre b2 e SB
    PointToPointHelper p2p_b2sb;
    p2p_b2sb.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p_b2sb.SetChannelAttribute("Delay", StringValue("50ms"));

    /*  --------- containers dos nos -----------   */

    NodeContainer node_db1 = NodeContainer(nodes.Get(0), nodes.Get(1));
    NodeContainer node_b1b2 = NodeContainer(nodes.Get(1), nodes.Get(2));
    NodeContainer node_b2sa = NodeContainer(nodes.Get(2), nodes.Get(3));
    NodeContainer node_b2sb = NodeContainer(nodes.Get(2), nodes.Get(4));

    NetDeviceContainer net_db1 = p2p_db1.Install(node_db1);
    NetDeviceContainer net_b1b2 = p2p_b1b2.Install(node_b1b2);
    NetDeviceContainer net_b2sa = p2p_b2sa.Install(node_b2sa);
    NetDeviceContainer net_b2sb = p2p_b2sb.Install(node_b2sb);

    /*  --------- endereçamento IP ----------   */

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface_db1 = address.Assign(net_db1);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface_b1b2 = address.Assign(net_b1b2);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interface_b2sa = address.Assign(net_b2sa);

    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer interface_b2sb = address.Assign(net_b2sb);

    // sink: destino de um fluxo de dados
    // Vector to store the sink applications for later goodput calculation
    std::vector<Ptr<PacketSink>> sinks;

    uint16_t port = 50000;
    uint32_t halfFlows = nFlows / 2;

    // configura a aplicação sink no servidor A
    for (uint32_t i = 0; i < halfFlows; i++)
    {
        Address sinkAddr(InetSocketAddress(interface_b2sa.GetAddress(1), port + i));
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddr);
        ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(3));
        sinkApp.Start(Seconds(1.0));
        sinkApp.Stop(Seconds(simTime));

        sinks.push_back(DynamicCast<PacketSink>(sinkApp.Get(0)));
    }

    // configura a aplicação sink no servidor B
    for (uint32_t i = halfFlows; i < nFlows; i++)
    {
        Address sinkAddr(InetSocketAddress(interface_b2sb.GetAddress(1), port + i));
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddr);
        ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(4));
        sinkApp.Start(Seconds(1.0));
        sinkApp.Stop(Seconds(simTime));

        sinks.push_back(DynamicCast<PacketSink>(sinkApp.Get(0)));
    }

    // configura a aplicação source
    for (uint32_t i = 0; i < nFlows; i++)
    {
        // uma hora vai pro A, outra vai pro B
        Ipv4Address destIp = (i < halfFlows) ? interface_b2sa.GetAddress(1) : interface_b2sb.GetAddress(1);
        uint16_t destPort = port + i;

        BulkSendHelper source("ns3::TcpSocketFactory",
                        InetSocketAddress(destIp, destPort));

        source.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer srcApp = source.Install(nodes.Get(0));
        srcApp.Start(Seconds(1.0 + 0.1 * i));
        srcApp.Stop(Seconds(simTime));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // define o XML para onde vai a animação
    AnimationInterface anim("lab2_p2.xml");

    anim.SetConstantPosition(nodes.Get(0), 10.0, 30.0);
    anim.SetConstantPosition(nodes.Get(1), 30.0, 30.0);
    anim.SetConstantPosition(nodes.Get(2), 50.0, 30.0);
    anim.SetConstantPosition(nodes.Get(3), 70.0, 10.0);
    anim.SetConstantPosition(nodes.Get(4), 70.0, 50.0);

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    std::cout << "----- Goodput Results -----" << std::endl;

    for (uint32_t i = 0; i < nFlows; i++)
    {
        double bytesReceived = sinks[i]->GetTotalRx();
        double goodputMbps = (bytesReceived * 8) / (simTime * 1e6);

        std::cout << "Flow " << i
                << " received: " << bytesReceived << " bytes, "
                << "Goodput = " << goodputMbps << " Mbps"
                << (i < halfFlows ? " (Dest1)" : " (Dest2)") 
                << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
