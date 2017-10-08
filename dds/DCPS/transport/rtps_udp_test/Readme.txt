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
4) Change the constructor in TransportRegistry.cpp to include the new transport
 
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



(N) Discovery happens outside of the transport in OpenDDS.  This is annoying because it uses another module.
 
(N) These are the points where send and receives happen: 
 RtpsUdpTestReceiveStrategy.cpp
 38 ssize_t
 39 RtpsUdpTestReceiveStrategy::receive_bytes(iovec iov[],
 40                                       int n,
 41                                       ACE_INET_Addr& remote_address,
 42                                       ACE_HANDLE fd)


 RtpsUdpTestSendStrategy.cpp

 55 size_t
 56 RtpsUdpTestSendStrategy::send_bytes_i(const iovec iov[], int n)
 57 {

(N) I need to re-run my experiment where I match what I am sending with the wireshark data sent.  The updates seems to have changed my conception of how this code works.

This is not a virtual function and seems to be specifically for sends.  This specific method is used in send_bytes_i and other methods internal to RtpsUdp
RtpsUdpTestSendStrategy.cpp
173 RtpsUdpTestSendStrategy::send_single_i(const iovec iov[], int n,                       
174                                    const ACE_INET_Addr& addr) 


RtpsUdpTestSendStrategy.cpp
172 ssize_t
173 RtpsUdpTestSendStrategy::send_single_i(const iovec iov[], int n,
174                                    const ACE_INET_Addr& addr)
175 { 
176 #ifdef ACE_LACKS_SENDMSG
177   char buffer[UDP_MAX_MESSAGE_SIZE];                      
178   char *iter = buffer;
179   for (int i = 0; i < n; ++i) {
180     if (size_t(iter - buffer + iov[i].iov_len) > UDP_MAX_MESSAGE_SIZE) {
181       ACE_ERROR((LM_ERROR, "(%P|%t) RtpsUdpTestSendStrategy::send_single_i() - "
182                  "message too large at index %d size %d\n", i, iov[i].iov_len));
183       return -1;        
184     }                                              
185     std::memcpy(iter, iov[i].iov_base, iov[i].iov_len);
186     iter += iov[i].iov_len;
187   }
188   const ssize_t result = link_->unicast_socket().send(buffer, iter - buffer, addr);
189 #else
190   const ssize_t result = link_->unicast_socket().send(iov, n, addr);
191 #endif
192   if (result < 0) { 
193     ACE_TCHAR addr_buff[256] = {};  
194     int err = errno;
195     addr.addr_to_string(addr_buff, 256, 0);
196     errno = err;
197     ACE_ERROR((LM_ERROR, "(%P|%t) RtpsUdpTestSendStrategy::send_single_i() - "
198       "destination %s failed %p\n", addr_buff, ACE_TEXT("send")));
199   }                                 
200   return result;
201 }

* This is interesting because const ssize_t result = link_->unicast_socket().send(iov, n, addr); is where you need to add your send implementation.


(N) I think this is a good start for the bits being sent:
| Start Header | RTPS Data | End Header |

Start Header
1 byte      'H' 
1 byte      'D'
1 byte      Size of the Packet
1 byte      Read Byte - Each bit represents whether someone has read or not


RTPS Data
This is the specific data provided by DDS

End Header
1 byte      'E'
1 byte      'D'


>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File Locking

This works because flock exclusively locks the file until the locker completes execution.  Thereafter, the next process that is being blocked will run blocking the other processes until it completes.

  3 #include <fcntl.h>
  4 #include <string.h>
  5 #include <sys/file.h>
  6 #include <errno.h>
  7 
  8 int main(int argc, char** argv) {
  9     pid_t pid;
 10     pid = fork();
 11     sleep(1);
 12 
 13     {
 14         FILE* fd;
 15         
 16         fd = fopen("test.cpp", "r");
 17         if(fd == NULL) {
 18             fprintf(stderr, "Cannot open file\n");
 19             printf("Error\n");
 20             return 1;
 21         }   
 22 
 23         int lock_int = flock(fileno(fd), LOCK_EX);
 24         if(lock_int != 0) {
 25             printf("Error in PID %d\n", pid);
 26             return 2;
 27         }
 28 
 29         char byte;
 30         int numByte;
 31         printf("The PID is %d\n", pid);
 32         do {
 33             byte = fgetc(fd);
 34             if(byte != EOF) {
 35                 printf("%c", byte);
 36             }   
 37             else
 38             {
 39                 fclose(fd);
 40                 sleep(1);
 41                 fd = fopen("test.cpp", "r");
 42                 lock_int = flock(fileno(fd), LOCK_EX);
 43                 if(lock_int != 0) {
 44                     printf("Error in PID %d\n file locking\n", pid);
 45                     return 2;
 46                 }   
 47                 printf("The PID is %d\n", pid);
 48             }   
 49         } while(1);
 50         
 51         fclose(fd);
 52     }   
 53     
 54     return 0;
 55 }  
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


Another important consideration is how iovec works
 43    struct iovec
 44    {
 45      /// byte count to read/write
 46      u_long iov_len;
 47      /// data to be read/written
 48      char *iov_base;
 49 
 50      // WSABUF is a Winsock2-only type.
 51 #  if defined (ACE_HAS_WINSOCK2) && (ACE_HAS_WINSOCK2 != 0)
 52      operator WSABUF &(void) { return *((WSABUF *) this); }
 53 #  endif /* defined (ACE_HAS_WINSOCK2) && (ACE_HAS_WINSOCK2 != 0) */
 54    };
 55 #endif /* ACE_LACKS_IOVEC */



(!) Receives are really strange.  Basically the retval determines the length.  However, the iov[n].iov_len is meaningless.  I found that the
first index buffer sized by ret is the received value.


RtpsUdpReceiveStrategy.cpp
 38 size_t
 39 RtpsUdpReceiveStrategy::receive_bytes(iovec iov[],
 40                                       int n,
 41                                       ACE_INET_Addr& remote_address,
 42                                       ACE_HANDLE fd)
 43 {
 44   const ACE_SOCK_Dgram& socket =
 45     (fd == link_->unicast_socket().get_handle())
 46     ? link_->unicast_socket() : link_->multicast_socket();
 47 #ifdef ACE_LACKS_SENDMSG
 48   char buffer[0x10000];
 49   ssize_t scatter = socket.recv(buffer, sizeof buffer, remote_address);
 50   char* iter = buffer;
 51   for (int i = 0; scatter > 0 && i < n; ++i) {
 52     const size_t chunk = std::min(static_cast<size_t>(iov[i].iov_len), // int on LynxOS
 53                                   static_cast<size_t>(scatter));
 54     std::memcpy(iov[i].iov_base, iter, chunk);
 55     scatter -= chunk;
 56     iter += chunk;
 57   }
 58   const ssize_t ret = (scatter < 0) ? scatter : (iter - buffer);
 59 #else
 60   const ssize_t ret = socket.recv(iov, n, remote_address);
 61 
 62   printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Receive\n");
 63   printf("%d\n", iov[0].iov_len);
 64   for(int i = 0; i < ret; i++) {
 65     //for(int j = 0; j < iov[i].iov_len; j++) {
 66     char *data = (char *)(iov[0].iov_base);
 67     printf("0x%x ", 0xff & data[i]);
 68   }
 69   printf("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Receive\n\n");
 70 #endif
 71   remote_address_ = remote_address;
 72   return ret;
 73 }

