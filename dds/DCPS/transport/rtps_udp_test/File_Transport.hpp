#ifndef FILE_TRANSPORT

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <string>

#include "ace/INET_Addr.h"

#include <map>


#define ERROR_SIZE -1

typedef std::map<unsigned, unsigned> File_Index_Map;
typedef std::map<unsigned, unsigned>::iterator File_Index_Itr;

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
        int receive(const iovec iov[], int n, int port);

        void save_receive(const iovec iov[], int n, int port);
        ~File_Transport();
    private:
        File_Transport();
        short get_iov_size(const iovec iov[], int n);
        unsigned get_open_file_size(FILE *fd);
        FILE *open(const ACE_INET_Addr &addr, char *ot);
        FILE *open(const int port_num, char *ot);
        bool port_exist(unsigned port_num);


        File_Index_Map file_index_map_;


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
