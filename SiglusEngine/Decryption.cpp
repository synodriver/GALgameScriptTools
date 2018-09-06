#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

extern "C"
{
	__declspec(dllexport) void decrypt1(unsigned char* buf,size_t size,unsigned char* key);
	__declspec(dllexport) void decrypt2(unsigned char* buf,size_t size);
	__declspec(dllexport) void decompress(unsigned char* inBuf,unsigned char* outBuf,int size);
	__declspec(dllexport) void fakeCompress(unsigned char* inBuf,unsigned char* outBuf,int size);
	__declspec(dllexport) unsigned char* compress(unsigned char* inBuf, int inSize, int *compLen ,int level);
	int searchData(unsigned char *buf,int dataSize,int compLevel,unsigned char *compBuf,int *compLen);
	int match(unsigned char *inBuf,unsigned char *matchBuf,int size);
}

void decrypt1(unsigned char* buf,size_t size,unsigned char* key)
{
	size_t keyIndex=0;
	for(size_t index=0;index<size;index++)
	{
		buf[index]^=key[keyIndex];
		keyIndex++;
		if(keyIndex>15)
			keyIndex=0;
	}
}

void decrypt2(unsigned char* buf,size_t size)
{
	static unsigned char key[]={
	0x70,0xF8,0xA6,0xB0,0xA1,0xA5,0x28,0x4F,0xB5,0x2F,0x48,0xFA,0xE1,0xE9,0x4B,0xDE,
  	0xB7,0x4F,0x62,0x95,0x8B,0xE0,0x03,0x80,0xE7,0xCF,0x0F,0x6B,0x92,0x01,0xEB,0xF8,
	0xA2,0x88,0xCE,0x63,0x04,0x38,0xD2,0x6D,0x8C,0xD2,0x88,0x76,0xA7,0x92,0x71,0x8F,
	0x4E,0xB6,0x8D,0x01,0x79,0x88,0x83,0x0A,0xF9,0xE9,0x2C,0xDB,0x67,0xDB,0x91,0x14,
	0xD5,0x9A,0x4E,0x79,0x17,0x23,0x08,0x96,0x0E,0x1D,0x15,0xF9,0xA5,0xA0,0x6F,0x58,
	0x17,0xC8,0xA9,0x46,0xDA,0x22,0xFF,0xFD,0x87,0x12,0x42,0xFB,0xA9,0xB8,0x67,0x6C,
	0x91,0x67,0x64,0xF9,0xD1,0x1E,0xE4,0x50,0x64,0x6F,0xF2,0x0B,0xDE,0x40,0xE7,0x47,
	0xF1,0x03,0xCC,0x2A,0xAD,0x7F,0x34,0x21,0xA0,0x64,0x26,0x98,0x6C,0xED,0x69,0xF4,
	0xB5,0x23,0x08,0x6E,0x7D,0x92,0xF6,0xEB,0x93,0xF0,0x7A,0x89,0x5E,0xF9,0xF8,0x7A,
	0xAF,0xE8,0xA9,0x48,0xC2,0xAC,0x11,0x6B,0x2B,0x33,0xA7,0x40,0x0D,0xDC,0x7D,0xA7,
	0x5B,0xCF,0xC8,0x31,0xD1,0x77,0x52,0x8D,0x82,0xAC,0x41,0xB8,0x73,0xA5,0x4F,0x26,
	0x7C,0x0F,0x39,0xDA,0x5B,0x37,0x4A,0xDE,0xA4,0x49,0x0B,0x7C,0x17,0xA3,0x43,0xAE,
	0x77,0x06,0x64,0x73,0xC0,0x43,0xA3,0x18,0x5A,0x0F,0x9F,0x02,0x4C,0x7E,0x8B,0x01,
	0x9F,0x2D,0xAE,0x72,0x54,0x13,0xFF,0x96,0xAE,0x0B,0x34,0x58,0xCF,0xE3,0x00,0x78,
	0xBE,0xE3,0xF5,0x61,0xE4,0x87,0x7C,0xFC,0x80,0xAF,0xC4,0x8D,0x46,0x3A,0x5D,0xD0,
	0x36,0xBC,0xE5,0x60,0x77,0x68,0x08,0x4F,0xBB,0xAB,0xE2,0x78,0x07,0xE8,0x73,0xBF};

	size_t keyIndex=0;
	for(size_t index=0;index<size;index++)
	{
		buf[index]^=key[keyIndex];
		keyIndex++;
		if(keyIndex>255)
			keyIndex=0;
	}
}

