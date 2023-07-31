#ifndef INC_COMMIF_H_
#define INC_COMMIF_H_

typedef enum EM_COMM_MSG_TYPE {
	OPT_GET_DEV_LOC_REQ		= 0x3011,
	OPT_GET_DEV_LOC_RES		= 0x3012,
} EM_COMM_MSG_TYPE;

typedef struct st_Comm_Header {
	unsigned int dwMessageID;
	unsigned int dwTotalMsgLen;
} st_Comm_Header;

typedef struct st_Body_Get_Device_Location_Req {
	unsigned int uiFuncID;
	unsigned int uiSeqNo;
} st_Body_Get_Device_Location_Req;

typedef struct st_Get_Device_Location_Req {
	st_Comm_Header header;
    st_Body_Get_Device_Location_Req msg;
} st_Get_Device_Location_Req;

typedef struct st_Body_Get_Device_Location_Res {
	unsigned int uiFuncID;
	unsigned int uiSeqNo;
} st_Body_Get_Device_Location_Res;

typedef struct st_Get_Device_Location_Res {
	st_Comm_Header header;
    st_Body_Get_Device_Location_Res msg;
} st_Get_Device_Location_Res;

#endif /* INC_COMMIF_H_ */