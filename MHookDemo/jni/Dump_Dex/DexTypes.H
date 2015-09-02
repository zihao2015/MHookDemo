#ifndef __DexTypes_H_
#define __DexTypes_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
#include "DexMaps.H"
//__________________________________________________________
//存放typeItem
typedef struct typeItem{
	struct typeItem* next;
	u4	 type;
}typeItem;
//存放typeList
//位置空间在Code中
//需要保存加载地址等信息，所以加载Base段
typedef struct typeList{
	struct typeList* next;
	Base	BS;
	struct typeItem* item;
	u4		Size;
}typeList;
class Dex_Types : DexUtil{
public:
	typeItem* Idsitem = NULL;
	typeList* Codelist = NULL;
/*
************************************************************
*				LoadToFile
*1.老函数，将加载内存替换成现在保存的文件偏移内存
************************************************************
*/
	size_t LoadToFile(size_t inLoad){
		if(inLoad == 0)return 0;
		typeList* mList = Codelist;
		while(mList != NULL){
			if(inLoad == mList->BS.Offset_Load){
				return mList->BS.Offset_File;
			}
			mList = mList->next;
		}
		return inLoad;
	}
/*
************************************************************
*				getListCodeLen
*获取需要多少的Code空间*
************************************************************
*/
	size_t getCodeLen(){
		size_t mlen = 0;
		typeList* _list = Codelist;
		while(_list){
			mlen += _list->BS.MemSize;
			_list = _list->next;
		}
		return mlen;
	}
/*
************************************************************
*				getIdsCount
*获取Ids个数，然而空间为getIdsCount()*4
************************************************************
*/
	size_t getIdsCount(){
		typeItem* mitem = Idsitem;
		size_t mNo = 0;
		while(mitem != NULL){
			mitem = mitem->next;
			mNo++;
		}
		return mNo;
	}
/*
************************************************************
*				Dex_Types
*解析TypeItem和TypeList
*因为typeList在内存中，所以需要借助Base模块帮助其加载
*由于typeIds不存在分开存放，所以一次解析完，而且不需要保存偏移表加载地址、
*存放时会只需要将数据关联即可
*注意:Code长度可能不是Code说明长度，我们分析String 到\0结束
************************************************************
*/
	Dex_Types(DexFile* inDex,Dex_Maps* inmaps){
		Parse("Dex_Types Item");
		//获取typeIds实际地址
		u4* mMemIds =(u4*)AddAddr2(inDex->pHeader,inDex->pHeader->typeIdsOff);
		//遍历typeIds
		typeItem* _idsitem = NULL;
		for(int m_i = 0;m_i < inDex->pHeader->typeIdsSize;m_i++){			typeItem* _item = (typeItem*)Alloc(sizeof(typeItem));
			_item->type = *mMemIds++;
			if(_idsitem == NULL){
				Idsitem = (typeList*)AddToLast(Idsitem,_item);
				_idsitem = (typeList*)GetLastLink(Idsitem);
			}else{
				_idsitem = (typeList*)AddLink(_idsitem,_item);
			}
		}
		//查找Maps中是否存在TypeList
		Parse("Dex_Types List");
		mapsItem* _mapsItem = inmaps->getTypeItem(kDexTypeTypeList);
		Codelist = NULL;
		if(NULL == _mapsItem){
			Parse("Dex_Types List不存在");
			return;
		}
		//存在Type list，对Type list进行遍历
		u4* mMemList =(u4*)AddAddr2(inDex->pHeader, _mapsItem->offset);
		u4	mMemStart = (u4)mMemList;
		//遍历Type list
		typeList* _Codelist = NULL;
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){			typeList* _List =(typeList*)Alloc(sizeof(typeList));
			//设置偏移地址
			_List->BS.Offset_Load = SubAddr2(mMemList,inDex->pHeader);
			_List->Size = *mMemList;
			mMemList++;
			//遍历type Item,小型链接就不在分开处理
			for(int m_j = 0;m_j < _List->Size;m_j++){
				typeItem* _Item = (typeItem*)Alloc(sizeof(typeItem));
				_Item->type = *(u2*)mMemList;
				mMemList = (u4*)AddAddr2(mMemList,2);
				_List->item =(typeItem*)AddToLast(_List->item,_Item);
			}
			//设置内存大小
			mMemList = (u4*)AlignAdd((u4)mMemList,0x04);
			_List->BS.MemSize =SubAddr2(mMemList,mMemStart);
			mMemStart =(u4)mMemList;
			if(_Codelist == NULL){
				Codelist = (typeList*)AddToLast(Codelist,_List);
				_Codelist = (typeList*)GetLastLink(Codelist);
			}else{
				_Codelist = (typeList*)AddLink(_Codelist,_List);
			}
		}
	}
