#ifndef __Dex_Encoded_H_
#define __Dex_Encoded_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" 
#include "DexUtil.H"
//__________________________________________________________
typedef struct Encoded_Item{
	struct Encoded_Item* next;
	u4	type;
	u4	Value;
	u4 	KeyLen;
	u4 	ValueLen;
}Encoded_Item;
typedef struct Encoded_List{
	struct Encoded_List* next;
	Base	BS;
	Encoded_Item* item;
	size_t Itemsize;
}Encoded_List;

//
class Dex_Encoded : DexUtil{
public:
	Encoded_List* CodeList = NULL;
/*
************************************************************
*				getValueLen
*获取值宽度
************************************************************
*/
	static size_t getValueLen(int inType){
		int mTypeClass = inType&kDexAnnotationValueTypeMask;
		int mTypeLen = inType >>kDexAnnotationValueArgShift;
		switch (mTypeClass){
		case kDexAnnotationNull:
		case kDexAnnotationBoolean:
			return 0;
		}
		return mTypeLen+1;
	}
/*
************************************************************
*				LoadToFile
*1.老函数，将加载内存替换成现在保存的文件偏移内存
************************************************************
*/
	size_t LoadToFile(size_t inLoad){
		if(inLoad == NULL)return NULL;
		Encoded_List* mList = CodeList;
		while(mList != NULL){
			if(mList->BS.Offset_Load == inLoad){
				return mList->BS.Offset_File;
			}
			mList = mList->next;
		}
		return inLoad;
	}
/*
************************************************************
*				Dex_Encoded
*解析Encoded
*	1.Size -> Leb128
*	2.Type -> Leb128
*	3.Value -> Byte | Short | 3Byte | Long(No Leb128)
*
************************************************************
*/
	Dex_Encoded(DexFile* inDex,Dex_Maps* inmaps){
		//获取基本数据
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		mapsItem* _mapsItem =inmaps->getTypeItem(kDexTypeEncodedArrayItem);
		DEXLOG("[Parse]Dex_Encoded %08x %08x",_mapsItem->offset,_mapsItem->size);
		//设置起始地址
		u1* encodedIds =(u1*)((u1*)inDex->pHeader + _mapsItem->offset);
		//遍历全部Encoded_List
		Encoded_List* _CodeList = NULL;
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			Encoded_List* mlist = (Encoded_List*)Alloc(sizeof(Encoded_List));
			mlist->BS.Offset_Load =(u4)encodedIds - (u4)inDex->pHeader;
			mlist->Itemsize = readLeb128(encodedIds);
			size_t mOffset = (size_t)encodedIds;
 			encodedIds = NEXTLeb128(encodedIds);
			//遍历全部Encoded_Item
			for(int m_j = 0;m_j < mlist->Itemsize;m_j++){
				Encoded_Item* mitem = (Encoded_Item*)Alloc(sizeof(Encoded_Item));
				mitem->type =readLeb128(encodedIds);
				mitem->KeyLen = readLimitLeb128(encodedIds);
				encodedIds = NEXTLeb128(encodedIds);
				mitem->ValueLen = getValueLen(mitem->type);
				switch(mitem->ValueLen){
					case 4:
						mitem->Value+= *(encodedIds+3)<<24;
					case 3:
						mitem->Value += *(encodedIds+2)<<16;
					case 2:
						mitem->Value += *(encodedIds+1)<<8;
					case 1:
						mitem->Value += *encodedIds&0xFF;
				}
				encodedIds += mitem->ValueLen;
				mlist->item =(Encoded_Item*)AddToLast(mlist->item,mitem);
			}
			mOffset = (size_t)encodedIds - mOffset;
			mlist->BS.MemSize = mOffset;
			//将_item添加到列表中，AddToLast非常耗时
			if(_CodeList == NULL){
				CodeList = (Encoded_List*)AddToLast(CodeList,mlist);
				_CodeList = (Encoded_List*)GetLastLink(CodeList);
			}else{
				_CodeList = (Encoded_List*)AddLink(_CodeList,mlist);
			}
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
		DEXLOG("[+]Dex_Encoded DatatoFile");
		if(NULL == CodeList){
			DEXLOG("没有储存数据!");
			return ;
		}
		str_ModMem* modList =inMem->getByName("EncodedArray");
		if(NULL == modList){
			modList = inMem->newNameMem("EncodedArray",AlignAdd(GetSize(CodeList),0x04));
			DEXLOG("EncodedArray %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeItem(kDexTypeEncodedArrayItem)->offset = modList->Offset;
		//
		u1* mMem = modList->Addr;
		Encoded_List* mlist = CodeList;
		//遍历父项
		while(NULL != mlist){
			//设置加载地址
			mlist->BS.Offset_File =AddAddr2(modList->Offset,SubAddr2(mMem,modList->Addr));
			Encoded_Item* mitem = mlist->item;
			mMem = writeL128(mMem,mlist->Itemsize);
			//遍历子项
			while(mitem != NULL){
				mMem = writeL128(mMem,mitem->type);
				switch(mitem->ValueLen){
				case 4:
					*(mMem+3) = (mitem->Value>>24) &0xFF;
				case 3:
					*(mMem+2) = (mitem->Value>>16) &0xFF;
				case 2:
					*(mMem+1) = (mitem->Value>>8) &0xFF;
				case 1:
					*mMem = mitem->Value &0xFF;
				}
				mMem += mitem->ValueLen;
				mitem = mitem->next;
			}
			mlist = mlist->next;
		}
	}
};
#endif
