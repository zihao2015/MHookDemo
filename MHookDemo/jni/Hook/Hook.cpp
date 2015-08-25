#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "Substrate.h"
#include "HFile/NativeLog.h"
//dvm  Hook 段
#define libdvm			"/system/lib/libdvm.so"
#define dexFileParse	"_Z12dexFileParsePKhji"
#define libc			"/system/lib/libc.so"
#define fopen			"fopen"
#define mmap			"mmap"
//调用外部Dump_DexFile
extern void Dump_DexFile(void* inAddr,size_t inLen,void* inDex);
//
void* (*_dexFileParse)(int *, unsigned int, int);
void* My_dexFileParse(int *inAddr, unsigned int length, int parseFlags){
	void* Out_DexFile = _dexFileParse(inAddr,length,parseFlags);
	LOGD("LibCall dexFileParse:inDex:0x%08X,length:0x%08X,DexFile:0x%08X",inAddr,length,Out_DexFile);
	//屏蔽长度过小的长度
	if(length > 0x2A00){
		Dump_DexFile(inAddr,length,Out_DexFile);
	}
	return Out_DexFile;
}
//fopen
void* (*_fopen)(const char * path,const char * mode);
void* My_fopen(const char * path,const char * mode){
	LOGD("LibCall fopen Path:%s,Mode:%s",path,mode);
	return _fopen(path,mode);
}
//mmap
void*(*_mmap)(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void* My_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset){
	void* Result =_mmap(addr,len,prot,flags,fd,offset);
	LOGD("LibCall _mmap %08x %08x %08x %08x %08x %08x",addr,len,prot,flags,fd,offset);
	LOGD("LibCall _mmap Result %08x ",Result);
	return Result;
}

/**
 *			Hook_Main
 *align_Len
 */
static bool first = true;
int Hook_Main(){
	LOGD("Hook_Main");
	if(first != true)return -1;
	first = false;
	LOGD("Hook_Main StartHook");
	MSImageRef image = MSGetImageByName(libdvm);
	void* mFun = NULL;
	if(image != NULL){		
		mFun = MSFindSymbol(image,dexFileParse);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_dexFileParse,(void**)&_dexFileParse);
		}
	}
	// Hook fopen，
	image = MSGetImageByName(libc);
		if(image != NULL){
			mFun = MSFindSymbol(image,fopen);
			if(mFun != NULL){
				MSHookFunction(mFun,(void*)&My_fopen,(void**)&_fopen);
			}
		}
	return 0;
}

