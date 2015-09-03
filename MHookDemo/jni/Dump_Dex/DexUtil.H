#ifndef __DexUtil_H_
#define __DexUtil_H_
#define AddAddr2(arg0,arg1)		((u4)arg0+(u4)arg1)
#define SubAddr2(arg0,arg1)		((u4)arg0-(u4)arg1)
#define SubAddr3(arg0,arg1,arg2)		((u4)arg0-(u4)arg1-(u4)arg2)
#define Ali(arg0,arg1)			(((u4)arg0+((u4)arg1-1))&(~((u4)arg1-1)))
/*
 *			单向链接基本类
 * 1.Offset_Load表示内存加载地址
 * 2.Offset_File表示文件偏移地址
 * 3.next指向下一个链表
 * 4.MemSize表示内存宽度
 */
typedef struct Base{
	u4	MemSize;
	u4	Offset_Load;
	u4	Base_Load;
	u4	Offset_File;
	u4	Base_File;
/*
************************************************************
*			GetLoad
*获取加载内存地址*
************************************************************
*/
	u4 GetLoad(){
		return Base_Load+Offset_Load;
	}
/*
************************************************************
*			GetLoad
*获取加载内存地址*
************************************************************
*/
	void SetLoad(u4 inBase,u4 inOffset){
		Base_Load = inBase;
		if(inOffset > inBase){
			Offset_Load = inOffset - inBase;
		}else{
			Offset_Load = inOffset;
		}
	}
/*
************************************************************
*			GetLoad
*获取加载内存地址*
************************************************************
*/
	void SetOffset(char* inAddr,char* inBase){
		if(inAddr < inBase){
			DEXLOG("[ERR]设置偏移地址失败");
			return ;
		}
		Offset_File = (u4)(inAddr-inBase);
	}
/*
************************************************************
*			SetBaseLoad
*设置加载内存基准地址*
************************************************************
*/
	void SetBaseLoad(u4 inOffset){
		Base_Load = inOffset;
	}
/*
************************************************************
*			SetLoadOffset
*设置内存偏移地址*
************************************************************
*/
	void SetLoadOffset(u4 inOffset){
		Offset_Load = inOffset;
	}
/*
************************************************************
*			SetFileOffset
*设置文件偏移地址*
************************************************************
*/
	void SetFileOffset(u4 inOffset){
		Offset_File = inOffset;
	}
/*
************************************************************
*			SetMemSize
*设置内存宽度*
************************************************************
*/
	void SetMemSize(u4 inMemSize){
		MemSize = inMemSize;
	}
}Base;
/*
************************************************************
*				Link
*Dex通用类
************************************************************
*/
typedef struct Link{
	struct Link* next = NULL;
	struct Base	 BS;
}Link;
/*
************************************************************
*				AddToLast
*将数据添加到链表
*如果不保存连接最后指，使用AddToLast是一个非常耗时的操作
*所以使用一个AddLink函数
************************************************************
*/
void* AddToLast(void* inSrc,void* in_next){
	if(NULL == inSrc)return in_next;
	Link* Src = (Link*)inSrc;
	while(NULL != Src->next){
		Src = Src->next;
	}
	Src->next = (Link*)in_next;
	return inSrc;
}
/*
************************************************************
*				AddLink
*将数据添加到链表
*只是设置链表指向，返回下一个链表
************************************************************
*/

