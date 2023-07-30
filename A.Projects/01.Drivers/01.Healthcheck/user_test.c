#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "commif.h"

unsigned int ledState = 0;
int main (void){
    
    while(1){
        int fd = open("/dev/healthcheck", O_WRONLY);
        if (fd == -1) {
            perror("Unable to open /dev/healthcheck");
            exit(1);
        }

        st_Led_Ctrl_Req lcReq;
        lcReq.header.dwMessageID = OPT_LED_CTRL_REQ;
        lcReq.header.dwTotalMsgLen = sizeof(st_Led_Ctrl_Req);
        lcReq.msg.uiFuncID = 0;
        lcReq.msg.uiSeqNo = 0;
        ledState = ledState == 0 ? 1 : 0;
        lcReq.msg.uiLedState = ledState;
        lcReq.msg.uiPwm = 0;

        char* byteSnd = (char*)&lcReq;
        if (write(fd, byteSnd, sizeof(st_Led_Ctrl_Req)) != sizeof(st_Led_Ctrl_Req)) {
            perror("Error writing to /dev/healthcheck");
            close(fd);
            exit(1);
        }
        close(fd);

        sleep(1);
    }

    return 0;
}