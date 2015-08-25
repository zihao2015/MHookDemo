#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H"
#include "DexParse.H"
//
extern char* AppName;
//__________________________________________________________
/*
*					Dump说明
* 1.传入参数为DexFile*
* 2.通过被解析的DexFile*,得知BassAddr和Dex Length
* 3.将Dex内存连续的数据全部获取
* 5.保存文件
*/
//__________________________________________________________
/*
*				关于修复Dex_AnnotDirDex_AnnotDir
* 1.不需要修复Mapitems
* 2.根据DexFile结构体，判断需要的内存是否的Dex所需的内存内。
*	不存在内存中就要进行修复偏移。
* 3.
*/
//__________________________________________________________
/*
************************************************************
*			一次运行DUMP出来三个文件
*版本1.直接DUMP数据
*版本2.延时DUMP数据
*版本3.根据DexFile之修复Class Data Sources Offset
//*版本2.Dump出dexFileParse函数返回的DexFile指针
*版本3.原本调用dexFileParse函数调用的内存和宽度，在利用dexFileParse分析，在DUMP
************************************************************
*/
void* Dex_Parse(void* in){
	DumpInfo* Info = (DumpInfo*)in;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	char* saveName = (char*)malloc(1024);
	memset(saveName,0,1024);
	LOGD("inDex:0x%08X,length:0x%08X,DexFile:0x%08X",Info->addr,Info->len,Info->Dex);
	DumpD("先直接DUMP dexFileParse 函数传进来的参数!");
	//版本1.Dump
	DumpD("@下载Demo1!");
	Mod_Mem *mem = new Mod_Mem();
	str_ModMem* mMem = mem->newNameMem("Demo1",Info->len);
	memcpy(mMem->Addr,(void*)Info->Dex->pHeader,Info->Dex->pHeader->fileSize);
	sprintf(saveName,"Demo1_%08x.dex",tv.tv_sec);
	mem->SaveFile(AppName,saveName);
	DumpD("延时15S!");
	sleep(5);
	DumpD("下载Demo2!");
	//版本2.Dump
	mem = new Mod_Mem();
	mMem = mem->newNameMem("Demo2",Info->len);
	memcpy(mMem->Addr,(void*)Info->Dex->pHeader,Info->Dex->pHeader->fileSize);
	sprintf(saveName,"Demo2_%08x.dex",tv.tv_sec);
	mem->SaveFile(AppName,saveName);
	DumpD("下载Demo3 ing!");
	//版本3.只修复ClassDef
	DexParse* parse = new DexParse(Info->addr,Info->Dex);
	memset(saveName,0,1024);
	sprintf(saveName,"Demo3_%08x.dex",tv.tv_sec);
	parse->DumpToFile(AppName,saveName);
	//
	return NULL;
}
/*
************************************************************
*				Dump_DexFile
* @param	inAddr -> 指向DexFile
* @cmt	创建一个线程，然后用线程去执行解码 DexFile
************************************************************
*/
void Dump_DexFile(void* inAddr,size_t inLen,void* inDex){
	DumpD("Dump_DexFile@创建子线程!");
	DumpInfo* info = (DumpInfo*)malloc(sizeof(DumpInfo)+1);
	memset(info,0,sizeof(DumpInfo)+1);
	info->addr = inAddr;
	info->len = inLen;
	info->Dex = (DexFile*)inDex;
	pthread_t thread;
	pthread_create(&thread,NULL,Dex_Parse,info);
}
