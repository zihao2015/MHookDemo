#ifndef __DexFields_H_
#define __DexFields_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
//__________________________________________________________
typedef struct FieldsItem{
	struct FieldsItem* next;
	u4  classIdx;           /* index into typeIds list for defining class */
	u4  typeIdx;            /* index into typeIds for field type */
	u4  nameIdx;            /* index into stringIds for field name */
}FieldsItem;
class Dex_Fileds : DexUtil{
public:
	FieldsItem* Idsitem = NULL;
/*
************************************************************
*				Dex_Fileds
*解析Fileds
************************************************************
*/
	Dex_Fileds(DexFile* inDex,Dex_Maps* inmaps){
		//获取基本数据
		mapsItem*	_mapsItem =inmaps->getTypeItem(kDexTypeFieldIdItem);
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		DEXLOG("[Parse]Dex_Fileds %08x %08x",pHeader->fieldIdsOff,pHeader->fieldIdsSize);
		//按照Fileds保存方式设置头，然后按结构体读取
		DexFieldId* fieldIds =(DexFieldId*)AddAddr2(pHeader,pHeader->fieldIdsOff);
		//遍历FieldsItem
		FieldsItem* _Idsitem = NULL;
		for(int m_i = 0;m_i < pHeader->fieldIdsSize;m_i++){			FieldsItem* _item = (FieldsItem*)Alloc(sizeof(FieldsItem));
			_item->classIdx = fieldIds->classIdx;
			_item->typeIdx = fieldIds->typeIdx;
			_item->nameIdx = fieldIds->nameIdx;
			fieldIds++;
			//将_item添加到列表中，AddToLast非常耗时
			if(_Idsitem == NULL){
				Idsitem = (FieldsItem*)AddToLast(Idsitem,_item);
				_Idsitem = (FieldsItem*)GetLastLink(Idsitem);
			}else{
				_Idsitem = (FieldsItem*)AddLink(_Idsitem,_item);
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
		DEXLOG("Dex_Fileds IDStoFile");
		str_ModMem* modIds =inMem->getByName("FiledsIDS");
		if(modIds== NULL){
			modIds = inMem->newNameMem("FiledsIDS",GetCount(Idsitem)*8);
			DEXLOG("Fileds Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeItem(kDexTypeFieldIdItem)->offset = modIds->Offset;
		inmaps->getTypeItem(kDexTypeFieldIdItem)->size = GetCount(Idsitem);
		inheader->Header->fieldIdsOff = modIds->Offset;
		inheader->Header->fieldIdsSize = GetCount(Idsitem);
		//设置Fileds IDS数据
		FieldsItem* mitem = Idsitem;
		DexFieldId* mID = (DexFieldId*)modIds->Addr;
		while(mitem != NULL){
			mID->classIdx = mitem->classIdx;
			mID->typeIdx = mitem->typeIdx;
			mID->nameIdx = mitem->nameIdx;
			mID++;mitem = mitem->next;
		}
	}
};


#endif
