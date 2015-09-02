#ifndef __DexParse_H_
#define __DexParse_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
//
#include "DexUtil.H"
#include "DexHeader.H"
#include "DexString.H"
#include "DexTypes.H"
#include "DexProto.H"
#include "DexFields.H"
#include "DexMethods.H"
#include "DexMaps.H"
#include "DexClassDef.H"
#include "Dex_Encoded.H"
#include "Dex_Annot.H"
#include "DexClassCode.H"
#include "DexCode.H"
#include "DexDebug.H"
/*
 *						解析Dex文件格式
 */
class DexParse : DexUtil{
public:
	DexFile* dex = NULL;
	Mod_Mem *mem = NULL;
//IDS段
	Dex_Header* 	header = NULL;
	Dex_String* 	string = NULL;
	Dex_Types*  	types = NULL;
	Dex_Proto*  	proto = NULL;
	Dex_Fileds*		fileds = NULL;
	Dex_Methods* 	methods = NULL;
	Dex_ClassDef* 	classdef = NULL;
	Dex_Maps*		maps = NULL;
//可能不会存在段
	Dex_Encoded*	encoded = NULL;
	Dex_Annot* 		annot = NULL;
	Dex_Debug*		debug = NULL;
//最后融合段
	Dex_Code*		code = NULL;
	Dex_ClassCode*	classCode = NULL;

//__________________________________________________________
/*
************************************************************
*				DumpToFile
* Dump出文件*
* 内存数据排放问题
*	使用map item type codes的大小顺序开始排放
*	设置对齐方便数据查看 mMem = mem->newMem(Align(mem->Szie(),0x100));
*先解码DexFile
************************************************************
*/
	void DumpToFile(char* appName,char* inFile){
		mem = new Mod_Mem();
		Parse("[+]IDS DexParse");
		//header、string_ids、types_ids、
		header->IDStoFile(mem);
		string->IDStoFile(mem,header,maps);
		types->IDStoFile(mem,header,maps);
		proto->IDStoFile(mem,header,maps);
		fileds->IDStoFile(mem,header,maps);
		methods->IDStoFile(mem,header,maps);
		classdef->IDStoFile(mem,header,maps);
	//以下便是Code数据了,设置数据开始
		header->Header->dataOff= mem->Size();
	//解析IDS对应的数据(code)
		//TypeList
		types->DatatoFile(mem,maps);
		//proto(需要TypeList偏移地址)
		proto->DatatoFile(mem,header,maps,types);
		annot->AnnotToData(mem,maps);
		annot->AnnotSetToData(mem,maps);
		annot->AnnotSRLToData(mem,maps);
		annot->AnnotDirToData(mem,maps);
	//encoded array
		if(NULL != encoded){
			encoded->DatatoFile(mem,maps);
		}
	//Debug
		if(NULL != debug){
			debug->DatatoFile(mem,maps);
		}
	//Code
		code->DatatoFile(mem,maps,debug);
		classCode->DatatoFile(mem,maps,code);
		string->DatatoFile(mem,maps);
		classdef->DatatoFile(mem,types,annot,encoded,classCode);
	//根据DUMP数据正在修改MAPS和Header
		maps->IDStoFile(mem,header);
		Parse("[+]header");
		header->IDStoFile(mem);
	//Save To File
		Parse("[+]Save");
		mem->SaveFile(appName,inFile);
	}
/*
************************************************************
*				DexParse
* 解析DexFile
* 解析每一个Item，然后在DumpToFile中实现组装
* 将解析和组装分开做
* 	1.为了2代以上加固
* 	2.为了调试添加数据
*	3.
************************************************************
*/
 	DexParse(void* inAddr,DexFile* inDex){
 		// 初始化dex
		dex = inDex;
		//Header、maps
		header = new Dex_Header(inDex);
		maps = new Dex_Maps(inDex);
		//string、types、proto、fileds、methods、Class_Def
		string = new Dex_String(inDex);
		types = new Dex_Types(inDex,maps);
		proto = new Dex_Proto(inDex);
		fileds = new Dex_Fileds(inDex);
		methods = new Dex_Methods(inDex);
		classdef = new Dex_ClassDef(inDex);
		//将Annot分开处理、使用Annot类进行解析
		//Annot中包含kDexTypeAnnotationSetRefList、kDexTypeAnnotationSetItem、kDexTypeAnnotationItem、kDexTypeAnnotationsDirectoryItem
		annot = new Dex_Annot(inDex,maps);
		//下面首先要判断是否存在，然后再去初始化
		//Encoded、Debug、Dex_Code
		if(maps->getTypeItem(kDexTypeEncodedArrayItem) != NULL){
			encoded = new Dex_Encoded(inDex,maps);
		}
		if(maps->getTypeItem(kDexTypeDebugInfoItem) != NULL){
			debug = new Dex_Debug(inDex,maps);
		}
		//解析ClassData  然后根据内容解析Dex_Code
		code = new Dex_Code(inDex);
		classCode = new Dex_ClassCode(inDex,maps,code);
		Parse("Parse Over");
	}
};


#endif
