#ifndef __DexHeader_H_
#define __DexHeader_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
class Dex_Header:DexUtil{
public:
	DexHeader* Header = NULL;
/*
************************************************************
*				print
* 打印头数据
*提供Header地址，打印数据
************************************************************
*/
	static void Log(void* inAddr)
	{
		DEXLOG("Dex_Header@Log \n");
		if(GetBase(inAddr) == NULL){
			DEXLOG("Dex_Header print err!\n");
			return ;
		}else if(GetBase(inAddr) == inAddr){
			DEXLOG("Dex_Header Dex !\n");
		}else{
			DEXLOG("Dex_Header ODex!\n");
		}
		DexHeader* pHeader = (DexHeader*)GetBase(inAddr);
		DEXLOG("Dex Addr            : 0x%08x\n",pHeader);
		DEXLOG("magic               : '%.3s %.4s'\n", pHeader->magic,(char*)(pHeader->magic+4));
		DEXLOG("checksum            : %08x\n", pHeader->checksum);
		DEXLOG("signature           : %02x%02x...%02x%02x\n",
			pHeader->signature[0], pHeader->signature[1],
			pHeader->signature[kSHA1DigestLen-2],
			pHeader->signature[kSHA1DigestLen-1]);
		DEXLOG("file_size           : %d\n", pHeader->fileSize);
		DEXLOG("header_size         : %d\n", pHeader->headerSize);
		DEXLOG("link_size           : %d\n", pHeader->linkSize);
		DEXLOG("link_off            : %d (0x%06x)\n",
			pHeader->linkOff, pHeader->linkOff);
		DEXLOG("mapOff              : %d (0x%06x)\n",
					pHeader->mapOff, pHeader->mapOff);
		DEXLOG("string_ids_size     : %d\n", pHeader->stringIdsSize);
		DEXLOG("string_ids_off      : %d (0x%06x)\n",
			pHeader->stringIdsOff, pHeader->stringIdsOff);
		DEXLOG("type_ids_size       : %d\n", pHeader->typeIdsSize);
		DEXLOG("type_ids_off        : %d (0x%06x)\n",
			pHeader->typeIdsOff, pHeader->typeIdsOff);
		DEXLOG("field_ids_size      : %d\n", pHeader->fieldIdsSize);
		DEXLOG("field_ids_off       : %d (0x%06x)\n",
			pHeader->fieldIdsOff, pHeader->fieldIdsOff);
		DEXLOG("method_ids_size     : %d\n", pHeader->methodIdsSize);
		DEXLOG("method_ids_off      : %d (0x%06x)\n",
			pHeader->methodIdsOff, pHeader->methodIdsOff);
		DEXLOG("class_defs_size     : %d\n", pHeader->classDefsSize);
		DEXLOG("class_defs_off      : %d (0x%06x)\n",
			pHeader->classDefsOff, pHeader->classDefsOff);
		DEXLOG("data_size           : %d\n", pHeader->dataSize);
		DEXLOG("data_off            : %d (0x%06x)\n",
			pHeader->dataOff, pHeader->dataOff);
	}
/*
************************************************************
*				Dex_Header
*解析头文件
*拷贝保存头文件数据
************************************************************
*/
	Dex_Header(DexFile* inDex){
		DEXLOG("[Parse]Dex_Header %08x",inDex->pHeader);
		Header = (DexHeader*)malloc(sizeof(DexHeader));
		memcpy(Header,inDex->pHeader,sizeof(DexHeader));
		Log((void*)inDex->pHeader);
	}
/*
************************************************************
*				IDStoFile
*将头数据添加到内存模块中
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem){
		//判断是否存在 IDSString段，不存在就添加IDSString段
		DEXLOG("Dex_Header IDStoFile");
		str_ModMem* modIds =inMem->getByName("Header");
		if(modIds== NULL){
			modIds = inMem->newNameMem("Header",0x70);
			DEXLOG("Header %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//设置文件大小
		Header->fileSize =inMem->Size();
		Header->dataSize =inMem->Size() - Header->dataOff;
		//拷贝数据到内存
		memcpy(modIds->Addr,Header,0x70);
	}
};
#endif
