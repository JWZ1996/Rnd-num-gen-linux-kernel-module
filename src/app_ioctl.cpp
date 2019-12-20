#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <string>
#include <cstring>


#define BYTES_IN_TIME  1
#define NUMBER_OF_BYTES  8

struct ioctl_packet
{
	size_t byte_count;
	size_t time_ms_bound;
	char*  bytes;
};
char byte_arr[30];

int main() {
    int fd;
    int rnd_number;

    ioctl_packet packet;

    packet.byte_count = sizeof(int);

    packet.bytes = byte_arr;

    fd = open("/dev/char/240:0",O_RDWR);

    ioctl( fd, NUMBER_OF_BYTES, &packet);

    memcpy( &rnd_number, packet.bytes, packet.byte_count );

    std::cout << "Wylosowana liczba: " << rnd_number << std::endl;

    return 0;
}