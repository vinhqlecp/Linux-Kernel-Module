#ifndef INC_COMMIF_H_
#define INC_COMMIF_H_

typedef enum EM_COMM_MSG_TYPE{
	OPT_LED_CTRL_REQ		= 0x3001,
}EM_COMM_MSG_TYPE;

typedef struct st_Comm_Header
{
	unsigned int dwMessageID;
	unsigned int dwTotalMsgLen;
}st_Comm_Header;

typedef struct st_Body_Led_Ctrl_Req {
	unsigned int uiFuncID;
	unsigned int uiSeqNo;
    unsigned int uiLedState;
    unsigned int uiPwm;
}st_Body_Led_Ctrl_Req;

typedef struct st_Led_Ctrl_Req {
	st_Comm_Header header;
    st_Body_Led_Ctrl_Req msg;
}st_Led_Ctrl_Req;

#endif /* INC_COMMIF_H_ */