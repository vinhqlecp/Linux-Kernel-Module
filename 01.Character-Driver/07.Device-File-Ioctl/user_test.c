#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "vchar_driver.h"

int main() {
	int answer;

	int dev = open("/dev/vchar_device", O_WRONLY);
	if(dev == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}

	ioctl(dev, RD_VALUE, &answer);
	printf("The answer is %d\n", answer);

	answer = 123;

	ioctl(dev, WR_VALUE, &answer);
	ioctl(dev, RD_VALUE, &answer);
	printf("The answer is  now %d\n", answer);

	printf("Opening was successfull!\n");
	close(dev);
	return 0;
}