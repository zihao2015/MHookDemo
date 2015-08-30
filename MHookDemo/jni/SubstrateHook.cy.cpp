#include <stdio.h>
#include <unistd.h>
#include "Substrate.H"
#include <sys/stat.h>
#include "Common/Common.H"
#include "Hook/Hook.H"
#include "HFile/NativeLog.h"
//存放读取的配置文件
char* Config = NULL;
char* AppName = NULL;
//存放全部需要进程Hook的进程名
MSConfig(MSFilterLibrary, "/system/lib/libdvm.so");
//Dvm函数对应表
#define libdvm		"/system/lib/libdvm.so"
#define dvmLoadNativeCode	"_Z17dvmLoadNativeCodePKcP6ObjectPPc"
//libc函数对应表
#define libc		"/system/lib/libc.so"
#define jni_fork	"fork"
//Hook dvmLoadNativeCode
bool (*_dvmLoadNativeCode)(char* pathName, void* classLoader, char** detail);
bool My_dvmLoadNativeCode(char* pathName, void* classLoader, char** detail){
	//获取打印加载SO  进程和ID
	char* mName = getSelfCmdline();
	LOGD("My_dvmLoadNativeCode ");
	LOGD("Config:%s",Config);
	LOGD("pathName:%s",pathName);
	LOGD("name:%s,tid:%d",mName,gettid());
	//判断是否找到工作路径, 先查找工作路径
	if(AppName == NULL){
		//判断/data/data/%s/ ,  mName是否存在
		char* data_data = (char*)malloc(1024);
		memset(data_data,0,1024);
		sprintf(data_data,"/data/data/%s/",mName);
		struct stat buf;
		//判断目录是否存在，不存在就继续查找
		if(stat(data_data, &buf) == -1){
			//判断加载的文件是否来自/data/data目录
			memset(data_data,0,1024);
			//memcpy(data_data,pathName,strlen(pathName));
			if(strstr(pathName,"/data/data/") != NULL){
				memcpy(data_data,pathName+strlen("/data/data/"),strlen(pathName)-strlen("/data/data/"));
				strtok(data_data,"/");
				AppName = strdup(data_data);	
			}else if(strstr(pathName,"/data/app-lib/") != NULL){
				memcpy(data_data,pathName+strlen("/data/app-lib/"),strlen(pathName)-strlen("/data/app-lib/"));
				strtok(data_data,"-");
				strtok(data_data,"/");
				AppName = strdup(data_data);
			}
		}else{
			AppName = (char*)malloc(strlen(mName)+1);
			memset(AppName,0,strlen(mName)+1);
			memcpy(AppName,mName,strlen(mName));
		}
		free(data_data);	
	}
	//判断AppName是否获取到，获取到打印日志
	if(AppName != NULL){
		LOGD("dvmLoadNativeCode AppName:%s",AppName);	
	}
	//配置文件不存在，直接退出
	if(Config == NULL)return _dvmLoadNativeCode(pathName,classLoader,detail);

	//判断配置文件中是否有进程名，有进行HOOK
	if((strstr(Config,mName)!= NULL)){
		LOGD("dvmLoadNativeCode Hook_Main");
		Hook_Main();
	}else{
	//再判断加载路径中是能和配置文件匹配，匹配则Hook
		char *delim = ",";
		char* msrc = (char*)malloc(strlen(Config)+1);
		memset(msrc,0,strlen(Config)+1);
		memcpy(msrc,Config,strlen(Config));
		char *p  =  strtok(Config, delim);
		if(*p == NULL)return _dvmLoadNativeCode(pathName,classLoader,detail);
		do{
			if(strstr(pathName,p) != NULL){
				LOGD("dvmLoadNativeCode Hook_Main");
				Hook_Main();
				free(msrc);
				return _dvmLoadNativeCode(pathName,classLoader,detail);	
			}
		}while(p = strtok(NULL, delim));
		free(msrc);
	}
	//返回旧函数
	return _dvmLoadNativeCode(pathName,classLoader,detail);
}
//__________________________________________________________
int (*_fork)(void);
int My_fork(void){
	int result = _fork();
	if(result == 0){
		if(Config != NULL){
			free(Config);
		}
		//获取配置文件
		Config = getConfig();
	}
	return result;
}
/**
 *			MSInitialize
 *	程序入口点，
 * 		一定是最开始运行，但是不一定是进程中最开始运行
 *
 */

MSInitialize
{
	//获取当前进程号，名称
	char* mName = getSelfName();
	LOGD("MSInitialize name:%s,tid:%d",mName,gettid());	
	//获取配置文件
	Config = getConfig();
	LOGD("MSInitialize Config:%s",Config);
	LOGD("MSInitialize@Hook dvmLoadNativeCode");	
	//开始Hook 一些基本函数
	MSImageRef image = MSGetImageByName(libdvm);
	void* mFun = NULL;
	if(image != NULL){		
		mFun = MSFindSymbol(image,dvmLoadNativeCode);
		if(mFun != NULL){
			MSHookFunction(mFun,(void*)&My_dvmLoadNativeCode,(void**)&_dvmLoadNativeCode);
		}
	}
	image = MSGetImageByName(libc);
	if(image != NULL){		
		mFun = MSFindSymbol(image,jni_fork);
		//开始Hook fork
		if(mFun != NULL){
//			MSHookFunction(mFun,(void*)&My_fork,(void**)&_fork);
		}
	}/**/
}


