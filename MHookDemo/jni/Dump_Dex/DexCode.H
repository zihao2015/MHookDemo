#ifndef __DexCode_H_
#define __DexCode_H_

#include "DexDebug.H"
//__________________________________________________________
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

class Dex_Code : DexUtil{
public:
	 DexCodeL* Code = NULL;
 /*
 ************************************************************
 *				LoadToFile
*1.老函数，将加载内存替换成现在保存的文件偏移内存
 ************************************************************
 */
	 u4 LoadToFile(u4 inLoadOff){
		 if(inLoadOff == 0)return 0;
		 DexCodeL* mCode = Code;
		 while(mCode != NULL){
			 if(mCode->BS.Offset_Load == inLoadOff){
//				 Parse("Dex_Code LoadToFile %08x %08x",inLoadOff,mCode->BS.Offset_File);
				 return mCode->BS.Offset_File;
			 }
			 mCode = mCode->next;
		 }
		 Parse("Dex_Code LoadToFile %08x",inLoadOff);
		 return inLoadOff;
	 }
/*
************************************************************
*				Dex_Code
*设置Dex_Code
************************************************************
*/
	Dex_Code(DexFile* inDex){
		Parse("Dex_Code no add to link");
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
	void Add(DexFile* inDex,u4 inOffset){
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
			Parse("Code  Try_CatchIngfo %08x %08x",mCode->Try_CatchIngfo,mCode->Try_CatchSize);
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
	void DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps,Dex_Debug* debug){
		if(NULL == Code){
			Parse("没有存在Code数据,直接退出!");
			return ;
		}
		//获取Code模块内存，没有就新建
		Parse("Code DatatoFile");
		str_ModMem* modList =inMem->getByName("Code");
		if(NULL == modList){
			modList = inMem->newNameMem("Code",AlignAdd(GetSize(Code),0x1000));
			Parse("Code Addr %08x Length %08x Offset %08x",modList->Addr,modList->Length,modList->Offset);
			Parse("Code  GetCount %08x ",GetCount(Code));
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
};

#endif
