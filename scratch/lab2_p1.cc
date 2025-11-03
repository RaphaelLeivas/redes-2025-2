#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/names.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traffic-control-module.h"
#include "ns3/udp-header.h"

#define STOP_TIME 20.0
#define SERVER_PORT 15
#define PACKET_SIZE 1024
#define NETWORK_BANDWIDTH "5Mbps"
#define NETWORK_PROPAGATION_DELAY "2ms"
#define MAX_FLOWS 20

using namespace ns3;

static void
CwndTracer(uint32_t flowId, uint32_t oldCwnd, uint32_t newCwnd)
{
  std::cout << "Flow " << flowId << " cwnd: " << oldCwnd << " -> " << newCwnd << std::endl;
}

int
main(int argc, char* argv[])
{
    Ptr<UniformRandomVariable> randVar = CreateObject<UniformRandomVariable> (); 

    uint32_t nClients = 4;
    std::string dataRate = "1Mbs";
    std::string delay = "20ms";
    double errorRate = 0.00001;
    uint32_t nFlows = 3;
    std::string transport_prot = "";
    double simTime = 10.0;

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

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // cria 4 nos
    NodeContainer nodes;
    nodes.Create(nClients);

     // cria os links entre os nos
    NodeContainer n0n1 = NodeContainer(nodes.Get(0), nodes.Get(1));
    NodeContainer n1n2 = NodeContainer(nodes.Get(1), nodes.Get(2));
    NodeContainer n2n3 = NodeContainer(nodes.Get(2), nodes.Get(3));

    // helper do ponto a ponto
    PointToPointHelper p2p_d0d1;
    p2p_d0d1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p_d0d1.SetChannelAttribute("Delay", StringValue("0.01ms"));

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (errorRate));

    PointToPointHelper p2p_d1d2;
    p2p_d1d2.SetDeviceAttribute("DataRate", StringValue(dataRate));
    p2p_d1d2.SetChannelAttribute("Delay", StringValue(delay));
    p2p_d1d2.SetDeviceAttribute("ReceiveErrorModel", PointerValue(em));

    PointToPointHelper p2p_d2d3;
    p2p_d0d1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p_d0d1.SetChannelAttribute("Delay", StringValue("0.01ms"));

    // instala devices em cada link
    NetDeviceContainer d0d1 = p2p_d0d1.Install(n0n1);
    NetDeviceContainer d1d2 = p2p_d1d2.Install(n1n2);
    NetDeviceContainer d2d3 = p2p_d2d3.Install(n2n3);

    // internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // enderaçamento IP
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i0i1 = address.Assign(d0d1);
    
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i1i2 = address.Assign(d1d2);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i2i3 = address.Assign(d2d3);

    // servidor UDP no ultimo
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(nodes.Get(3));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // cliente UDP no primeiro
    UdpEchoClientHelper echoClient(i2i3.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nFlows));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // aplicacao
    ApplicationContainer clientApp = echoClient.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Install sinks on receiver node
    uint16_t port = 5000;
    std::vector<ApplicationContainer> sinks(nFlows);

    for (uint32_t i = 0; i < nFlows; i++)
    {
        Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port + i));
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
        sinks[i] = sinkHelper.Install(nodes.Get(3));
        sinks[i].Start(Seconds(1.0));
    }

    // Install BulkSend applications on sender node
    for (uint32_t i = 0; i < nFlows; i++)
    {
        BulkSendHelper source("ns3::TcpSocketFactory",
                            InetSocketAddress(i0i1.GetAddress(0), port + i));
        source.SetAttribute("MaxBytes", UintegerValue(0)); // Send data forever

        ApplicationContainer sourceApp = source.Install(nodes.Get(0));
        sourceApp.Start(Seconds(1.0));

        // Trace congestion window
        Ptr<Socket> socket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());
        socket->TraceConnectWithoutContext("CongestionWindow",
        MakeBoundCallback(&CwndTracer, i));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // define o XML para onde vai a animação
    AnimationInterface anim("lab2_p1.xml");

    // Posiciona os clientes em linha em y = 10
    for (uint32_t i = 0; i < nClients; ++i)
    {
        anim.SetConstantPosition(nodes.Get(i), 10.0 + 10.0 * i, 10.0);
    }

    Simulator::Stop(Seconds(STOP_TIME));
    Simulator::Run();

    // Compute goodput
    std::cout << "\n=== Goodput Results ===" << std::endl;
    for (uint32_t i = 0; i < nFlows; i++)
    {
        uint64_t totalBytes = DynamicCast<PacketSink>(sinks[i].Get(0))->GetTotalRx();
        double goodput = (totalBytes * 8.0) / (simTime * 1e6);
        std::cout << "Flow " << i << ": " << goodput << " Mbps" << std::endl;
    }

    Simulator::Destroy();
    return 0;
}
