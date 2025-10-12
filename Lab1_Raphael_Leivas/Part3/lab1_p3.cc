/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"

#define MAX_PACKETS 20
#define SERVER_PORT 9
#define MAX_WIFI 9

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TwoWifiNetworksExample");

int
main(int argc, char* argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    bool tracing = false;
    uint32_t nPackets = 1;
    uint32_t nWifi = 1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifi", "Número de dispositivos STA WIFI por rede", nWifi);
    cmd.AddValue("tracing", "Se precisa ou nao dos arquivos PCAP", tracing);
    cmd.AddValue("nPackets", "Numero de pacotes para enviar", nPackets);
    cmd.Parse(argc, argv);
    
    if (nPackets > MAX_PACKETS)
    {
        nPackets = MAX_PACKETS;
    }

    if (nWifi > MAX_WIFI)
    {
        nWifi = MAX_WIFI;
    }

    // ponto a ponto entre os dois APs
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices = pointToPoint.Install(p2pNodes);

    // --- primeira rede wifi -----
    NodeContainer wifiStaNodes1;
    wifiStaNodes1.Create(nWifi);
    NodeContainer wifiApNode1 = p2pNodes.Get(0);

    YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy1;
    phy1.SetChannel(channel1.Create());

    WifiHelper wifi1;
    WifiMacHelper mac1;
    Ssid ssid1 = Ssid("ns-3-ssid-1");

    mac1.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue(ssid1),
                 "ActiveProbing", BooleanValue(false));

    NetDeviceContainer staDevices1 = wifi1.Install(phy1, mac1, wifiStaNodes1);

    mac1.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid1));
    NetDeviceContainer apDevice1 = wifi1.Install(phy1, mac1, wifiApNode1);

    // --- segunda rede wifi -----
    NodeContainer wifiStaNodes2;
    wifiStaNodes2.Create(nWifi);
    NodeContainer wifiApNode2 = p2pNodes.Get(1);

    YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy2;
    phy2.SetChannel(channel2.Create());

    WifiHelper wifi2;
    WifiMacHelper mac2;
    Ssid ssid2 = Ssid("ns-3-ssid-2");

    mac2.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue(ssid2),
                 "ActiveProbing", BooleanValue(false));

    NetDeviceContainer staDevices2 = wifi2.Install(phy2, mac2, wifiStaNodes2);

    mac2.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid2));
    NetDeviceContainer apDevice2 = wifi2.Install(phy2, mac2, wifiApNode2);

    // --- Mobility setup ---
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));

    mobility.Install(wifiStaNodes1);
    mobility.Install(wifiStaNodes2);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode1);
    mobility.Install(wifiApNode2);

    // --- Internet stack ---
    InternetStackHelper stack;
    stack.Install(wifiApNode1);
    stack.Install(wifiApNode2);
    stack.Install(wifiStaNodes1);
    stack.Install(wifiStaNodes2);

    // --- Addressing ---
    Ipv4AddressHelper address;

    // p2p link
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);

    // wifi1 network
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterfaces1 = address.Assign(staDevices1);
    Ipv4InterfaceContainer apInterface1 = address.Assign(apDevice1);

    // wifi2 network
    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterfaces2 = address.Assign(staDevices2);
    Ipv4InterfaceContainer apInterface2 = address.Assign(apDevice2);

    // --- Applications ---
    UdpEchoServerHelper echoServer(SERVER_PORT);
    ApplicationContainer serverApps = echoServer.Install(wifiStaNodes2.Get(nWifi - 1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(staInterfaces2.GetAddress(nWifi - 1), SERVER_PORT);
    echoClient.SetAttribute("MaxPackets", UintegerValue(nPackets));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(wifiStaNodes1.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("lab1_p3.xml");

    Simulator::Stop(Seconds(10.0));

    // --- Tracing ---
    if (tracing)
    {
        phy1.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy2.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

        pointToPoint.EnablePcapAll("two-wifi");
        phy1.EnablePcap("two-wifi-ap1", apDevice1.Get(0));
        phy2.EnablePcap("two-wifi-ap2", apDevice2.Get(0));
    }

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
