#ifndef __DexMethods_H_
#define __DexMethods_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
//__________________________________________________________
typedef struct MethodsItem{
	struct MethodsItem* next;
	u4  classIdx;           /* index into typeIds list for defining class */
	u4  protoIdx;           /* index into protoIds for method prototype */
	u4  nameIdx;            /* index into stringIds for method name */
}MethodsItem;
class Dex_Methods : DexUtil{
public:
	MethodsItem* Idsitem = NULL;
/*
************************************************************
*				getIdsCount
*获取Ids个数，然而空间为getIdsCount()*8
************************************************************
*/
	size_t getIdsCount(){
		MethodsItem* mitem = Idsitem;
		size_t mNo = 0;
		while(mitem != NULL){
			mitem = mitem->next;
			mNo++;
		}
		return mNo;
	}
/*
************************************************************
*				Dex_Methods
*解析Methods
*
************************************************************
*/
	Dex_Methods(DexFile* inDex){
		Parse("Dex_Methods IDStoFile");
		//根据MethodIds的文件结构，设置文件头MethodIds
		DexMethodId* MethodIds =(DexMethodId*)((u1*)inDex->pHeader + inDex->pHeader->methodIdsOff);
		//遍历MethodIds
		for(int m_i = 0;m_i < inDex->pHeader->methodIdsSize;m_i++){			MethodsItem* _item =(MethodsItem*)Alloc(sizeof(MethodsItem));
			_item->classIdx = MethodIds->classIdx;
			_item->protoIdx = MethodIds->protoIdx;
			_item->nameIdx = MethodIds->nameIdx;
			//指向下一个数据、将数据添加到链表
			MethodIds++;
			Idsitem = (MethodsItem*)AddToLast(Idsitem,_item);
		}
	}
/*
************************************************************
*				IDStoFile
*将FiledsIDS宽度放入Mod_Mem模块中,并设置数据
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem,Dex_Header* inheader,Dex_Maps* inmaps){
		//判断是否存在 IDSString段，不存在就添加IDSString段
		Parse("Dex_Methods IDStoFile");
		str_ModMem* modIds =inMem->getByName("MethodsIDS");
		if(modIds== NULL){
			modIds = inMem->newNameMem("MethodsIDS",getIdsCount()*8);
			Parse("Methods Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeItem(kDexTypeMethodIdItem)->offset = modIds->Offset;
		inmaps->getTypeItem(kDexTypeMethodIdItem)->size = getIdsCount();
		inheader->Header->methodIdsOff = modIds->Offset;
		inheader->Header->methodIdsSize = getIdsCount();
		//设置Fileds IDS数据
		MethodsItem* mitem = Idsitem;
		DexMethodId* mID = (DexMethodId*)modIds->Addr;
		//设置数据
		while(mitem != NULL){
			mID->classIdx = mitem->classIdx;
			mID->protoIdx = mitem->protoIdx;
			mID->nameIdx = mitem->nameIdx;
			mID++;
			mitem = mitem->next;
		}
	}
/*
************************************************************
*				SetFileds
*����SetFileds
************************************************************
*/
	void SetMemory(str_ModMem* inMem){
		MethodsItem* mitem = Idsitem;
		DexMethodId* memIDS = (DexMethodId*)inMem->Addr;
		Parse("Method Offset:0x%08x",inMem->Offset);
		while(mitem != NULL){
			memIDS->classIdx = mitem->classIdx;
			memIDS->protoIdx = mitem->protoIdx;
			memIDS->nameIdx = mitem->nameIdx;
			memIDS++;
			mitem = mitem->next;
		}
	}
};


#endif
