#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <string>
#include <cstring>

// IOCTL numbers
#define RND_GEN_MAGIC_NR 'r'

// CONSTS: 
#define CORES_COUNT 3
#define RND_GEN_NUMBER_OF_BYTES  _IO(RND_GEN_MAGIC_NR,1)

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

    fd = open("/dev/char/241:0",O_RDWR);

    ioctl( fd, RND_GEN_NUMBER_OF_BYTES, &packet);

    memcpy( &rnd_number, packet.bytes, packet.byte_count );

    std::cout << "Wylosowana liczba: " << rnd_number << std::endl;

    return 0;
}