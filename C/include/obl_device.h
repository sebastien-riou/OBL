#pragma once


//optional dependencies

#ifndef OBL_CUSTOM_MEM_ACCESS
static void obl_read8  (uint8_t *dat, uintptr_t addr)	{*dat = *((uint8_t*) addr);}
static void obl_read16 (uint16_t*dat, uintptr_t addr)	{*dat = *((uint16_t*)addr);}
static void obl_read32 (uint32_t*dat, uintptr_t addr)	{*dat = *((uint32_t*)addr);}
static void obl_read64 (uint64_t*dat, uintptr_t addr)	{*dat = *((uint64_t*)addr);}
static void obl_write8 (uint8_t  dat, uintptr_t addr)	{*((uint8_t*)addr)  = dat;}
static void obl_write16(uint16_t dat, uintptr_t addr)	{*((uint16_t*)addr) = dat;}
static void obl_write32(uint32_t dat, uintptr_t addr)	{*((uint32_t*)addr) = dat;}
static void obl_write64(uint64_t dat, uintptr_t addr)	{*((uint64_t*)addr) = dat;}
#endif

#ifndef OBL_CUSTOM_EXEC
typedef int (*obl_execfunc_t)(void);
static int obl_call(uintptr_t addr){
	obl_execfunc_t f = (obl_execfunc_t)addr;
	return f();
}
#endif

#ifndef OBL_CUSTOM_IO
static void obl_putchar_flush(){fflush(stdout);}
static void obl_putchar(char c){putchar(c);}
static char obl_getchar(){return getchar();}
#endif

//generic IO based on obl_putchar and obl_getchar
static uint32_t obl_get_until(uint8_t*buf, int32_t size, uint8_t*sep, unsigned int nsep){
  int i=0;
  while (i+1<size) {
    uint8_t c = obl_getchar();
    buf[i++]=c;
    int found=0;
    for(unsigned int j=0;j<nsep;j++){
      if (c == sep[j]) {
        found=1;
				break;
      }
    }
    if(found) break;
  }
  buf[i]=0;
	return i;
}

static void obl_send_str(const char*msg){
  while(*msg){
    obl_putchar(*msg++);
  }
}