/*
************************************************************
*				IDStoFile
*将TydeIDS宽度放入Mod_Mem模块中,
*会实现里面数据存放(与StringIDS不同，因为typeIds中不存在于位置有关数据)
*但是List内容需要的DatatoFile中实现
*关联也在DatatoFile中实现
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem,Dex_Header* inheader,Dex_Maps* inmaps){
		//判断是否存在 IDSString段，不存在就添加IDSString段
		Parse("Dex_Types IDStoFile");
		str_ModMem* modIds =inMem->getByName("TypeIDS");
		if(modIds == NULL){
			modIds = inMem->newNameMem("TypeIDS",getIdsCount()*4);
			Parse("Type Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeItem(kDexTypeTypeIdItem)->offset = modIds->Offset;
		inmaps->getTypeItem(kDexTypeTypeIdItem)->size = getIdsCount();
		inheader->Header->typeIdsOff = modIds->Offset;
		inheader->Header->typeIdsSize = getIdsCount();
		//设置Type IDS数据
		u4* memIds = (u4*)modIds->Addr;
		typeItem* _item = Idsitem;
		while(_item != NULL){
			*memIds = _item->type;
			_item = _item->next;
			memIds++;
		}
	}
/*
************************************************************
*				DatatoFile
*将StringCode按文件保存形式保存,这里应该在之前就保存好了StringIDS模块
*然后关联StringIDS和StringCode关联信息
************************************************************
*/
	void DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps){
		Parse("Dex_Types DatatoFile");
		//判断是否存在 IDSString段，不存在就添加IDSString段
		str_ModMem* modList =inMem->getByName("TypeList");
		if(NULL == modList){
			modList = inMem->newNameMem("TypeList",AlignAdd(getCodeLen(),0x04));
			Parse("TypeList %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//获取List数据
		str_ModMem* memCode = inMem->getByName("TypeList");
		//修改maps偏移地址
		inmaps->getTypeItem(kDexTypeTypeList)->offset = memCode->Offset;
		//设置数据
		typeList* mlist = Codelist;
		u4* memList = (u4*)modList->Addr;
		u4 memStart= (u4)memList;
		//写全部List数据
		while(mlist != NULL){
			mlist->BS.Offset_File = modList->Offset+(u4)memList-memStart;
			*memList = mlist->Size;
			memList++;
			u2* memItem = (u2*)memList;
			typeItem* mitem = mlist->item;
			while(mitem != NULL){
				*memItem = mitem->type;
				mitem = mitem->next;
				memItem++;
			}
			memItem = Ali(memItem,0x04);
			memList = (u4*)memItem;
			mlist = mlist->next;/**/
		}
	}
/*
************************************************************
*				SettypeList
*老函数 新版不会使用
************************************************************
*/
	void SettypeList(str_ModMem* inMem){
		typeList* mlist = Codelist;
		Parse("type List Offset:0x%08x",inMem->Offset);
		u4* mSIZE= (u4*)inMem->Addr;
		size_t Off_File = inMem->Offset;
		while(mlist != NULL){
			mlist->BS.Offset_File = Off_File;
			*mSIZE = mlist->Size;
			mSIZE++;
			u2* memItem = (u2*)mSIZE;
			typeItem* mitem = mlist->item;
			while(mitem != NULL){
				*memItem = mitem->type;
				mitem = mitem->next;
				memItem++;
			}
			//设置内存对齐
			memItem = Ali(memItem,0x04);
			mSIZE = (u4*)memItem;
			Off_File = (u4)mSIZE - (u4)inMem->Addr + inMem->Offset;
			mlist = mlist->next;/**/
		}
	}
};


#endif

