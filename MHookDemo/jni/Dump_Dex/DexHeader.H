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
		Parse("Dex_Header@Log \n");
		if(GetBase(inAddr) == NULL){
			Parse("Dex_Header print err!\n");
			return ;
		}else if(GetBase(inAddr) == inAddr){
			Parse("Dex_Header Dex !\n");
		}else{
			Parse("Dex_Header ODex!\n");
		}
		DexHeader* pHeader = (DexHeader*)GetBase(inAddr);
		Parse("Dex Addr            : 0x%08x\n",pHeader);
		Parse("magic               : '%.3s %.4s'\n", pHeader->magic,(char*)(pHeader->magic+4));
		Parse("checksum            : %08x\n", pHeader->checksum);
		Parse("signature           : %02x%02x...%02x%02x\n",
			pHeader->signature[0], pHeader->signature[1],
			pHeader->signature[kSHA1DigestLen-2],
			pHeader->signature[kSHA1DigestLen-1]);
		Parse("file_size           : %d\n", pHeader->fileSize);
		Parse("header_size         : %d\n", pHeader->headerSize);
		Parse("link_size           : %d\n", pHeader->linkSize);
		Parse("link_off            : %d (0x%06x)\n",
			pHeader->linkOff, pHeader->linkOff);
		Parse("mapOff            : %d (0x%06x)\n",
					pHeader->mapOff, pHeader->mapOff);
		Parse("string_ids_size     : %d\n", pHeader->stringIdsSize);
		Parse("string_ids_off      : %d (0x%06x)\n",
			pHeader->stringIdsOff, pHeader->stringIdsOff);
		Parse("type_ids_size       : %d\n", pHeader->typeIdsSize);
		Parse("type_ids_off        : %d (0x%06x)\n",
			pHeader->typeIdsOff, pHeader->typeIdsOff);
		Parse("field_ids_size      : %d\n", pHeader->fieldIdsSize);
		Parse("field_ids_off       : %d (0x%06x)\n",
			pHeader->fieldIdsOff, pHeader->fieldIdsOff);
		Parse("method_ids_size     : %d\n", pHeader->methodIdsSize);
		Parse("method_ids_off      : %d (0x%06x)\n",
			pHeader->methodIdsOff, pHeader->methodIdsOff);
		Parse("class_defs_size     : %d\n", pHeader->classDefsSize);
		Parse("class_defs_off      : %d (0x%06x)\n",
			pHeader->classDefsOff, pHeader->classDefsOff);
		Parse("data_size           : %d\n", pHeader->dataSize);
		Parse("data_off            : %d (0x%06x)\n",
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
		Log((void*)inDex->pHeader);
		Header = (DexHeader*)malloc(0x70);
		memcpy(Header,inDex->pHeader,0x70);
	}
/*
************************************************************
*				IDStoFile
*将头数据添加到内存模块中
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem){
		//判断是否存在 IDSString段，不存在就添加IDSString段
		Parse("Dex_Header IDStoFile");
		str_ModMem* modIds =inMem->getByName("Header");
		if(modIds== NULL){
			modIds = inMem->newNameMem("Header",0x70);
			Parse("Header %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//设置文件大小
		Header->fileSize =inMem->Size();
		Header->dataSize =inMem->Size() - Header->dataOff;
		//拷贝数据到内存
		memcpy(modIds->Addr,Header,0x70);
	}
};
#endif