void* AddLink(void* inSrc,void* in_next){
	if((NULL == inSrc)||(NULL == in_next))return NULL;
	Link* Src = (Link*)inSrc;
	Src->next = (Link*)in_next;
	return in_next;
}
/*
************************************************************
*				GetSize
*将数据添加到链表
************************************************************
*/
size_t GetSize(void* inlink){
	Link* link =(Link*)inlink;
	size_t mSize = 0;
	while(link != NULL){
		mSize += link->BS.MemSize;
		link = link->next;
	}
	return mSize;
}
/*
************************************************************
*				GetSize
*将数据添加到链表
************************************************************
*/
size_t GetCount(void* inlink){
	Link* link =(Link*)inlink;
	size_t mSize = 0;
	while(link != NULL){
		mSize++;
		link = link->next;
	}
	return mSize;
}
/*
************************************************************
*				GetLastLink
*将数据添加到链表
************************************************************
*/
void* GetLastLink(void* inlink){
	Link* link =(Link*)inlink;
	if(link == NULL)return NULL;
	while(link->next != NULL){
		link = link->next;
	}
	return link;
}
/*
************************************************************
*			DexUtil
*Dex通用类
************************************************************
*/
class DexUtil{
public:
/*
************************************************************
*			NEXTLeb128
* @param	inAddr -> 数据起始地址
* @return	result -> 返回下一个数据地址
************************************************************
*/
static unsigned char* NEXTLeb128(void* inAddr){
	unsigned char* in_Addr =(unsigned char*)inAddr;
	int m_i;
	for(m_i = 0;m_i < 5;m_i++){
		if(*(in_Addr+m_i) <= 0x7f){
			return in_Addr+m_i+1;
		}
	}
	return in_Addr+m_i+1;
}
/*
************************************************************
*			readLeb128
* @param	inAddr ->	数据起始地址
* @return	result ->	读取readLeb128(unsigned leb128)数据
************************************************************
*/
static u4 readLeb128(void* inAddr){
	unsigned char* in_Addr = (unsigned char*)inAddr;
	u4 result = 0;
	for(int m_i = 0;m_i < 5;m_i++){
		u1 m_byte = *(in_Addr + m_i);
		result |= (m_byte&0x7F)<<(m_i*7);
		if(m_byte <= 0x7f){
			return result;
		}
	}
	return result;
}/*
************************************************************
*			readSLeb128
* @param	inAddr ->	数据起始地址
* @return	result ->	读取readLeb128(signed leb128)数据
************************************************************
*/
static u4 readSLeb128(unsigned char* inAddr){
	u4	result_2 = 0xFFFFFFFF;
	u4	result_1 = 0x00000000;
	for(int m_i = 0;m_i < 5;m_i++){
		char m_byte = *(inAddr + m_i);
		result_2 &= ~(0x7F<<(m_i*7));
		result_1 |= (m_byte&0x7F)<<(m_i*7);
		result_2 |= (m_byte&0x7F)<<(m_i*7);
		if((m_byte & 0x80) == 0)
		{
			if((m_byte & 0x40)){
				return result_2;
			}
			return result_1;
		}
	}
	return result_1;
}/*
************************************************************
*			readLeb128
* @param	inAddr ->	数据起始地址
* @param	inDeta ->	数据指针—存放数据
* @return	result ->	返回下一个数据地址
************************************************************
*/
static unsigned char* readLeb128(void* inAddr,u4* inDeta){
	*inDeta = readLeb128(inAddr);
	return NEXTLeb128(inAddr);
}/*
************************************************************
*			readSLeb128
* @param	inAddr ->	数据起始地址
* * @param	inDeta ->	数据指针—存放数据
* @return	result ->	返回下一个数据地址
************************************************************
*/
static unsigned char* readSLeb128(unsigned char* inAddr,u4* inDeta){
	*inDeta = readSLeb128(inAddr);
	return NEXTLeb128(inAddr);
}
/*
************************************************************
*			readLimitLeb128
* @param	inAddr ->	数据起始地址
* @return	result ->	返回Leb128数据宽度
************************************************************
*/
static u4 readLimitLeb128(unsigned char* inAddr){
	int m_i = 0;
	for(m_i = 0;m_i < 5;m_i++){
		if(*(inAddr+m_i) <= 0x7f){
			return  m_i+1;
		}
	}
	return  m_i+1;
}
/*
************************************************************
*			writeL128
* @param	inAddr ->	数据起始地址
* @param	mData ->	数据
* @return	result ->	返回Leb128数据宽度
************************************************************
*/
static unsigned char* writeL128(unsigned char* inAddr,u4 mData){
	int m_i;
	for(m_i = 0;m_i< 5;m_i++){
		u1 mByte =	mData &0x7F;
		mData >>= 7;
		if(mData != 0){
			*(inAddr+m_i) = mByte|0x80;
		}else{
			*(inAddr+m_i) = mByte&0x7F;
			return NEXTLeb128(inAddr);
		}
	}
	return NEXTLeb128(inAddr);
}
static unsigned char* writeSL128(unsigned char* inAddr,u4 mData){
	if((mData&0x80000000) == 0){
		writeL128(inAddr,mData);
	}else{
		unsigned int m_Data = 0xFFFFFFFF;
		int m_i;
		for(m_i = 0;m_i< 5;m_i++){
			u1 mByte = (mData>>(m_i*7));
			m_Data >>= 7;
			mData >>= 7;
			if((m_Data^mData) == 0){
				*(inAddr+m_i) = mByte&0x7F;
				return NEXTLeb128(inAddr);

			}else{
				*(inAddr+m_i) = mByte|0x80;
			}
		}
	}
	return NEXTLeb128(inAddr);
}
/*
************************************************************
*			writeAdd128_Limit
*会根据原先数据宽度设置数据
* @param	inAddr ->	数据起始地址
* @param	mData ->	数据
* @return	result ->	返回Leb128数据宽度
************************************************************
*/
static int writeAdd128_Limit(unsigned char* inAddr,u4 inData){
	u4 mLength = readLimitLeb128(inAddr);	for(int m_i = 0;m_i< mLength;m_i++){
		u1 mByte = (inData>>(m_i*7))&0x7F;
		if((m_i+1)!= mLength){
			*(inAddr+m_i) = mByte|0x80;
		}else{
			*(inAddr+m_i) = mByte;
		}
	}
	return 0;
}
/*
************************************************************
*			Alloc
*获取清零内存
* @param	inSize ->	内存宽度
* @return	out ->		内存指针
************************************************************
*/
	static void* Alloc(size_t inSize){
		void* out = malloc(inSize);
		memset(out,0,inSize);
		return out;
	}
/*
************************************************************
*			Align
*对齐设置
* @param	inAddr ->	结束地址
* @param	inAli ->	对齐宽度
* @return	out ->		对齐需要宽度
************************************************************
*/
	static size_t Align(size_t inAddr,size_t inAli){
		size_t mSzie = inAli - inAddr&(inAli-1);
		return  mSzie;
	}

/*
************************************************************
*			AlignAdd
*对齐设置
* @param	inAddr ->	结束地址
* @param	inAli ->	对齐宽度
* @return	out ->		对齐需要宽度
************************************************************
*/
	size_t AlignAdd(size_t inAddr,size_t inAli){
		size_t mSzie = inAli - inAddr&(inAli-1);
		return  inAddr+mSzie;
	}
/*
************************************************************
*			isDex
*判断是否是Dex文件
* @param	inAddr ->	Dex起始地址
* @return	bool ->		0->不是Dex或ODex文件
* 						1->存在Dex或ODex的Magic
************************************************************
*/
	static bool isDex(void* inAddr){
		if((0 == memcmp(inAddr,"dex",3))||(0 == memcmp(inAddr,"dey",3))){
			return 1;
		}
		return 0;
	}
/*
************************************************************
*			SaveFile
*将内存数据保存成文件
* @param	addr ->		内存起始地址
* @param	inLen->		内存宽度
* @param	appName ->	应用名字
* @param	inFile ->	文件名字
************************************************************
*/
	static void SaveFile(void* addr,size_t inLen,char* appName,char* inFile){
		char* filePath = (char*)malloc(1024);
		memset(filePath,0,1024);
		sprintf(filePath,"/sdcard/DumpDex/%s",inFile);
		FILE* mfd=fopen(filePath,"wb");
		//查找可写文件路径
		if(!mfd){
			DEXLOG("[Dump]Fail SD Path:%s",filePath);
			DEXLOG("[Dump]Mod_Mem::SaveFile() SD Open file fail!");
			memset(filePath,0,1024);
			if(appName != NULL){
				sprintf(filePath,"/data/data/%s/%s",appName,inFile);
				mfd=fopen(filePath,"wb");
				if(!mfd){
					DEXLOG("[Dump]Fail Data_Data Path:%s",filePath);
					DEXLOG("[Dump]Mod_Mem::SaveFile() Data Open file fail!");
					free(filePath);
					return;
				}
			}else{
				DEXLOG("[Dump]Fail appName Is Null");
				free(filePath);
				return ;
			}
		}
		fwrite(addr,1,inLen,mfd);
		fclose(mfd);
		DEXLOG("[Dump]Dump File Path:%s",filePath);
		free(filePath);
	}
/*
************************************************************
*			GetTimeName
*获取实时时间名字
* @param	inName ->	名字前缀
************************************************************
*/
	static char*  GetTimeName(char* inName){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		int m_HookM = (tv.tv_sec/60)%60;
		int m_HookH = (tv.tv_sec/3600)%24;
		char* mName = (char*)Alloc(1024);
		sprintf(mName,"%s_%02d_%02d",inName,m_HookH,m_HookM);
		return mName;
	}
/*
************************************************************
*				Dex_Header
*解析头文件
************************************************************
*/
	static void* GetBase(void* inAddr){
		u1* mAddr =(u1*)inAddr;
		if(0 == memcmp(mAddr,"dey",3)){
			return mAddr+0x28;
		}else if(0 == memcmp(mAddr,"dex",3)){
			return mAddr;
		}
		return NULL;
	}
};


#endif
