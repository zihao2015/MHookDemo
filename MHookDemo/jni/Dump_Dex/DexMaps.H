#ifndef __DexMaps_H_
#define __DexMaps_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
//__________________________________________________________
typedef struct mapsItem{
	struct mapsItem* next;
	u4	type;       		 /*Section type*/
 	u4	unused;				/*unused*/
	u4	size;        		/* section size*/
	u4	offset;     		/* section offset */
}mapsItem;
//
typedef struct maplist{
	struct mapsItem* item;
	size_t mapSize;
}maplist;
/*
************************************************************
*				Dex_Maps*
************************************************************
*/
class Dex_Maps : DexUtil{
public:
	maplist* Map = NULL;
/*
************************************************************
*
*
************************************************************
*/
	void LogMaps(){

	}
/*
************************************************************
*				getIdsCount
*获取Ids个数
************************************************************
*/
	size_t getIdsCount(){
		mapsItem* mitem = Map->item;
		size_t mNo = 0;
		while(mitem != NULL){
			mitem = mitem->next;
			mNo++;
		}
		return mNo;
	}
/*
************************************************************
*				Dex_Maps
*解析Maps
************************************************************
*/
	Dex_Maps(DexFile* inDex){
		Parse("Dex_Maps");
		//获取Map个数
		Map = (maplist*)Alloc(sizeof(maplist));
		Map->mapSize = *((u4*)((u1*)inDex->pHeader + inDex->pHeader->mapOff));
		DexMapId* MapsIds =(DexMapId*)((u1*)inDex->pHeader + inDex->pHeader->mapOff+4);
		//遍历Map内容
		for(int m_i = 0;m_i < Map->mapSize;m_i++){
			mapsItem* _item = (mapsItem*)Alloc(sizeof(mapsItem));
			_item->type = MapsIds->type;
			_item->unused = MapsIds->unused;
			_item->size = MapsIds->size;
			_item->offset = MapsIds->offset;
			MapsIds++;
			Map->item = (mapsItem*)AddToLast(Map->item,_item);
		}
	}
/*
************************************************************
*				setMaps
************************************************************
*/
	void setMaps(str_ModMem* mMem){
		*((u4*)mMem->Addr) =Map->mapSize;
		mapsItem* mitem = Map->item;
		Parse("[+]Maps Offset:0x%08x",mMem->Offset);
		DexMapId* mapIds =(DexMapId*)(mMem->Addr+4);
		while(mitem != NULL){
			mapIds->type = mitem->type;
			mapIds->unused = mitem->unused;
			mapIds->size = mitem->size;
			mapIds->offset = mitem->offset;
			mapIds++;
			mitem = mitem->next;
		}
	}
/*
************************************************************
*				getTypeOffset
*获取子模块结构体
************************************************************
*/
	mapsItem* getTypeOffset(u2 in_type){
		mapsItem* mItem = Map->item;
		while(mItem != NULL){
			if(mItem->type == in_type){
				return mItem;
			}
			mItem = mItem->next;
		}
		return NULL;
	}
/*
************************************************************
*				setTypeOffset
*设置子模块偏移地址
************************************************************
*/
	void setTypeOffset(u2 in_type,u4 inoffset){
		mapsItem* mItem = Map->item;
		while(mItem != NULL){
			if(mItem->type == in_type){
				mItem->offset = inoffset;
				return ;
			}
			mItem = mItem->next;
		}
	}
/*
************************************************************
*				IDStoFile
*这里只是获取内存宽度，不设置数据
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem,	Dex_Header* inheader){
		//判断是否存在 IDSString段，不存在就添加IDSString段
		Parse("Dex_Maps IDStoFile");
		str_ModMem* modIds =inMem->getByName("MapsIDS");

		if(modIds == NULL){
			modIds = inMem->newNameMem("MapsIDS",getIdsCount()*12+4);
			Parse("Dex_Maps DatatoFile %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//设置头系统文件中的数据
		getTypeOffset(kDexTypeMapList)->offset = modIds->Offset;
		getTypeOffset(kDexTypeMapList)->size = getIdsCount();
		inheader->Header->mapOff =modIds->Offset;
		//
		*((u4*)modIds->Addr) =Map->mapSize;
		mapsItem* mitem = Map->item;
		DexMapId* mapIds =(DexMapId*)(modIds->Addr+4);
		while(mitem != NULL){
			mapIds->type = mitem->type;
			mapIds->unused = mitem->unused;
			mapIds->size = mitem->size;
			mapIds->offset = mitem->offset;
			mapIds++;
			mitem = mitem->next;
		}
	}
};
#endif
