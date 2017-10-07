10/6/2017

Replace the following
RtpsUdp     RtpsUdpTest
RTPS_UDP    RTPS_UDP_TEST
rtps_udp    rtps_udp_test
Rtps_Udp    Rtps_Udp_Test

Notes
We also had to change the constructor in the TransportRegistry.cpp File

(N) Steps to make a new transport
1) Copy the rtps_udp to rtps_udp_test
2) Replace the strings 
    ls | xargs sed -i 's/RtpsUdp/RtpsUdpTest/g'
    ls | xargs sed -i 's/RTPS_UDP/RTPS_UDP_TEST/g'
    ls | xargs sed -i 's/rtp_udp/rtps_udp_test/g'
    ls | xargs sed -i 's/Rtps_Udp/Rtps_Udp_Test/g'
3) Rename file
    rename -v "s/RtpsUdp/RtpsUdpTest/g" *
    rename -v "s/RTPS_UDP/RTPS_UDP_TEST/g" *
    rename -v "s/rtps_udp/rtps_udp_test/g" *
    rename -v "s/Rtps_Udp/Rtps_Udp_Test/g" *
3) Change the constructor in TransportRegistry.cpp to include the new transport
 
 67 TransportRegistry::TransportRegistry()
 68   : global_config_(make_rch<TransportConfig>(DEFAULT_CONFIG_NAME))
 69 {
 70   DBG_ENTRY_LVL("TransportRegistry", "TransportRegistry", 6);
 71   config_map_[DEFAULT_CONFIG_NAME] = global_config_;
 72 
 73   lib_directive_map_["tcp"]       = "dynamic OpenDDS_Tcp Service_Object * OpenDDS_Tcp:_make_TcpLoader()";
 74   lib_directive_map_["udp"]       = "dynamic OpenDDS_Udp Service_Object * OpenDDS_Udp:_make_UdpLoader()";
 75   lib_directive_map_["multicast"] = "dynamic OpenDDS_Multicast Service_Object * OpenDDS_Multicast:_make_MulticastLoader()";
 76   lib_directive_map_["rtps_udp"]  = "dynamic OpenDDS_Rtps_Udp Service_Object * OpenDDS_Rtps_Udp:_make_RtpsUdpLoader()";
 77   lib_directive_map_["shmem"]     = "dynamic OpenDDS_Shmem Service_Object * OpenDDS_Shmem:_make_ShmemLoader()";
    >> Add your transport here
 78 
 79   // load_transport_lib() is used for discovery as well:
 80   lib_directive_map_["rtps_discovery"] = lib_directive_map_["rtps_udp"];
 81   lib_directive_map_["repository"] = "dynamic OpenDDS_InfoRepoDiscovery Service_Object * OpenDDS_InfoRepoDiscovery:_make_IRDiscoveryLoader()";
 82 }

