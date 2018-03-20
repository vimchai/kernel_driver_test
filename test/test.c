#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>  
#include <string.h>  

#define DEVICE_NAME "/dev/hello"

int main(int argc, char** argv)
{
    int fd = -1;
    int val = 0;
    char *buffer;  
    char *mapBuf;  

    fd = open(DEVICE_NAME, O_RDWR);
    if(fd == -1) {
        printf("Failed to open device %s.\n", DEVICE_NAME);
        return -1;
    }

    printf("Read original value:\n");
    read(fd, &val, sizeof(val));
    printf("%d.\n\n", val);
    val = 5;
    printf("Write value %d to %s.\n\n", val, DEVICE_NAME);
        write(fd, &val, sizeof(val));

    printf("Read the value again:\n");
        read(fd, &val, sizeof(val));
        printf("%d.\n\n", val);

    printf("before mmap\n");  
    sleep(1);//睡眠15秒，查看映射前的内存图cat /proc/pid/maps  
    buffer = (char *)malloc(1024);  
    memset(buffer, 0, 1024);  
    mapBuf = mmap(NULL, 102, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);//内存映射，会调用驱动的mmap函数  
    printf("after mmap\n");  
    sleep(1);//睡眠15秒，在命令行查看映射后的内存图，如果多出了映射段，说明映射成功  
      
    /*测试二：往映射段读写数据，看是否成功*/  
    strcpy(mapBuf, "Driver Test");//向映射段写数据  
    memset(buffer, 0, 1024);  
    strcpy(buffer, mapBuf);//从映射段读取数据  
    printf("buf = %s\n", buffer);//如果读取出来的数据和写入的数据一致，说明映射段的确成功了  
      
      
    munmap(mapBuf, 1024);//去除映射  
    free(buffer);  

    close(fd);
    return 0;
}