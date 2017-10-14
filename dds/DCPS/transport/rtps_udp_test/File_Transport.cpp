#include "File_Transport.hpp"



File_Transport &File_Transport::instance() {
    static File_Transport ft;

    return ft;
}

int File_Transport::send(const iovec iov[], int n, const ACE_INET_Addr& addr) {
    FILE *fd;
    int lock_status = 0;
    int size = 0;


    printf("Send Port Number %u\n", addr.get_port_number());

    // TODO: We really need a good location for the file.  We will just leave it in the working directory for now.

    fd = open(addr, "a");
    if(fd == NULL) {
        printf("Error: Cannot open file to write.\n");
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

    return 0;
}

int File_Transport::receive(const iovec iov[], int n, int port_num) {
    FILE *fd;
    int lock_status = 0;
    int size = 0;

    fd = open(port_num, "r");
    if(fd == NULL) {
        printf("ERROR: Cannot Open File to Read.\n");
        return ERROR_SIZE;
    }

    unsigned file_index = file_index_map_[port_num];
    unsigned last_index = get_open_file_size(fd);
    
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
        hdr_ptr = reinterpret_cast<header *>(hdr_char);
        printf("%c%c : %u\n", hdr_ptr->H, hdr_ptr->D, hdr_ptr->size);
        size = hdr_ptr->size; 

        // TODO: Have no idea why I need to do one extra fgetc when I already got all my data earlier.
        fgetc(fd);           
        
        // Temporary Memory Allocation to make we are getting the data
        for(int i=0; i < size; i++) {
            char val = fgetc(fd);
            printf("0x%x ", val & 0xff);
        }
        
        printf("\n");
        file_index_map_[port_num] += sizeof(header) + size + sizeof(footer);
    }
    
    // Here is an interesting question: how do we know if a reader
    // has not read from this ip address?  There can be multiple
    // readers.

    fclose(fd);

    printf("Final File Index %u\n", file_index_map_[port_num]); 

    return size;
}

void File_Transport::save_receive(const iovec iov[], int n, int port) {
    
}

File_Transport::~File_Transport() {

}


File_Transport::File_Transport() {

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

FILE *File_Transport::open(const ACE_INET_Addr &addr, char *ot) {
    FILE *fp;
    char addr_str[80];   
    int lock_status = 0;    
                 
    addr.addr_to_string(addr_str, 80);                           
    std::string file_name(addr_str);
    file_name = file_name + ".txt";   
    fp = fopen(file_name.c_str(), ot);
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

FILE *File_Transport::open(const int port, char *ot) {
    FILE *fp;
    char addr_str[80];
    int lock_status = 0;

    sprintf(addr_str, "127.0.0.1:%d.txt", port);
    fp = fopen(addr_str, ot);
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
    for(File_Index_Itr it = file_index_map_.begin(); it != file_index_map_.end(); ++it) {
        if(it->first == port_num) {
            ret_val = true;
            break;
        }
    }

    return ret_val;
}
