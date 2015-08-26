#include "stdio.h"
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
/*
************************************************************
* 获取进程名字
************************************************************
*/
char* getpidName(pid_t inpid){
	char proc_pid_path[1024];
	char buf[1024];
	char* outname = NULL;
	memset(buf,0,1024);
	memset(proc_pid_path,0,1024);
	sprintf(proc_pid_path, "/proc/%d/status", inpid);
	FILE* fp = fopen(proc_pid_path, "r");
	if(NULL != fp){
		if( fgets(buf, 1023, fp)== NULL ){
			fclose(fp);
		}
		fclose(fp);
		outname = (char*)malloc(1024);
		memset(outname,0,1024);
		sscanf(buf, "%*s %s", outname);
	}
	return outname;
}
/*
************************************************************
* 获取配置文件
************************************************************
*/
 char* getConfig(){
	FILE* fd =fopen("/sdcard/MHookDemo/Config.txt","r");
	//判断文件是否打开成功*没有SD的程序不能成功
	if(fd == NULL){
		if(fd == NULL){
			return NULL;
		}
	}
	char* mConfig = (char*)malloc(10240);
	memset(mConfig,0,10240);
	fgets(mConfig,10240,fd);
	fclose(fd);
	size_t mConfigSize = strlen(mConfig);
	if(mConfigSize == 0){
		free(mConfig);
		return NULL;
	}
	return mConfig;
}
/*
************************************************************
*				trim
************************************************************
*/
 void trim(char *s)
{
	int len = strlen(s);
	/* trim trailing whitespace */
	while ( len > 0 && isspace(s[len-1]))
	      s[--len]='\0';
	/* trim leading whitespace */
	memmove(s, &s[strspn(s, " \n\r\t\v")], len);
}
/*
************************************************************
* 获取</proc/self/status> 内容
************************************************************
*/
char* getSelfCmdline(){
	FILE* fd =fopen("/proc/self/status","r");
	if(fd == NULL)return NULL;
	char* mBuf = (char*)malloc(1024);	
	memset(mBuf,0,1024);
	fgets(mBuf,1024,fd);
	trim(mBuf);
	return mBuf;
}
/*
************************************************************
* 获取</proc/self/status> 名字呢看内容
************************************************************
*/
char* getSelfName(){
	FILE* fd =fopen("/proc/self/status","r");
	if(fd == NULL)return NULL;
	char* mBuf = (char*)malloc(1024);	
	memset(mBuf,0,1024);
	fgets(mBuf,1024,fd);
	if((strlen(mBuf) == 0)||(strstr(mBuf,":") == NULL)){
		free(mBuf);
		return NULL;
	}
	char* mOut = strdup(strstr(mBuf,":")+1);
	free(mBuf);
	trim(mOut);
	return mOut;
}
