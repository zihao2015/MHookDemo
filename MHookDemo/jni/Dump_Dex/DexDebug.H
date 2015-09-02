#ifndef __DexDebug_H_
#define __DexDebug_H_
#include "DexUtil.H"
struct DebugItem{
	DebugItem* 	next;
	u4	param_name;
};
//说明Size_Reg不表示DebugItem长度
struct DebugList{
	DebugList* 	next;
	Base 	BS;
	struct DebugItem* Item;
	u4	number;
	u4	Size_Reg;
};
class Dex_Debug:DexUtil{
public:
	DebugList* debug = NULL;
/*
************************************************************
*				Dex_Debug
*设置Dex_Debug*
************************************************************
*/
	Dex_Debug(){
		debug = (DebugList*)Alloc(sizeof(DebugList));
	}
/*
************************************************************
*				Dex_Debug
*设置Dex_Debug*
************************************************************
*/
	Dex_Debug(DexFile* inDex,Dex_Maps* inMap){
		DEXLOG("[Parse]Dex_Debug");
		mapsItem* inMapsItem = inMap->getTypeItem(kDexTypeDebugInfoItem);
		if(inMapsItem == NULL)return;
		u4	mMemStart =(u4)inDex->pHeader + inMapsItem->offset;
		u1* mMem =(u1*)mMemStart;
		for(int m_i = 0;m_i < inMapsItem->size;m_i++){
			DebugList* _List = (DebugList*)Alloc(sizeof(DebugList));
			_List->BS.SetLoadOffset((u4)mMem-(u4)inDex->pHeader);
			mMem = readLeb128(mMem,&_List->number);
			mMem = readLeb128(mMem,&_List->Size_Reg);
//			Parse("Dex_Debug number:%08x",_List->number);
//			Parse("Dex_Debug Size_Reg:%08x",_List->Size_Reg);
			u4 mData = 0;
			do{
				mMem = readLeb128(mMem,&mData);
				if(mData != 0){
					DebugItem* _Item = (DebugItem*)Alloc(sizeof(DebugItem));
					_Item->param_name = mData;
//					Parse("Dex_Debug param_name:%08x",mData);
					_List->Item = (DebugItem*)AddToLast(_List->Item,_Item);
				}
			}while(mData != 0);
			_List->BS.MemSize = SubAddr3(mMem,_List->BS.Offset_Load,inDex->pHeader);
			debug =  (DebugList*)AddToLast(debug,_List);
		}
	}
/*
************************************************************
*				DatatoFile
*设置DatatoFile*
************************************************************
*/
	void DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == debug){
			DEXLOG("没有存在debug数据,直接退出!");
			return ;
		}
		//获取debug模块内存，没有就新建
		DEXLOG("Dex_Debug DatatoFile");
		str_ModMem* modList =inMem->getByName("DebugInfo");
		if(NULL == modList){
			modList = inMem->newNameMem("DebugInfo",AlignAdd(GetSize(debug),0x04));
			DEXLOG("debug %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		U1*	mMem = modList->Addr;
		DebugList* _List =debug;
		while(NULL != _List){
			_List->BS.Offset_File = AddAddr2(modList->Offset,SubAddr2(mMem,modList->Addr));
			mMem = writeL128(mMem,_List->number);
			mMem = writeL128(mMem,_List->Size_Reg);
			DebugItem* _Item = _List->Item;
			while(NULL != _Item){
				mMem =  writeL128(mMem,_Item->param_name);
				_Item = _Item->next;
			}
			_List = _List->next;
		}
	}
/*
************************************************************
*				LoadToFile
*将加载地址改变为文件偏移地址
************************************************************
*/
	size_t LoadToFile(size_t inLoad){
		if(inLoad == 0)return 0;
		DebugList* mList = debug;
		while(mList != NULL){
			if(mList->BS.Offset_Load == inLoad){
				return mList->BS.Offset_File;
			}
			mList = mList->next;
		}
		return inLoad;
	}
};

#endif
