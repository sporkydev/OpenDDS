#ifndef FILE_TRANSPORT

#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>

#include "ace/INET_Addr.h"

#define ERROR_SIZE -1

class File_Transport {
    public:
        static File_Transport &instance();
        int send(const iovec iov[], int n, const ACE_INET_Addr& addr);
        int receive(const iovec iov[], int n, const ACE_INET_Addr& addr);

        ~File_Transport();
    private:
        File_Transport();

        unsigned file_index;

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
