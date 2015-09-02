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
*				Dex_Methods
*解析Methods
*
************************************************************
*/
	Dex_Methods(DexFile* inDex,Dex_Maps* inmaps){
		//获取基本数据
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		mapsItem* _mapsItem =inmaps->getTypeItem(kDexTypeMethodIdItem);
		DEXLOG("[Parse]Dex_Methods %08x %08x",pHeader->methodIdsOff,pHeader->methodIdsSize);
		//根据MethodIds的文件结构，设置文件头MethodIds
		DexMethodId* MethodIds =(DexMethodId*)AddAddr2(pHeader,pHeader->methodIdsOff);
		//遍历MethodIds
		MethodsItem* _Idsitem = NULL;
		for(int m_i = 0;m_i < pHeader->methodIdsSize;m_i++){			MethodsItem* _item =(MethodsItem*)Alloc(sizeof(MethodsItem));
			_item->classIdx = MethodIds->classIdx;
			_item->protoIdx = MethodIds->protoIdx;
			_item->nameIdx = MethodIds->nameIdx;
			//指向下一个数据、将数据添加到链表
			MethodIds++;
			//将_item添加到列表中，AddToLast非常耗时
			if(_Idsitem == NULL){
				Idsitem = (MethodsItem*)AddToLast(Idsitem,_item);
				_Idsitem = (MethodsItem*)GetLastLink(Idsitem);
			}else{
				_Idsitem = (MethodsItem*)AddLink(_Idsitem,_item);
			}
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
		DEXLOG("Dex_Methods IDStoFile");
		str_ModMem* modIds =inMem->getByName("MethodsIDS");
		if(modIds== NULL){
			modIds = inMem->newNameMem("MethodsIDS",GetCount(Idsitem)*8);
			DEXLOG("Methods Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeItem(kDexTypeMethodIdItem)->offset = modIds->Offset;
		inmaps->getTypeItem(kDexTypeMethodIdItem)->size = GetCount(Idsitem);
		inheader->Header->methodIdsOff = modIds->Offset;
		inheader->Header->methodIdsSize = GetCount(Idsitem);
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
};


#endif