static void obl_send_hex08(uint8_t b){
  const char hex[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  uint8_t h=hex[b >> 4];
  uint8_t l=hex[b & 0xF];
  obl_putchar(h);
  obl_putchar(l);
}

static void obl_send_hex64(uint64_t v){
  for(unsigned int i=0;i<8;i++){
    obl_send_hex08(v>>((7-i)*8));
  }
}

//Commands definition
#define OBL_CMD_CLA_8  	0x08
#define OBL_CMD_CLA_16 	0x10
#define OBL_CMD_CLA_32 	0x20
#define OBL_CMD_CLA_64 	0x40
#define OBL_CMD_INS_READ    0x0A
#define OBL_CMD_INS_WRITE   0x0C
#define OBL_CMD_INS_CALL    0xCC
#define OBL_CMD_INS_EXIT    0xEE
#define OBL_CMD_INS_INFO    0xF0
#define OBL_CMD_READ_8		(OBL_CMD_CLA_8  | (OBL_CMD_INS_READ <<8))
#define OBL_CMD_READ_16		(OBL_CMD_CLA_16 | (OBL_CMD_INS_READ <<8))
#define OBL_CMD_READ_32		(OBL_CMD_CLA_32 | (OBL_CMD_INS_READ <<8))
#define OBL_CMD_READ_64		(OBL_CMD_CLA_64 | (OBL_CMD_INS_READ <<8))
#define OBL_CMD_WRITE_8	 	(OBL_CMD_CLA_8  | (OBL_CMD_INS_WRITE<<8))
#define OBL_CMD_WRITE_16	(OBL_CMD_CLA_16 | (OBL_CMD_INS_WRITE<<8))
#define OBL_CMD_WRITE_32	(OBL_CMD_CLA_32 | (OBL_CMD_INS_WRITE<<8))
#define OBL_CMD_WRITE_64	(OBL_CMD_CLA_64 | (OBL_CMD_INS_WRITE<<8))
#define OBL_CMD_CALL      (OBL_CMD_INS_CALL<<8)
#define OBL_CMD_EXIT      (OBL_CMD_INS_EXIT<<8)
#define OBL_CMD_INFO      (OBL_CMD_INS_INFO<<8)
#define OBL_CMD_INVALID   0

#define OBL_SUCCESS 0
//warnings or errors
#define OBL_ERROR_NOT_SET 1
#define OBL_RECEIVE_EMPTY 2
//real errors
#define OBL_ERROR_RECEIVE_CONV_FAIL 		0x8000
#define OBL_ERROR_UNKNOWN_CMD 		  		0x8001
#define OBL_ERROR_RECEIVE_BYTE_OVERFLOW 0x8002
#define OBL_ERROR_UNEXPECTED_SEP        0x8003
#define OBL_ERROR_RECEIVE_NUM_OVERFLOW  0x8004

#define OBL_DATA_SIZE_LIMIT 0x1000000
#define OBL_OK "ok"
#define OBL_KO "ko"

typedef struct obl_cmd_record_struct {
  const char*str;
  const uint16_t opcode;
} obl_cmd_record_t;

static const obl_cmd_record_t obl_commands[] = {
  {"call",OBL_CMD_CALL},
  {"exit",OBL_CMD_EXIT},
  {"info",OBL_CMD_INFO},
  {"rd08",OBL_CMD_READ_8	},
  {"rd16",OBL_CMD_READ_16	},
  {"rd32",OBL_CMD_READ_32	},
  {"rd64",OBL_CMD_READ_64	},
  {"wr08",OBL_CMD_WRITE_8	},
  {"wr16",OBL_CMD_WRITE_16},
  {"wr32",OBL_CMD_WRITE_32},
  {"wr64",OBL_CMD_WRITE_64}
};
static unsigned int obl_commands_count = sizeof(obl_commands) / sizeof(obl_cmd_record_t);

//specialized IOs
static int obl_strtoul(uint64_t*dst, uint8_t*src, unsigned int len){
	//if(len<=1) return OBL_RECEIVE_EMPTY;
	//len--;//remove sep
	//src[len]=0;
	if(0==len) return OBL_RECEIVE_EMPTY;
	*dst = strtoul (src, NULL, 16);
	if(0==*dst){
		for(unsigned int i=0;i<len;i++){
			if(src[i]!='0') return OBL_ERROR_RECEIVE_CONV_FAIL;
		}
	}
	return OBL_SUCCESS;
}
static int obl_receive_num(uint64_t*num){
  uint8_t sep[]={'\r','\n',' '};
  uint8_t buf[8*2+1+1+1];//buffer larger than needed by one to distinguish separator from over long number
  uint32_t len=obl_get_until(buf, sizeof(buf),sep,sizeof(sep));
	if(0==len) return OBL_RECEIVE_EMPTY;
	if(len>8*2+1) return OBL_ERROR_RECEIVE_NUM_OVERFLOW;
	return obl_strtoul(num,buf,len-1);//-1 to remove separator
}

static int obl_receive_data_byte(uint8_t*b){
  uint8_t sep[]={'\r','\n',' '};
  uint8_t buf[2+1];
  uint32_t len=obl_get_until(buf, sizeof(buf),sep,sizeof(sep));
	uint64_t tmp;
  int status = obl_strtoul (&tmp,buf,len);
	if(status) return status;
	if(tmp>0xFF) return OBL_ERROR_RECEIVE_BYTE_OVERFLOW;
	*b=tmp;
	return OBL_SUCCESS;
}

static int obl_receive_cmd(uint16_t*cmd){
	*cmd = OBL_CMD_INVALID;
  uint8_t sep[]={'\r','\n',' '};
  uint8_t buf[4+1+1+1];
  uint32_t len;
	do{
  	len=obl_get_until(buf, sizeof(buf),sep,sizeof(sep));
		//printf("obl_receive_cmd: len=%d, '%s'\n",len,buf);
		if(0==len) return OBL_RECEIVE_EMPTY;
	}while(buf[0]=='\n');
	char s=buf[len-1];
	buf[len-1]=0;
	for(unsigned int i=0;i<obl_commands_count;i++){
		if(strcmp(obl_commands[i].str,buf)==0) {
      *cmd = obl_commands[i].opcode;
			return OBL_SUCCESS;
    }
  }
	while(s!='\n') s=obl_getchar();//flush the whole line
	return OBL_ERROR_UNKNOWN_CMD;
}

static void obl_send_prompt(){
	obl_send_str("\n>");
	obl_putchar_flush();
}

static void obl_send_data_byte(uint8_t b){
  obl_putchar(' ');
  obl_send_hex08(b);
}

static void obl_send_status(const char*status){
	obl_send_str(status);
	//obl_send_str("\n");//sent by prompt
}

static int obl_main(){
	const char*ok=OBL_OK;
	const char*ko=OBL_KO;
  uint64_t stack_buf[4];
  uint16_t cmd = OBL_CMD_INVALID;
	uint64_t addr;
	uint64_t len;
  while(1){
		int error=OBL_ERROR_NOT_SET;
    //prompt
    obl_send_prompt();
    int status = obl_receive_cmd(&cmd);
		if(status) error=status;
		unsigned int access_unit = cmd & 0xFF;
		//printf("access_unit=%d\n",access_unit);
		do{
	    //common 1st arg
	    switch(cmd){
			case OBL_CMD_READ_8:
			case OBL_CMD_READ_16:
			case OBL_CMD_READ_32:
	    case OBL_CMD_READ_64:
			case OBL_CMD_WRITE_8:
			case OBL_CMD_WRITE_16:
			case OBL_CMD_WRITE_32:
	    case OBL_CMD_WRITE_64:
			case OBL_CMD_CALL:
	      if(obl_receive_num(&addr)) break;
	    }
	    //other args and processing
	    switch(cmd){
	    case OBL_CMD_READ_8:
			case OBL_CMD_READ_16:
			case OBL_CMD_READ_32:
	    case OBL_CMD_READ_64:
	      if(obl_receive_num(&len)) break;
	      if(len>OBL_DATA_SIZE_LIMIT) break;
				obl_send_status(ok);
				unsigned int incr = access_unit>>3;
				for(unsigned int i=0;i<len;i+=incr){
					uint64_t buf=0;
					switch(access_unit){
					case 8:
						obl_read8((uint8_t*)&buf,addr);
	          break;
					case 16:
						obl_read16((uint16_t*)&buf,addr);
	          break;
					case 32:
						obl_read32((uint32_t*)&buf,addr);
						break;
	        case 64:
						obl_read64(&buf,addr);
						break;
					}
	        for(unsigned int i=0;i<access_unit;i+=8){
	          obl_send_data_byte(buf>>i);
	        }
					addr+=incr;
				}
				error=0;
				break;
			case OBL_CMD_WRITE_8:
			case OBL_CMD_WRITE_16:
			case OBL_CMD_WRITE_32:
	    case OBL_CMD_WRITE_64:
				do{
					uint64_t buf=0;
					unsigned int bytes_cnt=0;
	        for(unsigned int i=0;i<access_unit;i+=8){
					  uint8_t b;
						int status = obl_receive_data_byte(&b);
						if(status!=OBL_SUCCESS) {
							error=status;
							break;
						}
						bytes_cnt++;
	          buf|=((uint64_t)b)<<i;
						char c = obl_getchar();
						if(c=='\n'){
							error=OBL_RECEIVE_EMPTY;
							break;
						} else if(c!=' '){
							error=OBL_ERROR_UNEXPECTED_SEP;
							break;
						}
	        }
					if(error!=OBL_ERROR_NOT_SET) {
						if(error!=OBL_RECEIVE_EMPTY) break;
						error = OBL_SUCCESS;
						if(0==bytes_cnt) break;
					}
					//printf("do write access\n");
	      	switch(access_unit){
					case 8:
						obl_write8(buf,addr);
						break;
					case 16:
						obl_write16(buf,addr);
						break;
					case 32:
						obl_write32(buf,addr);
						break;
	        case 64:
						obl_write64(buf,addr);
						break;
					}
					addr+=access_unit>>3;
				}while(error==OBL_ERROR_NOT_SET);
				obl_send_status(ok);error = OBL_SUCCESS;
				break;
			case OBL_CMD_CALL:
	      if(obl_call(addr)) obl_send_status(ok);
				error=OBL_SUCCESS;
	      break;
			case OBL_CMD_EXIT:{
				uint8_t b;
				int status = obl_receive_data_byte(&b);
				if(status!=OBL_SUCCESS) {
					error=status;
					break;
				}
				obl_send_status(ok);error=OBL_SUCCESS;
				obl_putchar('\n');
				char r = (char)b;
				return r;
				break;}
	    case OBL_CMD_INFO:
	      obl_send_status(ok);error=OBL_SUCCESS;
	      obl_send_str(" stack_buf=0x");
	      obl_send_hex64((uint64_t)(uintptr_t)stack_buf);
	      obl_send_str(", obl_main=0x");
	      obl_send_hex64((uint64_t)(uintptr_t)obl_main);
	      break;
	    }
		}while(0);
		if(error) {
			obl_send_status(ko);
			obl_putchar(' ');
			obl_send_hex64(error);
		}
  }
	return -1;//shall never happen
}
