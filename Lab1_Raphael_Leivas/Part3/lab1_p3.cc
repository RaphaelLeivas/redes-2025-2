/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

#define MAX_PACKETS 20
#define MAX_WIFI 9
#define SERVER_PORT 9

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

int
main(int argc, char* argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    CommandLine cmd;

    uint32_t nPackets = 1;
    uint32_t nWifi = 3;
    uint32_t tracing = 0;
    uint32_t nCsma = 3; // fixo, nao setado por linha de comando

    cmd.AddValue("nPackets", "Numero de pacotes para enviar", nPackets);
    cmd.AddValue("nWifi", "Numero de nós wifi", nWifi);
    cmd.AddValue("tracing", "Se precisa ou nao dos arquivos PCAP", tracing);

    cmd.Parse(argc, argv);

    if (nPackets > MAX_PACKETS)
    {
        nPackets = MAX_PACKETS;
    }

    if (nWifi > MAX_WIFI)
    {
        nWifi = MAX_WIFI;
    }

    // ponto a ponto entre os dois AP wifi
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1));
    csmaNodes.Create(nCsma);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    // ------ Configura o primeiro WIFI ------- //

    NodeContainer first_wifiStaNodes;
    first_wifiStaNodes.Create(nWifi);
    NodeContainer first_wifiApNode = p2pNodes.Get(0);

    YansWifiChannelHelper first_channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper first_phy;
    first_phy.SetChannel(first_channel.Create());

    WifiHelper first_wifi;
    WifiMacHelper first_mac;
    Ssid first_ssid = Ssid("ns-3-ssid-1");

    NetDeviceContainer first_staDevices;
    first_mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(first_ssid), "ActiveProbing", BooleanValue(false));
    first_staDevices = first_wifi.Install(first_phy, first_mac, first_wifiStaNodes);

    NetDeviceContainer first_apDevices;
    first_mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(first_ssid));
    first_apDevices = first_wifi.Install(first_phy, first_mac, first_wifiApNode);

    // ------ Configura o segundo WIFI ------- //

    NodeContainer second_wifiStaNodes;
    second_wifiStaNodes.Create(nWifi);
    NodeContainer second_wifiApNode = p2pNodes.Get(0);

    YansWifiChannelHelper second_channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper second_phy;
    second_phy.SetChannel(second_channel.Create());

    WifiHelper second_wifi;
    WifiMacHelper second_mac;
    Ssid second_ssid = Ssid("ns-3-ssid-2");

    NetDeviceContainer second_staDevices;
    second_mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(second_ssid), "ActiveProbing", BooleanValue(false));
    second_staDevices = second_wifi.Install(second_phy, second_mac, second_wifiStaNodes);

    NetDeviceContainer second_apDevices;
    second_mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(second_ssid));
    second_apDevices = second_wifi.Install(second_phy, second_mac, second_wifiApNode);


    // ------ configurações gerais ------ //

    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(first_wifiStaNodes);
    mobility.Install(second_wifiStaNodes);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(first_wifiApNode);
    mobility.Install(second_wifiApNode);

    InternetStackHelper stack;
    stack.Install(csmaNodes);
    stack.Install(first_wifiApNode);
    stack.Install(first_wifiStaNodes);
    stack.Install(second_wifiApNode);
    stack.Install(second_wifiStaNodes);

    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    // IP da primeira rede wifi
    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(first_staDevices);
    address.Assign(first_apDevices);

    // IP da segunda rede wifi
    address.SetBase("10.1.4.0", "255.255.255.0");
    address.Assign(second_staDevices);
    address.Assign(second_apDevices);

    UdpEchoServerHelper echoServer(SERVER_PORT);

    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1));
    serverApps.Stop(Seconds(10));

    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), SERVER_PORT);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(first_wifiStaNodes.Get(nWifi - 1));
    clientApps.Start(Seconds(2));
    clientApps.Stop(Seconds(10));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("lab1_p3.xml");

    // posicionamentos para a animação
    // for (uint32_t i = 0; i < 2; ++i)
    // {
    //     anim.SetConstantPosition(p2pNodes.Get(i), 10.0 + 10.0 * i, 10.0);
    // }
    // for (uint32_t i = 0; i < nCsma; ++i)
    // {
    //     anim.SetConstantPosition(csmaNodes.Get(i), 30.0 + 10.0 * i, 10.0);
    // }
    // for (uint32_t i = 0; i < nWifi; ++i)
    // {
    //     anim.SetConstantPosition(wifiStaNodes.Get(i), 80.0 + 10.0 * i, 10.0);
    // }
    // anim.SetConstantPosition(wifiApNode.Get(0), 110.0, 10.0);

    Simulator::Stop(Seconds(10));

    if (tracing)
    {
        first_phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        pointToPoint.EnablePcapAll("third");
        first_phy.EnablePcap("third", first_apDevices.Get(0));
        csma.EnablePcap("third", csmaDevices.Get(0), true);
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
