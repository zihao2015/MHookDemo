#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "Substrate.h"
#include "HFile/NativeLog.h"
//#define _DEBUG_			1

//dvm  Hook 段
#define libdvm			"/system/lib/libdvm.so"
#define dexFileParse	"_Z12dexFileParsePKhji"
#define dvmDexFileOpenFromFd "_Z20dvmDexFileOpenFromFdiPP6DvmDex"
#define dvmDexFileOpenPartial "_Z21dvmDexFileOpenPartialPKviPP6DvmDex"
//libc
#define libc			"/system/lib/libc.so"
#define open			"open"
#define fopen			"fopen"
#define mmap			"mmap"
#define ptrace			"ptrace"
#define free			"free"
#define munmap			"munmap"

//调用外部Dump_DexFile
extern void Dump_DexFile(void* inAddr,size_t inLen,void* inDex);
//__________________________________________________________________________________________
//									Libdvm
void* (*_dexFileParse)(int *, unsigned int, int);
void* My_dexFileParse(int *inAddr, unsigned int length, int parseFlags){
	void* Out_DexFile = _dexFileParse(inAddr,length,parseFlags);
	LOGD("LibCall dexFileParse:inDex:0x%08X,length:0x%08X,DexFile:0x%08X",inAddr,length,Out_DexFile);
	//屏蔽长度过小的长度
/*	if(length > 0x2A00){
		Dump_DexFile(inAddr,length,Out_DexFile);
	}/**/
	Dump_DexFile(inAddr,length,Out_DexFile);
	return Out_DexFile;
}
//dvmDexFileOpenFromFd
int (*_dvmDexFileOpenFromFd)(int fd, void* ppDvmDex);
int My_dvmDexFileOpenFromFd(int fd, void* ppDvmDex){
	LOGD("LibCall My_dvmDexFileOpenFromFd");
	return _dvmDexFileOpenFromFd(fd,ppDvmDex);
}
//dvmDexFileOpenPartial
int (*_dvmDexFileOpenPartial)(const void* addr, int len, void* ppDvmDex);
int My_dvmDexFileOpenPartial(const void* addr, int len, void* ppDvmDex){
	LOGD("LibCall _dvmDexFileOpenPartial");
	return _dvmDexFileOpenPartial(addr,len,ppDvmDex);
}
//__________________________________________________________________________________________
//									Libc
//open	int open(const char*pathname,int flags);
void* (*_open)(const char * path,int flags);
void* My_open(const char * path,int flags){
	LOGD("LibCall open Path:%s,Mode:%d",path,flags);
	return _open(path,flags);
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
//ptrace
int (*_ptrace)(int request, int pid, int addr, int data);
int My_ptrace(int request, int pid, int addr, int data){
	int mres = _ptrace(request,pid,addr,data);
	LOGD("LibCall _ptrace in %08x %08x %08x %08x %08x %08x",request,pid,addr,data);
	LOGD("LibCall _ptrace res %08x ",mres);
	return mres;
}
//free
void (*_free)(void *ptr);
void My_free(void *ptr){
	LOGD("LibCall _free %08x",ptr);
	_free(ptr);
}
// int munmap(void *start,size_t length);
int (*_munmap)(void *start,size_t length);
int My_munmap(void *start,size_t length){
	LOGD("LibCall _munmap %08x %08x",start,length);
	return _munmap(start,length);
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
		mFun = MSFindSymbol(image,dvmDexFileOpenFromFd);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_dvmDexFileOpenFromFd,(void**)&_dvmDexFileOpenFromFd);
		}
		mFun = MSFindSymbol(image,dvmDexFileOpenPartial);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_dvmDexFileOpenPartial,(void**)&_dvmDexFileOpenPartial);
		}
	}
	// Hook fopen，
#ifdef 	_DEBUG_
	image = MSGetImageByName(libc);
	if(image != NULL){
		mFun = MSFindSymbol(image,open);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_open,(void**)&_open);
		}
		mFun = MSFindSymbol(image,fopen);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_fopen,(void**)&_fopen);
		}
		mFun = MSFindSymbol(image,ptrace);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_ptrace,(void**)&_ptrace);
		}
		mFun = MSFindSymbol(image,free);
		if(mFun != NULL){
		//	MSHookFunction(mFun,(void*)&My_free,(void**)&_free);
		}
		mFun = MSFindSymbol(image,munmap);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_munmap,(void**)&_munmap);
		}

	}
#endif
	return 0;
}

