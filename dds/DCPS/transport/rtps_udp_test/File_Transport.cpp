#include "File_Transport.hpp"
#include <stdlib.h>


File_Transport &File_Transport::instance() {
    static File_Transport ft;

    return ft;
}

int File_Transport::send(const iovec iov[], int n, const ACE_INET_Addr& addr) {
    FILE *fd;
    int size = 0;
    int port_num = addr.get_port_number();

    // TODO: We really need a good location for the file.  We will just leave it in the working directory for now.

    // TODO: Here is a problem.  It seems that the endpoints we are using are correct.  The files must represent endpoints, thus a single file per ip port number configuration.
    for(File_Name_Map_Itr it = file_name_map.begin(); it != file_name_map.end(); ++it) {
        // We should not write for our current port number
        if(port_num != it->first) {
            fd = open(it->second.c_str(), "a");
            if(fd == NULL) {
                fclose(fd);
                return ERROR_SIZE;
            }
    
            // Write File
            // 1) Write the header
            header hdr;
            hdr.H = 'H';
            hdr.D = 'D';
            hdr.size = get_iov_size(iov, n);
            fwrite(&hdr, sizeof(char), sizeof(hdr), fd);    
    
            // 2) Write the Data
            for(int i = 0; i < n; i++) {
                char *data = (char *)(iov[i].iov_base);
                fwrite(data, sizeof(char), iov[i].iov_len, fd);
            }
            
            // 3) Write the footer
            footer ftr;
            ftr.E = 'E';
            ftr.D = 'D';
            fwrite(&ftr, sizeof(char), sizeof(ftr), fd);    
    
            fclose(fd);
        }
    }
   
    return 0;
}

int File_Transport::receive(const iovec iov[], int n, const ACE_INET_Addr& addr) {
    FILE *fd;
    int size = 0;
    unsigned port_num = addr.get_port_number();

    printf("Port Number : %u\n", port_num);
    if(port_exist(port_num)) {

        // TODO: We need to put a limit on this.  This is dangerous, because there is a potential to be stuck in this method.
        // while(1) 
        {
            fd = open(file_name_map[port_num].c_str(), "r");
            if(fd == NULL) {
                fclose(fd);
                return ERROR_SIZE;
            }

            unsigned port_num = addr.get_port_number();
            if(file_index_map.count(port_num) == 0) {
                file_index_map[port_num] = 0;
            }

            // We assume that the port number represents an endpoint
            unsigned file_index = file_index_map[port_num];
            unsigned last_index = get_open_file_size(fd) - 1;
         
            printf("file index : %u, last index : %u\n", file_index, last_index);

            if(file_index != last_index) {
                header *hdr_ptr;
                char hdr_char[sizeof(header)];

                if(file_index == 0) {
                    fseek(fd, 0, SEEK_SET);
                } 
                else {
                    fseek(fd, file_index, SEEK_SET);
                }

                // TODO: There seems to be one extra byte left over after this read.  I am not sure this is correct.
                fgets(hdr_char, sizeof(header), fd);
                printf("Header Size: %d\n", sizeof(header));
                hdr_ptr = reinterpret_cast<header *>(hdr_char);
                printf("%c%c : %u\n", hdr_ptr->H, hdr_ptr->D, hdr_ptr->size);
                size = hdr_ptr->size; 

                // TODO: Have no idea why I need to do one extra fgetc when I already got all my data earlier.
                fgetc(fd);           
                
                // Temporary Memory Allocation to make we are getting the data
                for(int i=0; i < size; i++) {
                    char val = fgetc(fd);
                    printf("0x%x ", val);
                }
                printf("\n\n");
                
                file_index_map[port_num] += sizeof(header) + size + sizeof(footer);
            }
            
            // Here is an interesting question: how do we know if a reader
            // has not read from this ip address?  There can be multiple
            // readers.
            // Read

            fclose(fd);
        }
    }
    else {
        printf("Error: Port does not exist\n");
        size = 0;
    }

    return size;
}

File_Transport::~File_Transport() {

}


File_Transport::File_Transport() {
    // TODO: We have to assume static endpoints.  Since we cannot easily get the endpoints, we will need to add them here.a  We need to dynamically add this in the future
    file_index_map[21071] = 0;
    file_index_map[21070] = 0;

    file_name_map[21071] = "127.0.0.1:21071.txt";
    file_name_map[21070] = "127.0.0.1:21070.txt";
}

short File_Transport::get_iov_size(const iovec iov[], int n) {
    short ret_val = 0;
    for(int i = 0; i < n; i++) {
        ret_val += iov[i].iov_len;
    }

    return ret_val;
}

unsigned File_Transport::get_open_file_size(FILE *fp) {
    unsigned size = 0;

    if(fp != NULL) {
        unsigned current = 0;
        current = ftell(fp);
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }

    return size;
}

FILE *File_Transport::open(const char *filename, char *ot) {
    FILE *fp;   
    int lock_status = 0;                 
    fp = fopen(filename, ot);
    if(fp == NULL) {
        return NULL;
    }

    lock_status = flock(fileno(fp), LOCK_EX);
    if(lock_status != 0) { 
        fclose(fp);
        return NULL;
    }

    return fp;
}

bool File_Transport::port_exist(unsigned port_num) {
    bool ret_val = false;
    for(File_Name_Map_Itr it = file_name_map.begin(); it != file_name_map.end(); ++it) {
        if(it->first == port_num) {
            ret_val = true;
            break;
        }
    }

    return ret_val;
}
