#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <math.h>
#include <cstring>

#include <unistd.h>

      #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

typedef char BYTE;



int main()
{
	int fd;
    int rnd_number;



    fd = open("/dev/char/241:0",O_RDWR);


    // !!! NOT THE IOCTL !!!
    read(fd,&rnd_number,sizeof(int));


    std::cout << "Wylosowana liczba: " << rnd_number << std::endl;
	return 0;
}