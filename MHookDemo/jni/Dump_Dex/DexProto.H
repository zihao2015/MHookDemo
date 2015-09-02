#ifndef __DexProto_H_
#define __DexProto_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
#include "DexTypes.H"
//__________________________________________________________
typedef struct ProtoItem{
	struct ProtoItem* next;
	u4	shortyIdx;          /* index into stringIds for shorty descriptor */
	u4	returnTypeIdx;      /* index into typeIds list for return type */
	u4	parametersOff;      /* file offset to type_list for parameter types */
}ProtoItem;
class Dex_Proto : DexUtil{
public:
	ProtoItem* Idsitem = NULL;
/*
************************************************************
*				Dex_Proto
*解析Dex_Proto
*Dex_Proto需要type_list数据，我们解析时只保存加载数据
*后续将加载数据替换成文件偏移数据就OK了
************************************************************
*/
	Dex_Proto(DexFile* inDex){
		Parse("Dex_Proto");
		//在文件中按结构体很好的保存，设置起始按结构体读取
		DexProtoId* protoItem =(DexProtoId*)((u1*)inDex->pHeader + inDex->pHeader->protoIdsOff);
		for(int m_i = 0;m_i < inDex->pHeader->protoIdsSize;m_i++){
			ProtoItem* _item =(ProtoItem*)Alloc(sizeof(ProtoItem));
			_item->shortyIdx = protoItem->shortyIdx;
			_item->returnTypeIdx = protoItem->returnTypeIdx;
			_item->parametersOff = protoItem->parametersOff;
			protoItem++;/**/
			Idsitem =  (ProtoItem*)AddToLast(Idsitem,_item);
		}
	}
/*
************************************************************
*				getIdsCount
*获取Ids个数，然而空间为getIdsCount()*12
************************************************************
*/
	size_t getIdsCount(){
		ProtoItem* _item = Idsitem;
		size_t mNo = 0;
		while(_item != NULL){
			_item = _item->next;
			mNo++;
		}
		return mNo;
	}
/*
************************************************************
*				IDStoFile
*将ProtoIDS宽度放入Mod_Mem模块中,
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem,Dex_Header* inheader,Dex_Maps* inmaps){
		//判断是否存在 IDSString段，不存在就添加IDSString段
		Parse("Dex_Proto IDStoFile");
		str_ModMem* modIds =inMem->getByName("ProtoIDS");
		if(modIds== NULL){
			modIds = inMem->newNameMem("ProtoIDS",getIdsCount()*12);
			Parse("Proto Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeItem(kDexTypeProtoIdItem)->offset = modIds->Offset;
		inmaps->getTypeItem(kDexTypeProtoIdItem)->size = getIdsCount();
		inheader->Header->protoIdsOff = modIds->Offset;
		inheader->Header->protoIdsSize = getIdsCount();
	}
/*
************************************************************
*				DatatoFile
*主要设置根据Type list 地址设置偏移
************************************************************
*/
	void DatatoFile(Mod_Mem* inMem,Dex_Header* inheader,Dex_Maps* inmaps,Dex_Types* inType){
			//判断是否存在 IDSString段，不存在就添加IDSString段
			Parse("Dex_Proto IDStoFile");
			str_ModMem* modIds =inMem->getByName("ProtoIDS");
			if(modIds== NULL){
				modIds = inMem->newNameMem("ProtoIDS",getIdsCount()*12);
				Parse("Proto Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
			}
			//修改maps偏移地址
			inmaps->getTypeItem(kDexTypeProtoIdItem)->offset = modIds->Offset;
			inmaps->getTypeItem(kDexTypeProtoIdItem)->size = getIdsCount();
			inheader->Header->protoIdsOff = modIds->Offset;
			inheader->Header->protoIdsSize = getIdsCount();
			//设置Proto IDS数据
			ProtoItem* mitem = Idsitem;
			DexProtoId* mID = (DexProtoId*)modIds->Addr;
			while(mitem != NULL){
				mID->shortyIdx = mitem->shortyIdx;
				mID->returnTypeIdx = mitem->returnTypeIdx;
				mID->parametersOff = inType->LoadToFile(mitem->parametersOff);
				mitem = mitem->next;
				mID++;
			}
		}
/*
************************************************************
*				SetMemory
*老函数，新版本不会使用
************************************************************
*/
	void SetMemory(str_ModMem* inMem,Dex_Types* types){
		ProtoItem* mitem = Idsitem;
		DexProtoId* mID = (DexProtoId*)inMem->Addr;
		Parse("[+]Proto offset:0x%08x",inMem->Offset);
		while(mitem != NULL){
			mID->shortyIdx = mitem->shortyIdx;
			mID->returnTypeIdx = mitem->returnTypeIdx;
			mID->parametersOff = types->LoadToFile(mitem->parametersOff);
			mitem = mitem->next;
			mID++;
		}
	}
};
/**/
#endif
