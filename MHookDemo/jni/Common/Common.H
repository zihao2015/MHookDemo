#ifndef __COMMON_H_
#define __COMMON_H_
//声明函数供外部调用
extern char* getpidName(pid_t inpid);
extern char* getConfig();
extern char* getSelfName();
extern char* getSelfCmdline();
#endif
