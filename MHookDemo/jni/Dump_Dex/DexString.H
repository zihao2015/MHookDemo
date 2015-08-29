#ifndef __DexString_H_
#define __DexString_H_
#include <stdio.h>
#include <unistd.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Module_Mem.H" #include "DexMaps.H"
#include "DexUtil.H"
//__________________________________________________________
//字符串保存列表
typedef struct stringItem{
	struct stringItem* next;
	//
	char* 	str;
	u4		L128Len;
	u4		L128Code;
	u4		strLen;
	u4	 	size;
}stringItem;
class Dex_String : DexUtil{
public:
	stringItem* Idsitem = NULL;
/*
************************************************************
*				getCodeLen
*1.获取String Data 数据宽度
************************************************************
*/
	size_t getCodeLen(){
		size_t mSzie = NULL;
		stringItem* mitem = Idsitem;
		while(mitem != NULL){
			mSzie += mitem->size;
			mitem = mitem->next;
		}
		Parse("Dex_String@getCodeLen:0x%08x",mSzie);
		return mSzie;
	}
/*
************************************************************
*				Dex_String
*解析StringIDS和StringCode
*将数据保存在Idsitem中(Code也是)
*由于StringIDS不存在分开存放，所以一次解析完，而且不需要保存偏移表加载地址、
*存放时会只需要将数据关联即可
*注意:Code长度可能不是Code说明长度，我们分析String 到\0结束
************************************************************
*/
	Dex_String(DexFile* inDex){
		Parse("Dex_String");
		//获取IDS实际内存地址
		u4* mMemIds =(u4*)((u4)inDex->pHeader + inDex->pHeader->stringIdsOff);
		//遍历字符串
		for(int m_i = 0;m_i < inDex->pHeader->stringIdsSize;m_i++){			stringItem* _item =(stringItem*)Alloc(sizeof(stringItem));
			//获取字符串Code 起始地址
			u1* mMemCode =(u1*)((u4)inDex->pHeader + *mMemIds);
			_item->L128Code= readLeb128(mMemCode);
			_item->L128Len = readLimitLeb128(mMemCode);
			mMemCode = NEXTLeb128(mMemCode);
			_item->strLen= strlen((char*)mMemCode);
			//将长度最大的设置为字符串长度
			if(_item->L128Code > _item->strLen){
				_item->strLen = _item->L128Code;
			}
//			Parse("Dex_String %08x",_item->strLen);
			//不申请内存存放字符串，直接指向DexFile文件内存
//			_item->str = (char*)Alloc(_item->strLen+1);
//			memcpy(_item->str,mMemCode,_item->strLen);
			_item->str = (char*)mMemCode;
			_item->size = _item->strLen + _item->L128Len + 1;
			//将字符串添加到列表中
			Idsitem = (stringItem*)AddToLast(Idsitem,_item);
			//指向下一个字符串
			mMemIds++;
		}
	}
/*
************************************************************
*				getIdsCount
*获取StringIds个数，
*获取个数后4*getIdsCount()就是保存Ids需要的空间
************************************************************
*/
	size_t getIdsCount(){
		stringItem* mitem = Idsitem;
		size_t mNo = 0;
		while(mitem != NULL){
			mitem = mitem->next;
			mNo++;
		}
		return mNo;
	}
/*
************************************************************
*				SetMemory
*1.将数据输出成文件
*2.老函数，新版不会使用
************************************************************
*/
	void SetMemory(str_ModMem* inIdS,str_ModMem* inCode){
		stringItem* mitem = Idsitem;
		u4* idsAddr = (u4*)inIdS->Addr;
		unsigned char* idsCode = (unsigned char*)inCode->Addr;
		Parse("SetMemory@inCode->Offset:0x%08X,idsCode:0x%08x",inCode->Offset,idsCode);
		Parse("[+]String IDS offset:0x%08x,Code offset:0x%08x",inIdS->Offset,inCode->Offset);
		size_t offset = 0;
		//..
		while(mitem != NULL){
			*idsAddr = inCode->Offset + offset;
			writeL128(idsCode+offset,mitem->L128Code);
			memcpy(idsCode + offset +mitem->L128Len,mitem->str,mitem->strLen);
			offset += mitem->size;
			/*Parse("SetMemory@offset :0x%08x",offset);/**/
			idsAddr++;
			mitem = mitem->next;
		}
	}
/*
************************************************************
*				IDStoFile
*将字符串IDS宽度设置字符串，
*但是字符串内容需要的DatatoFile中实现
*关联也在DatatoFile中实现
************************************************************
*/
	void IDStoFile(Mod_Mem* inMem,Dex_Header* inheader,Dex_Maps* inmaps){
		Parse("Dex_String IDStoFile");
		//判断是否存在 IDSString段，不存在就添加IDSString段
		str_ModMem* modIds =inMem->getByName("StringIDS");
		if(modIds== NULL){
			modIds = inMem->newNameMem("StringIDS",getIdsCount()*4);
			Parse("String Ids %08x %08x %08x",modIds->Addr,modIds->Length,modIds->Offset);
		}
		//修改maps偏移地址
		inmaps->getTypeOffset(kDexTypeStringIdItem)->offset = modIds->Offset;
		inmaps->getTypeOffset(kDexTypeStringIdItem)->size = getIdsCount();
		inheader->Header->stringIdsOff = modIds->Offset;
		inheader->Header->stringIdsSize = getIdsCount();
	}
/*
************************************************************
*				DatatoFile
*将StringCode按文件保存形式保存,这里应该在之前就保存好了StringIDS模块
*然后关联StringIDS和StringCode关联信息
************************************************************
*/
	void DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps){
		Parse("Dex_String DatatoFile");
		//判断是否存在 IDSString段，不存在就添加IDSString段
		str_ModMem* modList = inMem->getByName("StringCode");
		if(NULL == modList){
			modList = inMem->newNameMem("StringCode",AlignAdd(getCodeLen(),0x100));
			Parse("Dex_String DatatoFile %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//获取需要关联的两个str_ModMem段(StringIDS和StringCode)
		str_ModMem* memIds = inMem->getByName("StringIDS");
		str_ModMem* memCode = inMem->getByName("StringCode");
		//修改maps偏移地址
		inmaps->setTypeOffset(kDexTypeStringDataItem,memCode->Offset);
		//填充数据
		stringItem* mitem = Idsitem;
		u4* idsAddr =(u4*)memIds->Addr;
		u1* CodeAddr = (u1*)memCode->Addr;
		size_t offset = 0;
		while(mitem != NULL){
			//设置IDS数据指向Code偏移内存
			*idsAddr = (u4)CodeAddr +offset- (u4)memCode->Addr+memCode->Offset;
			idsAddr++;
			//将数据写到Code偏移内存
			writeL128(CodeAddr+offset,mitem->L128Code);
			memcpy(CodeAddr + offset + mitem->L128Len,mitem->str,mitem->strLen);

			//设置下一个数据
			offset += mitem->size;
			mitem = mitem->next;
		}
	}
};
#endif
