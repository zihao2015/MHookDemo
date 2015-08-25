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
*				getIdsCount
*获取Ids个数，然而空间为getIdsCount()*20
************************************************************
*/
	size_t getIdsCount(){
		ClassDefItem* mitem = Idsitem;
		size_t mNo = 0;
		while(mitem != NULL){
			mitem = mitem->next;
			mNo++;
		}
		return mNo;
	}
/*
************************************************************
*				Dex_ClassDef
*解析Class Define
************************************************************
*/
	Dex_ClassDef(DexFile* inDex){
		Parse("Dex_ClassDef");
		DexClassDef* Item =(DexClassDef*)((u1*)inDex->pHeader + inDex->pHeader->classDefsOff);
		for(int m_i = 0;m_i < inDex->pHeader->classDefsSize;m_i++){			ClassDefItem* _item =(ClassDefItem*)Alloc(sizeof(ClassDefItem));
			_item->classIdx = Item->classIdx;
			_item->accessFlags = Item->accessFlags;
			_item->superclassIdx = Item->superclassIdx;
			_item->interfacesOff = Item->interfacesOff;
			_item->sourceFileIdx = Item->sourceFileIdx;
			_item->annotationsOff = Item->annotationsOff;
			_item->classDataOff = Item->classDataOff;
			_item->staticValuesOff = Item->staticValuesOff;
			Item++;
			Idsitem =(ClassDefItem*) AddToLast(Idsitem,_item);
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
		Parse("ClassDef IDStoFile");
		str_ModMem* modIds =inMem->getByName("ClassDef");
		if(modIds== NULL){
			modIds = inMem->newNameMem("ClassDef",getIdsCount()*0x20);
			Parse("ClassDefine %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeOffset(kDexTypeClassDefItem)->offset = modIds->Offset;
		inmaps->getTypeOffset(kDexTypeClassDefItem)->size = getIdsCount();
		inheader->Header->classDefsOff = modIds->Offset;
		inheader->Header->classDefsSize = getIdsCount();
		 return ;
		//设置Fileds IDS数据
		ClassDefItem* mitem = Idsitem;
		DexClassDef* dexItem =(DexClassDef*)(modIds->Addr);
		while(mitem != NULL){
			dexItem->classIdx = mitem->classIdx;
			dexItem->accessFlags = mitem->accessFlags;
			dexItem->superclassIdx = mitem->superclassIdx;
	//		dexItem->interfacesOff= inParse->types->LoadToFile(mitem->interfacesOff);
			dexItem->sourceFileIdx = mitem->sourceFileIdx;
		//	dexItem->annotationsOff = inParse->annot->LoadToFile(mitem->annotationsOff);
		//	dexItem->classDataOff = inParse->classCode->LoadToFile(mitem->classDataOff);
	//		dexItem->staticValuesOff = inParse->encoded->LoadToFile(mitem->staticValuesOff);
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
			Parse("DatatoFile 设置原先没有数据，这里申请数据可能会出错,也不自动修复Header和MAPS");
			modIds = inMem->newNameMem("ClassDef",getIdsCount()*0x20);
			Parse("ClassDef DatatoFile %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
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
/*
************************************************************
*				SetMemory
*1.��ȡ�б���IDS����
************************************************************
*/
	void SetMemory(str_ModMem* inMem,Dex_Types* types,Dex_Encoded* encoded,Dex_Annot* annot,Dex_ClassCode* classCode){
		Parse("[+]ClassDef Offset:0x%08x",inMem->Offset);
		ClassDefItem* mitem = Idsitem;
		DexClassDef* dexItem =(DexClassDef*)(inMem->Addr);
		while(mitem != NULL){
			dexItem->classIdx = mitem->classIdx;
			dexItem->accessFlags = mitem->accessFlags;
			dexItem->superclassIdx = mitem->superclassIdx;
			dexItem->interfacesOff= types->LoadToFile(mitem->interfacesOff);
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
