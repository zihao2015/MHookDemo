#ifndef __DexClassDef_H_
#define __DexClassDef_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
#include "DexTypes.H"
#include "Dex_Encoded.H"
#include "Dex_Annot.H"
#include "DexClassCode.H"
//__________________________________________________________
typedef struct ClassDefItem{
	struct ClassDefItem* next;
	u4  classIdx;           /* index into typeIds for this class */
	u4  accessFlags;
	u4  superclassIdx;      /* index into typeIds for superclass */
	u4  interfacesOff;      /* file offset to DexTypeList */
	u4  sourceFileIdx;      /* index into stringIds for source file name */
	u4  annotationsOff;     /* file offset to annotations_directory_item */
	u4  classDataOff;       /* file offset to class_data_item */
	u4  staticValuesOff;    /* file offset to DexEncodedArray */
}ClassDefItem;
//Class
class Dex_ClassDef : DexUtil{
public:
	ClassDefItem* Idsitem = NULL;
/*
************************************************************
*				Dex_ClassDef
*解析Class Define
************************************************************
*/
	Dex_ClassDef(DexFile* inDex,Dex_Maps* inmaps){
		//获取基本数据
		DexHeader*	pHeader = (DexHeader*)inDex->pHeader;
		mapsItem*	_mapsItem =inmaps->getTypeItem(kDexTypeClassDefItem);
		DEXLOG("[Parse]Dex_ClassDef %08x %08x",pHeader->classDefsOff,pHeader->classDefsSize);
		//根据DexClassDef的文件结构，设置文件头DexClassDef
		DexClassDef* Item =(DexClassDef*)AddAddr2(pHeader,pHeader->classDefsOff);
		//遍历ClassDefItem
		ClassDefItem* _Idsitem = NULL;
		for(int m_i = 0;m_i < pHeader->classDefsSize;m_i++){			ClassDefItem* _item =(ClassDefItem*)Alloc(sizeof(ClassDefItem));
			_item->classIdx = Item->classIdx;
			_item->accessFlags = Item->accessFlags;
			_item->superclassIdx = Item->superclassIdx;
			_item->interfacesOff = Item->interfacesOff;
			_item->sourceFileIdx = Item->sourceFileIdx;
			_item->annotationsOff = Item->annotationsOff;
			_item->classDataOff = Item->classDataOff;
			_item->staticValuesOff = Item->staticValuesOff;
			//指向下一个ClassDefItem数据
			Item++;
			//将_item添加到列表中，AddToLast非常耗时
			if(_Idsitem == NULL){
				Idsitem = (ClassDefItem*)AddToLast(Idsitem,_item);
				_Idsitem = (ClassDefItem*)GetLastLink(Idsitem);
			}else{
				_Idsitem = (ClassDefItem*)AddLink(_Idsitem,_item);
			}
		}
	}
/*
************************************************************
*				IDStoFile
*将Dex_ClassDef宽度放入Mod_Mem模块中,
*设置数据必须要偏移地址的数据，所以这次只是为了获取内存宽度
*
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem,Dex_Header* inheader,Dex_Maps* inmaps){
		//判断是否存在 IDSString段，不存在就添加IDSString段
		str_ModMem* modIds =inMem->getByName("ClassDef");
		if(modIds== NULL){
			modIds = inMem->newNameMem("ClassDef",GetCount(Idsitem)*0x20);
		}
		DEXLOG("[File]ClassDefine %08x %08x",modIds->Offset,modIds->Length);
		//修改maps偏移地址
		inmaps->getTypeItem(kDexTypeClassDefItem)->offset = modIds->Offset;
		inmaps->getTypeItem(kDexTypeClassDefItem)->size = GetCount(Idsitem);
		inheader->Header->classDefsOff = modIds->Offset;
		inheader->Header->classDefsSize = GetCount(Idsitem);
		 return ;
		//设置Fileds IDS数据
		ClassDefItem* mitem = Idsitem;
		DexClassDef* dexItem =(DexClassDef*)(modIds->Addr);
		while(mitem != NULL){
			dexItem->classIdx = mitem->classIdx;
			dexItem->accessFlags = mitem->accessFlags;
			dexItem->superclassIdx = mitem->superclassIdx;
			dexItem->sourceFileIdx = mitem->sourceFileIdx;
		//	全部需要偏移地址的都不赋值
		//	dexItem->interfacesOff= inParse->types->LoadToFile(mitem->interfacesOff);
		//	dexItem->annotationsOff = inParse->annot->LoadToFile(mitem->annotationsOff);
		//	dexItem->classDataOff = inParse->classCode->LoadToFile(mitem->classDataOff);
		//	dexItem->staticValuesOff = inParse->encoded->LoadToFile(mitem->staticValuesOff);
			dexItem++;
			mitem = mitem->next;
		}
	}
/*
************************************************************
*				DatatoFile
*将数据偏移地址设置为文件加载偏移。
*
************************************************************
*/
	void DatatoFile(Mod_Mem* inMem,Dex_Types* types,Dex_Annot* annot,Dex_Encoded*	encoded,Dex_ClassCode*	classCode){
		//
		str_ModMem* modIds =inMem->getByName("ClassDef");
		if(modIds== NULL){
			DEXLOG("DatatoFile 设置原先没有数据，这里申请数据可能会出错,也不自动修复Header和MAPS");
			modIds = inMem->newNameMem("ClassDef",GetCount(Idsitem)*0x20);
			DEXLOG("ClassDef DatatoFile %08x %08x ",modIds->Offset,modIds->Length);
		}
		//设置Fileds IDS数据
		ClassDefItem* mitem = Idsitem;
		DexClassDef* dexItem =(DexClassDef*)(modIds->Addr);
		while(mitem != NULL){
			dexItem->classIdx = mitem->classIdx;
			dexItem->accessFlags = mitem->accessFlags;
			dexItem->superclassIdx = mitem->superclassIdx;
			dexItem->interfacesOff = types->LoadToFile(mitem->interfacesOff);
			dexItem->sourceFileIdx = mitem->sourceFileIdx;
			dexItem->annotationsOff = annot->LoadToFile(mitem->annotationsOff);
			dexItem->classDataOff = classCode->LoadToFile(mitem->classDataOff);
			dexItem->staticValuesOff = encoded->LoadToFile(mitem->staticValuesOff);
			dexItem++;
			mitem = mitem->next;
		}
	}
};


#endif
