#ifndef FILE_TRANSPORT

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <string>
#include <map>

#include "ace/INET_Addr.h"

#define ERROR_SIZE -1

/* Assumptions
    1) All Addresses are Localhost
    2) The Endpoints or Ports are provided upon instantiation of the singleton
*/

typedef std::map<unsigned, std::string> File_Name_Map;
typedef std::map<unsigned, std::string>::iterator File_Name_Map_Itr;

class File_Transport {
    public:
        #pragma pack(1)
        struct header {
            char H;
            char D;
            short size;
        };

        #pragma pack(1)
        struct footer {
            char E;
            char D;
        };

        static File_Transport &instance();
        int send(const iovec iov[], int n, const ACE_INET_Addr& addr);
        int receive(const iovec iov[], int n, const ACE_INET_Addr& addr);

        ~File_Transport();
    private:
        File_Transport();
        short get_iov_size(const iovec iov[], int n);
        unsigned get_open_file_size(FILE *fd);
        FILE *open(const char *filename, char *ot);
        bool port_exist(unsigned port_num);
        

        std::map<unsigned, unsigned> file_index_map;
        File_Name_Map file_name_map;
    
        // We need to either remove the data or keep track of it in the file
        // Each IP Address will have its own file
        // Writers will append to the file
        // Readers will read from the file and keep a count of its index
        // Writers and Readers will need to open and close the file 
        // one at a time
        // We need File Pointer
        // We need locks
};

#endif
