#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

#define N_NODES 5
#define MAX_FLOWS 20

double CalculateGoodput(Ptr<PacketSink> sink, double startTime, double endTime)
{
    uint64_t bytesReceived = sink->GetTotalRx();  // payload bytes received
    double duration = endTime - startTime;        // seconds
    double goodput = (bytesReceived * 8) / duration; // bits per second
    return goodput;
}

int main(int argc, char *argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    std::string dataRate = "1Mbs";
    std::string delay = "20ms";
    double errorRate = 0.00001;
    int nFlows = 3;
    std::string transport_prot = "";
    // double simTime = 10.0;

    CommandLine cmd;
    cmd.AddValue("dataRate", "Taxa de envio de dados", dataRate);
    cmd.AddValue("delay", "Atraso no envio de dados", delay);
    cmd.AddValue("errorRate", "Taxa de erros", errorRate);
    cmd.AddValue("nFlows", "Numero de fluxos", nFlows);
    cmd.AddValue("transport_prot", "Protocolo", transport_prot);
    cmd.Parse(argc, argv);

    if (nFlows > MAX_FLOWS) {
        nFlows = MAX_FLOWS;
    }

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

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t portA = 5000;
    uint16_t portB = 5001;

    // Servidor A
    PacketSinkHelper sinkA("ns3::TcpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), portA));
    ApplicationContainer serverA = sinkA.Install(nodes.Get(3));
    serverA.Start(Seconds(0.0));
    serverA.Stop(Seconds(10.0));

    // Servidor B
    PacketSinkHelper sinkB("ns3::TcpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), portB));
    ApplicationContainer serverB = sinkB.Install(nodes.Get(4));
    serverB.Start(Seconds(0.0));
    serverB.Stop(Seconds(10.0));

    // for (int i = 0; i < nFlows; i++)
    // {
    //     // Flow i → Server A
    //     BulkSendHelper sourceA("ns3::TcpSocketFactory",
    //                         InetSocketAddress(interface_b2sa.GetAddress(1), portA + i));
    //     sourceA.SetAttribute("MaxBytes", UintegerValue(0));
    //     ApplicationContainer appA = sourceA.Install(nodes.Get(0));
    //     appA.Start(Seconds(1.0 + 0.1 * i));   // small stagger to avoid same-time SYN
    //     appA.Stop(Seconds(10.0));

    //     // Install matching sink on Server A
    //     PacketSinkHelper sinkA("ns3::TcpSocketFactory",
    //                         InetSocketAddress(Ipv4Address::GetAny(), portA + i));
    //     ApplicationContainer sinkAppA = sinkA.Install(nodes.Get(3));
    //     sinkAppA.Start(Seconds(0.0));
    //     sinkAppA.Stop(Seconds(10.0));

    //     // Flow i → Server B
    //     BulkSendHelper sourceB("ns3::TcpSocketFactory",
    //                         InetSocketAddress(interface_b2sb.GetAddress(1), portB + i));
    //     sourceB.SetAttribute("MaxBytes", UintegerValue(0));
    //     ApplicationContainer appB = sourceB.Install(nodes.Get(0));
    //     appB.Start(Seconds(1.0 + 0.1 * i));   // same stagger
    //     appB.Stop(Seconds(10.0));

    //     // Install matching sink on Server B
    //     PacketSinkHelper sinkB("ns3::TcpSocketFactory",
    //                         InetSocketAddress(Ipv4Address::GetAny(), portB + i));
    //     ApplicationContainer sinkAppB = sinkB.Install(nodes.Get(4));
    //     sinkAppB.Start(Seconds(0.0));
    //     sinkAppB.Stop(Seconds(10.0));
    // }

    // Cliente envia pro Servidor A
    BulkSendHelper sourceA("ns3::TcpSocketFactory",
                           InetSocketAddress(interface_b2sa.GetAddress(1), portA));
    sourceA.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer clientA = sourceA.Install(nodes.Get(0));
    clientA.Start(Seconds(1.0));
    clientA.Stop(Seconds(10.0));

    // Cliente envia pro Servidor B
    BulkSendHelper sourceB("ns3::TcpSocketFactory",
                           InetSocketAddress(interface_b2sb.GetAddress(1), portB));
    sourceB.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer clientB = sourceB.Install(nodes.Get(0));
    clientB.Start(Seconds(1.0));
    clientB.Stop(Seconds(10.0));

    // define o XML para onde vai a animação
    AnimationInterface anim("lab2_p2.xml");

    anim.SetConstantPosition(nodes.Get(0), 10.0, 30.0);
    anim.SetConstantPosition(nodes.Get(1), 30.0, 30.0);
    anim.SetConstantPosition(nodes.Get(2), 50.0, 30.0);
    anim.SetConstantPosition(nodes.Get(3), 70.0, 10.0);
    anim.SetConstantPosition(nodes.Get(4), 70.0, 50.0);

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    // Calculate goodput for each server
    Ptr<PacketSink> sinkAApp = DynamicCast<PacketSink>(serverA.Get(0));
    Ptr<PacketSink> sinkBApp = DynamicCast<PacketSink>(serverB.Get(0));

    double gpA = CalculateGoodput(sinkAApp, 1.0, 10.0);
    double gpB = CalculateGoodput(sinkBApp, 1.0, 10.0);

    std::cout << "Goodput to Server A: " << gpA / 1e6 << " Mbps" << std::endl;
    std::cout << "Goodput to Server B: " << gpB / 1e6 << " Mbps" << std::endl;

    Simulator::Destroy();

    return 0;
}
