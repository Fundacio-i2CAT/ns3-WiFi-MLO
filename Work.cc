/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2025 i2CAT
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Suneel Kumar (i2CAT)
 */
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-mac.h"
#include "ns3/gnuplot.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/wifi-acknowledgment.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/txop.h"
#include "ns3/pointer.h"
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>



// file manuplation functions

using namespace ns3;
//Function to check if a directory entry is a regular file
bool isRegularFile(const std::string& path) {
    struct stat path_stat;
    stat(path.c_str(), &path_stat);
    return S_ISREG(path_stat.st_mode);
}

// Function to delete files in a directory
void deleteFilesInDirectory(const std::string& directoryPath) {
    DIR* dir = opendir(directoryPath.c_str());
    if (dir == nullptr) {
        std::cerr << "Error opening directory: " << directoryPath << std::endl;
        return;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filePath = directoryPath + "/" + entry->d_name;
        if (isRegularFile(filePath) && entry->d_type != DT_DIR) {
            if (unlink(filePath.c_str()) != 0) {
                std::cerr << "Error deleting file: " << filePath << std::endl;
            }
        }
    }

    closedir(dir);
}
//Function to copy files from one directory to another
void copyFilesToDirectory(const std::string& sourceDirectory, const std::string& destinationDirectory) {
    DIR* dir = opendir(sourceDirectory.c_str());
    if (dir == nullptr) {
        std::cerr << "Error opening directory: " << sourceDirectory << std::endl;
        return;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string sourceFilePath = sourceDirectory + "/" + entry->d_name;
        std::string destinationFilePath = destinationDirectory + "/" + entry->d_name;

        if (isRegularFile(sourceFilePath) && entry->d_type != DT_DIR) {
            std::ifstream sourceFile(sourceFilePath, std::ios::binary);
            std::ofstream destinationFile(destinationFilePath, std::ios::binary);

            if (!sourceFile) {
                std::cerr << "Error opening source file: " << sourceFilePath << std::endl;
            } else if (!destinationFile) {
                std::cerr << "Error opening destination file: " << destinationFilePath << std::endl;
            } else {
                destinationFile << sourceFile.rdbuf();
                sourceFile.close();
                destinationFile.close();
            }
        }
    }

    closedir(dir);
}


NS_LOG_COMPONENT_DEFINE ("he-wifi-network");




int
main (int argc, char *argv[])
{

// Loop for running number of Seeds, right now it runs only one time
for(int NumSeeds=10;NumSeeds<=10;NumSeeds++)
{
  srand (5*NumSeeds);//2
 
  //bool useExtendedBlockAck {false};
  double simulationTime{30}; //seconds
  double distance{1}; //1meters
  int gi = 800;
  std::size_t nStations{40}; 
  std::string dlAckSeqType{"NO-OFDMA"};
  bool enableUlOfdma{false};
  bool enableBsrp{false};
  int mcs{-1}; // -1 indicates an unset value
  uint32_t payloadSize = 64; // must fit in the max TX duration when transmitting at MCS 0 over an RU of 26 tones
  std::string phyModel{"Yans"};
 

  

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nStations);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  NetDeviceContainer apDevice, staDevices;
  WifiMacHelper mac;
  WifiHelper wifi1;

  wifi1.SetStandard (WIFI_STANDARD_80211ax);
  Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceLoss", DoubleValue (40));
  std::ostringstream oss1, oss2;
  uint32_t mcs1 = 10; 
  oss1 << "HeMcs" << mcs1;
  uint32_t rtsThreshold = 65535;
  wifi1.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                 StringValue (oss1.str ()), "ControlMode",
                                 StringValue (oss1.str ()));
  Ssid ssid = Ssid ("ns3-80211ax");
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy1, phy2;

  // define 2 links 
  phy1.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy1.SetChannel (channel.Create ());
  phy2.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy2.SetChannel (channel.Create ());

  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));

  phy1.Set ("ChannelSettings", StringValue ("{0 ,160, BAND_6GHZ, 0}")); //phy1 15,160
  phy2.Set ("ChannelSettings", StringValue ("{0, 160, BAND_6GHZ, 0}")); //phy2 47,160

  staDevices = wifi1.Install (phy1, phy2, mac, wifiStaNodes);

  // Disable A-MPDU of station side 1sst, this is mandatory otherwise MLO would not work
  Ptr<NetDevice> dev;
  Ptr<WifiNetDevice> wifi_dev;
  for (int i = 0; i < nStations; i++)
    {

      dev = wifiStaNodes.Get (i)->GetDevice (0);
      wifi_dev = DynamicCast<WifiNetDevice> (dev);
      wifi_dev->GetMac ()->SetAttribute ("BE_MaxAmpduSize", UintegerValue (0));
    }

  mac.SetType ("ns3::ApWifiMac", "EnableBeaconJitter", BooleanValue (false), "Ssid",
               SsidValue (ssid));
  std::cout << "Scratch:: AP Wifidevice installation\n";
  apDevice = wifi1.Install (phy1, phy2, mac, wifiApNode);
  //Disable A-MPDU
  dev = wifiApNode.Get (0)->GetDevice (0);
  wifi_dev = DynamicCast<WifiNetDevice> (dev);
  wifi_dev->GetMac ()->SetAttribute ("BE_MaxAmpduSize", UintegerValue (0));

  RngSeedManager::SetSeed (NumSeeds*2);
  RngSeedManager::SetRun (NumSeeds*2); 
  int64_t streamNumber = 100;
  // Set guard interval and MPDU buffer size
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HeConfiguration/GuardInterval",
               TimeValue (NanoSeconds (gi)));


  // mobility.
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0)); //AP
  positionAlloc->Add (Vector ((distance), 0.0, 0.0)); //1 MLD
  float gap = 0.1;
  for (int i = 1; i <= nStations; i++, gap += 0.1)
    {
      positionAlloc->Add (Vector (gap, distance, 0.0)); //
    }

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);
  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterfaces;
  Ipv4InterfaceContainer apNodeInterface;

  staNodeInterfaces = address.Assign (staDevices);
  apNodeInterface = address.Assign (apDevice);

  // Configurating applications
  //Keep in mind ALL STA by dafult are MLD, we need to configure them as single link or multilink
  // You can change sta-wifi-mac
  // MLD STAs range from STA1 to STA 11 (index 0 to 10)
  //STA16 to STA22 can be set as background on link 1 (index 15 to 21)
  //STA12 to STA 15 can be set as background on link 2 (index 11 to 14)
  
  uint16_t port = 98;
  float RandomTime_ = 0.0;
  float Synch = 0.0799248; // to create differet starting time for aplication
  
  /////////////// setting MLD STAs  ///////////////////////
  int i;
  RandomTime_ = (std::rand() % 10000 + 1.0)/10000;

  for(i=1;i<=5;i++) //-
  {
    
    port++;
    ApplicationContainer serverApp;

    UdpServerHelper server (port);
    serverApp = server.Install (wifiApNode.Get (0));
    serverApp.Start (Seconds (0.0));
    serverApp.Stop (Seconds (simulationTime));

    UdpClientHelper client (apNodeInterface.GetAddress (0), port);
    client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client.SetAttribute ("Interval", TimeValue (Time ("0.0025"))); //packets/s 0.000225//0.00112
    client.SetAttribute ("PacketSize", UintegerValue (64));
    ApplicationContainer clientApp = client.Install (wifiStaNodes.Get (i));

     clientApp.Start (Seconds (RandomTime_));
     RandomTime_ = (std::rand() % 10000 + 1.0)/10000;
  
    clientApp.Stop (Seconds (simulationTime));
  }
