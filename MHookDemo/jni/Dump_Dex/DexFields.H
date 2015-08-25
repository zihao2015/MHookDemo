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
*				getIdsCount
*获取Ids个数，然而空间为getIdsCount()*8
************************************************************
*/
	size_t getIdsCount(){
		FieldsItem* mitem = Idsitem;
		size_t mNo = 0;
		while(mitem != NULL){
			mitem = mitem->next;
			mNo++;
		}
		return mNo;
	}
/*
************************************************************
*				Dex_Fileds
*解析Fileds
*
************************************************************
*/
	Dex_Fileds(DexFile* inDex){
		Parse("[+]Dex_Fileds");
		//按照Fileds保存方式设置头，然后按结构体读取
		DexFieldId* fieldIds =(DexFieldId*)((u1*)inDex->pHeader + inDex->pHeader->fieldIdsOff);
		for(int m_i = 0;m_i < inDex->pHeader->fieldIdsSize;m_i++){			FieldsItem* _item = (FieldsItem*)Alloc(sizeof(FieldsItem));
			_item->classIdx = fieldIds->classIdx;
			_item->typeIdx = fieldIds->typeIdx;
			_item->nameIdx = fieldIds->nameIdx;
			fieldIds++;
			Idsitem = (FieldsItem*)AddToLast(Idsitem,_item);
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
		Parse("Dex_Fileds IDStoFile");
		str_ModMem* modIds =inMem->getByName("FiledsIDS");
		if(modIds== NULL){
			modIds = inMem->newNameMem("FiledsIDS",getIdsCount()*8);
			Parse("Fileds Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeOffset(kDexTypeFieldIdItem)->offset = modIds->Offset;
		inmaps->getTypeOffset(kDexTypeFieldIdItem)->size = getIdsCount();
		inheader->Header->fieldIdsOff = modIds->Offset;
		inheader->Header->fieldIdsSize = getIdsCount();
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
/*
************************************************************
*				SetMemory
*老函数，新版本不会使用
************************************************************
*/
	void SetMemory(str_ModMem* inMem){
		FieldsItem* mitem = Idsitem;
		DexFieldId* memIDS = (DexFieldId*)inMem->Addr;
		Parse("Fileds Offset:0x%08x",inMem->Offset);
		while(mitem != NULL){
			memIDS->classIdx = mitem->classIdx;
			memIDS->typeIdx = mitem->typeIdx;
			memIDS->nameIdx = mitem->nameIdx;
			memIDS++;
			mitem = mitem->next;
		}
	}
};


#endif
