#include "File_Transport.hpp"



File_Transport &File_Transport::instance() {
    static File_Transport ft;

    return ft;
}

int File_Transport::send(const iovec iov[], int n, const ACE_INET_Addr& addr) {
    FILE *fd;
    int lock_status = 0;
    int size = 0;

    fd = fopen("somestring", "r");
    if(fd == NULL) {
        return ERROR_SIZE;
    }

    lock_status = flock(fileno(fd), LOCK_EX);
    if(lock_status != 0) {
        return ERROR_SIZE;
    }

    // Write File


    fclose(fd);

    return 0;
}

int File_Transport::receive(const iovec iov[], int n, const ACE_INET_Addr& addr) {
    FILE *fd;
    int lock_status = 0;
    int size = 0;

    fd = fopen("somethingstring", "r");
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