//Flow 1
 i = 0;
  {
    port++;
    ApplicationContainer serverApp;

    UdpServerHelper server (port);
    serverApp = server.Install (wifiApNode.Get (0));
    serverApp.Start (Seconds (0.0));
    serverApp.Stop (Seconds (simulationTime));

    UdpClientHelper client (apNodeInterface.GetAddress (0), port);
    client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client.SetAttribute ("Interval", TimeValue (Time ("0.0025"))); //packets/s 0.000225//0.00112
    client.SetAttribute ("PacketSize", UintegerValue (64));
    ApplicationContainer clientApp = client.Install (wifiStaNodes.Get (i));
    RandomTime_ = (std::rand() % 10000 + 1.0)/10000;
    clientApp.Start (Seconds (RandomTime_));
    clientApp.Stop (Seconds (simulationTime));
  }

RandomTime_+=Synch;
 //Background flow on link 2
for ( i = 11; i <=14 ; i++) //dont start with zero  10-21  ------>>5
  {
    //end target
     ApplicationContainer serverApp1;
     port++;
     UdpServerHelper server (port);
     serverApp1 = server.Install (wifiApNode.Get (0));
     serverApp1.Start (Seconds (0));
     serverApp1.Stop (Seconds (simulationTime));

     UdpClientHelper client (apNodeInterface.GetAddress (0), port);
     client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     client.SetAttribute ("Interval",
                          TimeValue (Time ("0.0001"))); //packets/s 0.000225//0.0013
     client.SetAttribute ("PacketSize", UintegerValue (1500));
     ApplicationContainer clientApp = client.Install (wifiStaNodes.Get (i));

    clientApp.Start (Seconds (11 ));
  
   
    clientApp.Stop (Seconds (40));
   }
  
 ////Background flow on link 1 not set
  for ( i = 15; i <= 21; i++) //dont start with zero  10-21  ------>>5
  {
    //end target
     ApplicationContainer serverApp1;
     port++;
     UdpServerHelper server (port);
     serverApp1 = server.Install (wifiApNode.Get (0));
     serverApp1.Start (Seconds (0));
     serverApp1.Stop (Seconds (simulationTime));

     UdpClientHelper client (apNodeInterface.GetAddress (0), port);
     client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
     client.SetAttribute ("Interval",
                          TimeValue (Time ("0.01"))); //packets/s 0.000225//0.0013
     client.SetAttribute ("PacketSize", UintegerValue (1500));
     ApplicationContainer clientApp = client.Install (wifiStaNodes.Get (i));

    clientApp.Start (Seconds (1+RandomTime_));
    
    clientApp.Stop (Seconds (simulationTime));
   }
    

  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();

  Simulator::Destroy ();
  
// For creating automatic multiple seeds

//  const char* path = "FreshScenarios/4-7/Journal/Setup3//Test1/Seed";
//  const char* path2="/our_CW";
//  std::stringstream ss;
// ss << path << NumSeeds<<path2;
// std::string Destination = ss.str();
// copyFilesToDirectory("FreshScenarios/",Destination);
// deleteFilesInDirectory("FreshScenarios/"); 
}
  return 0;
}
