#include "File_Transport.hpp"



File_Transport &File_Transport::instance() {
    static File_Transport ft;

    return ft;
}

int File_Transport::send(const iovec iov[], int n, const ACE_INET_Addr& addr) {
    FILE *fd;
    int lock_status = 0;
    int size = 0;

    // TODO: We really need a good location for the file.  We will just leave it in the working directory for now.

    fd = open(addr, "a");
    if(fd == NULL) {
        return ERROR_SIZE;
    }

    lock_status = flock(fileno(fd), LOCK_EX);
    if(lock_status != 0) {
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

int File_Transport::receive(const iovec iov[], int n, const ACE_INET_Addr& addr) {
    FILE *fd;
    int lock_status = 0;
    int size = 0;

    fd = open(addr, "r");
    if(fd == NULL) {
        return ERROR_SIZE;
    }

    lock_status = flock(fileno(fd), LOCK_EX);
    if(lock_status != 0) {
        return ERROR_SIZE;
    }

    // Read

    fclose(fd);

    return 0;
}

File_Transport::~File_Transport() {

}


File_Transport::File_Transport() : file_index(0) {

}

short File_Transport::get_iov_size(const iovec iov[], int n) {
    short ret_val = 0;
    for(int i = 0; i < n; i++) {
        ret_val += iov[i].iov_len;
    }

    return ret_val;
}

unsigned File_Transport::get_opened_file_size(FILE *fp) {
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
    addr.addr_to_string(addr_str, 80);                           
    std::string file_name(addr_str);
    file_name = file_name + ".txt";   
    fp = fopen(file_name.c_str(), ot);

    return fp;
}