void decompress(unsigned char* inBuf,unsigned char* outBuf,int size){
	void *end=outBuf+size;
	while(outBuf<end){
		short s=8,byte=*inBuf++;
		while(s>0 && outBuf!=end){
			if(byte&1){
				*outBuf++=*inBuf++;
			}
			else{
				unsigned short data=*(unsigned short*)inBuf++;
				int tempData=(data&0x0f)+2;
				data>>=4;
				while(tempData-->0){
					*outBuf=*(outBuf-data);
					*outBuf++;
				}
				*inBuf++;
			}
			s--;
			byte>>=1;
		}
	}
}

void fakeCompress(unsigned char* inBuf,unsigned char* outBuf,int size){
	void *end=inBuf+size;
	outBuf+=8;
	for(int count=8;inBuf<end;count++){
		if(count==8){
			*outBuf++=0xFF;
			count=0;
		}
		*outBuf++=*inBuf++;
	}
}

unsigned char* compress(unsigned char* inBuf, int inSize, int *compLen ,int level){
	if(level<2){
		level=2;
	}else if(level>17){
		level=17;
	}
	unsigned char *outBuf=(unsigned char*)calloc(inSize*2,sizeof(unsigned char)),*outPtr=outBuf;
	int ptr=0,outLen=0,s=0;
	outBuf+=8;
	while(ptr<inSize){
		unsigned char *byteBuf=outBuf;
		outLen++,outBuf++;
		for(s=0;s<8;s++){
			int offset=0,copyLen=0,scan=ptr,curLevel=level;
			if(scan>0xFFF){
				scan=0xFFF;
			}
			if(ptr<curLevel){
				curLevel=ptr;
			}
			if(ptr+curLevel>inSize){
				curLevel=inSize-ptr;
			}
			if(curLevel>1){
				offset=searchData(inBuf,scan,curLevel,inBuf,&copyLen);
			}
			if(offset==0){
				*byteBuf+=1<<s;
				*outBuf++=*inBuf++;
				ptr++;
				outLen++;
			}else{
				*(unsigned short*)outBuf=(offset<<4)+(copyLen-2);
				inBuf+=copyLen;
				ptr+=copyLen;
				outBuf+=2;
				outLen+=2;
			}
		}
	}
	outLen+=8;
	*(int*)outPtr=outLen;
	*(int*)(outPtr+4)=inSize;
	*compLen=outLen;
	return outPtr;
}

int searchData(unsigned char *buf,int dataSize,int compLevel,unsigned char *compBuf,int *compLen){
	unsigned char *ptr=buf;
	int curOffset=0,curCompLen=0,tempLen=0;
	for(ptr=buf-1;ptr>=buf-dataSize;ptr--){
		if(*(unsigned short*)ptr==*(unsigned short*)compBuf){
			tempLen=match(ptr,compBuf,compLevel);
			if(tempLen>curCompLen){
				curCompLen=tempLen;
				curOffset=buf-ptr;
				if(curCompLen==compLevel){
					break;
				}
			}
		}
	}
	*compLen=curCompLen;
	return curOffset;
}

int match(unsigned char *inBuf,unsigned char *matchBuf,int size){
	int i;
	for(i=0;i<size;i++){
		if(inBuf[i]!=matchBuf[i]){
			break;
		}
	}
	return i;
}