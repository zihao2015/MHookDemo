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
namespace Dex{
//Class Def
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
//Class Code
typedef struct DexClassDataHeader {
	u4 staticFieldsSize;
	u4 instanceFieldsSize;
	u4 directMethodsSize;
	u4 virtualMethodsSize;
} DexClassDataHeader;
/* expanded form of encoded_field */
typedef struct DexField {
	struct DexField* next;
	u4 fieldIdx;    /* index to a field_id_item */
	u4 accessFlags;
} DexField;
/* expanded form of encoded_method */
typedef struct DexMethod {
	struct DexMethod* next;
	u4 methodIdx;    /* index to a method_id_item */
	u4 accessFlags;
	u4 codeOff;      /* file offset to a code_item */
} DexMethod;
/* expanded form of class_data_item. Note: If a particular item is
 * absent (e.g., no static fields), then the corresponding pointer
 * is set to NULL. */
typedef struct DexClassData {
	struct DexClassData* next;
	Base	BS;
	DexClassDataHeader header;
	DexField*	Fields;
	DexMethod* 	Methods;
} DexClassData;
//Code
/*
 * Direct-mapped "try_item".
 */
typedef struct DexCatchItemL {
	DexCatchItemL* 	next;
	u4	Type;
	u4	Byte_Addr;
}DexCatchItemL;
//
typedef struct DexCatchListL{
	DexCatchListL* 	next;
	DexCatchItemL*	Item;
	u4	Size_Handler;
	u4	catch_all;
}DexCatchListL;
//
typedef struct DexTryL{
	DexTryL* 	next;
	u4	StartAddr;
	u2	Insncount;
	u2	Offset_Handler;
}DexTryL;
/*
 * Direct-mapped "code_item".
 *
 * The "catches" table is used when throwing an exception,
 * "debugInfo" is used when displaying an exception stack trace or
 * debugging. An offset of zero indicates that there are no entries.
 */
typedef struct DexCodeL{
	DexCodeL* 	next;
	Base	BS;
	u2	Size_Reg;
	u2	Size_inArg;
	u2	Size_outArg;
	u2	Size_Tries;
	u4	DebugInfo;
	u4	Size_Code;
//
	DexTryL* Try;
	DexCatchListL* Catch;
//
	void* CodeInfo;
	size_t CodeSize;
//
	void* Try_CatchIngfo;
	size_t Try_CatchSize;
	u4	CatchBlocks;
//
}DexCodeL;
//
class Dex_Class : DexUtil{
public:
	ClassDefItem* Idsitem = NULL;
	DexClassData* ClassD = NULL;
	 DexCodeL* Code = NULL;
/**
 *初始化Dex_Class
 *没有实现任何功能
 */
	Dex_Class(){}
/*
************************************************************
*				Dex_ClassDef
*解析Class Define*
************************************************************
*/
	void Dex_ClassDef(DexFile* inDex,Dex_Maps* inmaps){
		//获取基本数据
		DexHeader*	pHeader = (DexHeader*)inDex->pHeader;
		mapsItem*	_mapsItem =inmaps->getTypeItem(kDexTypeClassDefItem);
		DEXLOG("[Parse]Dex_ClassDef %08x %08x",pHeader->classDefsOff,pHeader->classDefsSize);
		//根据DexClassDef的文件结构，设置文件头DexClassDef
		DexClassDef* Item =(DexClassDef*)AddAddr2(pHeader,pHeader->classDefsOff);
		//遍历ClassDefItem
		ClassDefItem* _Idsitem = NULL;
		for(int m_i = 0;m_i < pHeader->classDefsSize;m_i++){
			ClassDefItem* _item =(ClassDefItem*)Alloc(sizeof(ClassDefItem));
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
	void ClassDef_IDStoFile(Mod_Mem* inMem,Dex_Header* inheader,Dex_Maps* inmaps){
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
*				ClassDef_DatatoFile
*将数据偏移地址设置为文件加载偏移。
*
************************************************************
*/
	void ClassDef_DatatoFile(Mod_Mem* inMem,Dex_Types* types,Dex_Annot* annot,Dex_Encoded* encoded){
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
//			dexItem->classDataOff = classCode->LoadToFile(mitem->classDataOff);
			dexItem->staticValuesOff = encoded->LoadToFile(mitem->staticValuesOff);
			dexItem++;
			mitem = mitem->next;
		}
	}
//
/*
************************************************************
*				Dex_ClassCode
*解析ClassCode,采用Maps中读取数据*
************************************************************
*/
	DexClassData* ClassCode_ConvertList(u1* inAddr,DexHeader* in_pheader){
		DexClassData* mList = (DexClassData*)Alloc(sizeof(DexClassData));
		//获取加载偏移
		mList->BS.Offset_Load = (u4)inAddr -(u4)in_pheader;
		inAddr = readLeb128(inAddr,&mList->header.staticFieldsSize);
		inAddr = readLeb128(inAddr,&mList->header.instanceFieldsSize);
		inAddr = readLeb128(inAddr,&mList->header.directMethodsSize);
		inAddr = readLeb128(inAddr,&mList->header.virtualMethodsSize);
		size_t msize =mList->header.staticFieldsSize + mList->header.instanceFieldsSize;
		for(int m_j =0;m_j < msize;m_j++){
			DexField* mField = (DexField*)Alloc(sizeof(DexField));
			inAddr = readLeb128(inAddr,&mField->fieldIdx);
			inAddr = readLeb128(inAddr,&mField->accessFlags);
			mList->Fields =(DexField*)AddToLast(mList->Fields,mField);
		}
		msize =mList->header.directMethodsSize + mList->header.virtualMethodsSize;
		for(int m_j =0;m_j < msize ;m_j++){
			DexMethod* mMethod = (DexMethod*)Alloc(sizeof(DexMethod));
			inAddr = readLeb128(inAddr,&mMethod->methodIdx);
			inAddr = readLeb128(inAddr,&mMethod->accessFlags);
			inAddr = readLeb128(inAddr,&mMethod->codeOff);
			mList->Methods =(DexMethod*)AddToLast(mList->Methods,mMethod);
		}
		mList->BS.MemSize = (u4)inAddr - mList->BS.Offset_Load - (u4)in_pheader;
		return mList;
	}
/*
************************************************************
*				Dex_ClassCode
*解析ClassCode,采用Class_Def中读取数据*
************************************************************
*/
	void Dex_ClassCode(DexFile* inDex){
		//获取基本数据
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		DEXLOG("[Parse]Dex_ClassCode ClassDef");
		int MethodNum = 0;
		ClassDefItem* _DefItem = Idsitem;
		DexClassData* _DataList = NULL;
		while(NULL != _DefItem){
			if(NULL != _DefItem->classDataOff){
				u1* _Addr = (u1*)AddAddr2(pHeader,_DefItem->classDataOff);
				DexClassData* _List = ClassCode_ConvertList(_Addr,pHeader);
				if(_DataList == NULL){
					ClassD = (DexClassData*)AddToLast(ClassD,_List);
					_DataList = (DexClassData*)GetLastLink(ClassD);
				}else{
					_DataList = (DexClassData*)AddLink(_DataList,_List);
				}
				MethodNum++;
			}
			_DefItem = _DefItem->next;
		}/**/
	}
/*
************************************************************
*				Dex_ClassCode
*解析ClassCode,采用Maps中读取数据*
************************************************************
*/
	void Dex_ClassCode(DexFile* inDex,Dex_Maps* inmaps){
		DEXLOG("[Parse]Dex_ClassCode Maps");
		//MethodNum为添加，Code个数
		mapsItem* inMapClassCode =inmaps->getTypeItem(kDexTypeClassDataItem);
		int MethodNum = 0;
		//定位头地址
		u1* mCodeAddr =(u1*)((u4)inDex->pHeader + inMapClassCode->offset);
		//遍历全部的DexClassData
		for(int m_i =0;m_i < inMapClassCode->size;m_i++){
			DEXLOG("Dex_ClassCode %d",m_i);
			DexClassData* mList = (DexClassData*)Alloc(sizeof(DexClassData));
			//获取加载偏移
			mList->BS.Offset_Load = (u4)mCodeAddr -(u4)inDex->pHeader;
			mCodeAddr = readLeb128(mCodeAddr,&mList->header.staticFieldsSize);
			mCodeAddr = readLeb128(mCodeAddr,&mList->header.instanceFieldsSize);
			mCodeAddr = readLeb128(mCodeAddr,&mList->header.directMethodsSize);
			mCodeAddr = readLeb128(mCodeAddr,&mList->header.virtualMethodsSize);
			size_t msize =mList->header.staticFieldsSize + mList->header.instanceFieldsSize;
			for(int m_j =0;m_j < msize;m_j++){
				DexField* mField = (DexField*)Alloc(sizeof(DexField));
				mCodeAddr = readLeb128(mCodeAddr,&mField->fieldIdx);
				mCodeAddr = readLeb128(mCodeAddr,&mField->accessFlags);
				mList->Fields =(DexField*)AddToLast(mList->Fields,mField);
			}
			msize =mList->header.directMethodsSize + mList->header.virtualMethodsSize;
			for(int m_j =0;m_j < msize ;m_j++){
				DexMethod* mMethod = (DexMethod*)Alloc(sizeof(DexMethod));
				mCodeAddr = readLeb128(mCodeAddr,&mMethod->methodIdx);
				mCodeAddr = readLeb128(mCodeAddr,&mMethod->accessFlags);
				mCodeAddr = readLeb128(mCodeAddr,&mMethod->codeOff);
				mList->Methods =(DexMethod*)AddToLast(mList->Methods,mMethod);
				if(mMethod->codeOff != 0){
					Code_Add(inDex,mMethod->codeOff);
					MethodNum++;
				}
			}
			mList->BS.MemSize = (u4)mCodeAddr - mList->BS.Offset_Load - (u4)inDex->pHeader;
			ClassD = (DexClassData*)AddToLast(ClassD,mList);
		}
		DEXLOG("[Parse]Dex_ClassCode MethodNum:%d",MethodNum);
	}
/*
************************************************************
*				DatatoFile
*设置DatatoFile*
************************************************************
*/
	void ClassCode_DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == ClassD){
			DEXLOG("没有存在ClassD数据,直接退出!");
			return ;
		}
		//获取debug模块内存，没有就新建
		DEXLOG("ClassD DatatoFile");
		str_ModMem* modList =inMem->getByName("ClassData");
		if(NULL == modList){
			modList = inMem->newNameMem("Code",AlignAdd(GetSize(ClassD),0x100));
			DEXLOG("ClassD %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//
		inmaps->setTypeOffset(kDexTypeClassDataItem,modList->Offset);
		//
		DexClassData* mList = ClassD;
		u1* mAddr = modList->Addr;
		while(mList != NULL){
			//
			mList->BS.Offset_File = modList->Offset +(u4)mAddr - (u4)modList->Addr;
			mAddr = writeL128(mAddr,mList->header.staticFieldsSize);
			mAddr = writeL128(mAddr,mList->header.instanceFieldsSize);
			mAddr = writeL128(mAddr,mList->header.directMethodsSize);
			mAddr = writeL128(mAddr,mList->header.virtualMethodsSize);
			DexField* mField = mList->Fields;
			while(mField != NULL){
				mAddr = writeL128(mAddr,mField->fieldIdx);
				mAddr = writeL128(mAddr,mField->accessFlags);
				mField = mField->next;
			}
			DexMethod* mMethod = mList->Methods;
			while(mMethod != NULL){
				mAddr = writeL128(mAddr,mMethod->methodIdx);
				mAddr = writeL128(mAddr,mMethod->accessFlags);
				//Load To File
				if(Code != NULL){
					mAddr = writeL128(mAddr,LoadToFile(mMethod->codeOff));
				}else{
					mAddr = writeL128(mAddr,mMethod->codeOff);
				}
				mMethod = mMethod->next;
			}
			mList = mList->next;
		}
	}
/*
************************************************************
*				Dex_Code
* 解析Dex_Code
* 对于Try和Catch分析出长度后直接保存内容，设置文件的是时候直接复制文件
*
*
************************************************************
*/
	void Code_Add(DexFile* inDex,u4 inOffset){
		DexCodeL* mCode = (DexCodeL*)Alloc(sizeof(DexCodeL));
		//设置加载偏移地址 设置文件内存地址
		mCode->BS.SetLoadOffset(inOffset);
		u1* mMem =(u1*)((u4)inDex->pHeader + inOffset);
		u4	mMemStart =(u4)mMem;
		//复制DexCodeL
		mCode->Size_Reg = *(u2*)mMem;mMem += 2;
		mCode->Size_inArg =*(u2*)mMem;mMem += 2;
		mCode->Size_outArg =*(u2*)mMem;mMem += 2;
		mCode->Size_Tries =*(u2*)mMem;mMem += 2;
		mCode->DebugInfo =*(u4*)mMem;mMem += 4;
		mCode->Size_Code =*(u4*)mMem;mMem += 4;
		//获取整个方法代码宽度、然后对齐、定方位代码尾端，查看是否有其他数据
		mCode->CodeSize =(mCode->Size_Code*2+3)&(~3);
		mCode->CodeInfo = (void*)mMem;
		mMem += mCode->CodeSize;
		//判断是否存在Try 和Catch,然后获取异常宽度
		if(mCode->Size_Tries != 0){
			mCode->Try_CatchIngfo = (void*)mMem;
			//解析TryCatch分支	 Number of instructions in try
			for(int m_i = 0;m_i < mCode->Size_Tries;m_i++){
					DexTryL* mTry  =(DexTryL*)Alloc(sizeof(DexTryL));
					mTry->StartAddr =*(u4*)mMem;
					mMem += 4;
					mTry->Insncount =*(u2*)mMem;
					mMem += 2;
					mTry->Offset_Handler =*(u2*)mMem;
					mMem += 2;
					mCode->Try =  (DexTryL*)AddToLast(mCode->Try,mTry);
			}
			// # Number of catch handler blocks: 0x2
			mMem = readLeb128(mMem,&mCode->CatchBlocks);
			for(int m_i = 0;m_i < mCode->CatchBlocks;m_i++){
				DexCatchListL* mList = (DexCatchListL*)Alloc(sizeof(DexCatchListL));
				//Size_Handler 是按Sleb128保存的，当小于0时表示存在catch_all内容
				//catch_all于blocks 一一对应。
				mMem = readSLeb128(mMem,&mList->Size_Handler);
				u4 msize = mList->Size_Handler;
				if(msize >= 0x80000000){
					msize = 0 - msize;
				}
				if(msize == 0){
					mMem = readLeb128(mMem,&mList->catch_all);
				}else{
					for(int m_j = 0;m_j < msize; m_j++){
						DexCatchItemL* _Item = (DexCatchItemL*)Alloc(sizeof(DexCatchItemL));
						mMem = readLeb128(mMem,&_Item->Type);
						mMem = readLeb128(mMem,&_Item->Byte_Addr);
						mList->Item =  (DexCatchItemL*)AddToLast(mList->Item,_Item);
					}
					if(mList->Size_Handler >= 0x80000000){
						mMem = readLeb128(mMem,&mList->catch_all);
					}
				}
				mCode->Catch =  (DexCatchListL*)AddToLast(mCode->Catch,mList);
			}
			//设置内存大小，必须对齐,
			mMem = (u1*)((u4)(mMem+3)&(~3));
			//直接获取保存数据
			mCode->Try_CatchSize =(u4)mMem - (u4)mCode->Try_CatchIngfo;
			DEXLOG("Code  Try_CatchIngfo %08x %08x",mCode->Try_CatchIngfo,mCode->Try_CatchSize);
		}

		mCode->BS.SetMemSize((u4)mMem - mMemStart);
		Code =  (DexCodeL*)AddToLast(Code,mCode);
	}
/*
************************************************************
*				DatatoFile
*设置DatatoFile*
************************************************************
*/
	void Code_DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps,Dex_Debug* debug){
		if(NULL == Code){
			DEXLOG("没有存在Code数据,直接退出!");
			return ;
		}
		//获取Code模块内存，没有就新建
		DEXLOG("Code DatatoFile");
		str_ModMem* modList =inMem->getByName("Code");
		if(NULL == modList){
			modList = inMem->newNameMem("Code",AlignAdd(GetSize(Code),0x1000));
			DEXLOG("Code Addr %08x Length %08x Offset %08x",modList->Addr,modList->Length,modList->Offset);
			DEXLOG("Code  GetCount %08x ",GetCount(Code));
		}
		//设置Code
		inmaps->setTypeOffset(kDexTypeCodeItem,modList->Offset);
		//
		DexCodeL* mCode = Code;
		u1* mMem = modList->Addr;
		int m_i = 0;
		while(NULL != mCode){
			mCode->BS.SetFileOffset(AddAddr2(mMem,SubAddr2(modList->Offset,modList->Addr)));
			//复制数据到内存
			*(u2*)mMem =mCode->Size_Reg;mMem+=2;
			*(u2*)mMem =mCode->Size_inArg;mMem+=2;
			*(u2*)mMem =mCode->Size_outArg;mMem+=2;
			*(u2*)mMem =mCode->Size_Tries;mMem+=2;
			*(u4*)mMem = debug->LoadToFile(mCode->DebugInfo);mMem+=4;
			//可能需要修改
			*(u4*)mMem =mCode->Size_Code;mMem+=4;
			//获取代码宽Size_Code
			memcpy(mMem,mCode->CodeInfo,mCode->CodeSize);
			mMem += mCode->CodeSize;
			//Try_Catch分支，是直接拷贝过来的，因为里面代码于位置无关，
			if(mCode->Size_Tries != 0){
				memcpy(mMem,mCode->Try_CatchIngfo,mCode->Try_CatchSize);
				mMem += mCode->Try_CatchSize;
			}
			mCode = mCode->next;
		}
	}
//
	u4 LoadToFile(u4 inAddr){



		return 0;
	}
};













};
#endif
