#ifndef __DexCode_H_
#define __DexCode_H_
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
	u4	CatchBlocks;
//
}DexCodeL;

class Dex_Code : DexUtil{
public:
	 DexCodeL* Code = NULL;
/*
************************************************************
*				Dex_Code
*设置Dex_Code
************************************************************
*/
	 DexCodeL* GetLastCode(){
		 if(Code == NULL)return NULL;
		 DexCodeL* mCode = Code;
		 while(mCode->next != NULL){
			 mCode = (DexCodeL*)mCode->next;
		 }
		 return mCode;
	 }
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
	}
/*
************************************************************
*				Dex_Code
*解析Dex_Code
************************************************************
*/
	void Add(DexFile* inDex,u4 inOffset){
		DexCodeL* mCode = (DexCodeL*)Alloc(sizeof(DexCodeL));
		//设置加载偏移地址 设置文件内存地址
		mCode->BS.SetLoadOffset(inOffset);
		u1* mMem =(u1*)((u4)inDex->pHeader + inOffset);
		u4	mMemStart =(u4)mMem;
		//复制DexCodeL
		mCode->Size_Reg =*(u2*)mMem;
		mMem += 2;
		mCode->Size_inArg =*(u2*)mMem;
		mMem += 2;
		mCode->Size_outArg =*(u2*)mMem;
		mMem += 2;
		mCode->Size_Tries =*(u2*)mMem;
		mMem += 2;
		mCode->DebugInfo =*(u4*)mMem;
		mMem += 4;
		mCode->Size_Code =*(u4*)mMem;
		mMem += 4;
		//获取整个方法代码宽度
		mCode->CodeInfo = Alloc(mCode->Size_Code*2);
		memcpy(mCode->CodeInfo,mMem,mCode->Size_Code*2);
		//定方位代码尾端，查看是否有其他数据
		mMem+= mCode->Size_Code*2;
		mMem = (u1*)((u4)(mMem+3)&(~3));
		//解析Try，
	//	Parse("mCode->Size_Tries:%08X",mCode->Size_Tries);
		for(int m_i = 0;m_i < mCode->Size_Tries;m_i++){
			//# Number of instructions in try
			DexTryL* mTry  =(DexTryL*)Alloc(sizeof(DexTryL));
			mTry->StartAddr =*(u4*)mMem;
			mMem += 4;
			mTry->Insncount =*(u2*)mMem;
			mMem += 2;
			mTry->Offset_Handler =*(u2*)mMem;
			mMem += 2;
			mCode->Try =  (DexTryL*)AddToLast(mCode->Try,mTry);
		}
		//解析Catch
		if(mCode->Size_Tries != 0){
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
					//# Bytecode address of catch-all handler
					mMem = readLeb128(mMem,&mList->catch_all);
				}else{
					for(int m_j = 0;m_j < msize; m_j++){
						DexCatchItemL* _Item = (DexCatchItemL*)Alloc(sizeof(DexCatchItemL));
						mMem = readLeb128(mMem,&_Item->Type);
						mMem = readLeb128(mMem,&_Item->Byte_Addr);
						mList->Item =  (DexCatchItemL*)AddToLast(mList->Item,_Item);
					}
					if(mList->Size_Handler >= 0x80000000){
						//# Bytecode address of catch-all handler
						mMem = readLeb128(mMem,&mList->catch_all);
					}
					//mCode->Catch =  (DexCatchListL*)AddToLast(mCode->Catch,mList);
				}
				mCode->Catch =  (DexCatchListL*)AddToLast(mCode->Catch,mList);
			}
		}
		//设置内存大小，必须对齐,
		mMem = (u1*)((u4)(mMem+3)&(~3));
		mCode->BS.SetMemSize((u4)mMem - mMemStart);
		Code =  (DexCodeL*)AddToLast(Code,mCode);
	}
/*
************************************************************
*				getCodeLen
*获取需要多少的Code空间*
************************************************************
*/
	size_t getCodeLen(){
		size_t msize = 0;
		DexCodeL* mCode = Code;
		while(mCode != NULL){
			msize += mCode->BS.MemSize;
			mCode = (DexCodeL*)mCode->next;
		}
		return msize;
	}
/*
************************************************************
*				DatatoFile
*设置DatatoFile*
************************************************************
*/
	void DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == Code){
			Parse("没有存在Code数据,直接退出!");
			return ;
		}
		//获取Code模块内存，没有就新建
		Parse("Code DatatoFile");
		str_ModMem* modList =inMem->getByName("Code");
		if(NULL == modList){
			modList = inMem->newNameMem("Code",GetSize(Code));
			Parse("Code %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置Code
		inmaps->setTypeOffset(kDexTypeCodeItem,modList->Offset);
		//
		DexCodeL* mCode = Code;
		u1* mMem = modList->Addr;
		while(NULL != mCode){
			mCode->BS.SetFileOffset(AddAddr2(mMem,SubAddr2(modList->Offset,modList->Addr)));
			//复制数据到内存
			*(u2*)mMem =mCode->Size_Reg;mMem+=2;
			*(u2*)mMem =mCode->Size_inArg;mMem+=2;
			*(u2*)mMem =mCode->Size_outArg;mMem+=2;
			*(u2*)mMem =mCode->Size_Tries;mMem+=2;
			*(u4*)mMem =mCode->DebugInfo;mMem+=4;
			*(u4*)mMem =mCode->Size_Code;mMem+=4;
			//获取代码宽度,对齐宽度
			u4 mCodeSize = mCode->Size_Code*2;
			memcpy(mMem,mCode->CodeInfo,mCodeSize);
			mMem += mCodeSize;
			mMem = (u1*)((u4)(mMem+3)&(~3));
			//判断TRY
			DexTryL* mTry = mCode->Try;
			while(NULL != mTry){
				*(u4*)mMem = mTry->StartAddr; mMem += 4;
				*(u2*)mMem = mTry->Insncount; mMem += 2;
				*(u2*)mMem = mTry->Offset_Handler; mMem += 2;
				mTry =mTry->next;
			}
			//判断Catch
			if(mCode->Size_Tries != 0){
				mMem = writeL128(mMem,mCode->CatchBlocks);
				DexCatchListL* mList =	mCode->Catch;
				for(int m_i = 0;m_i < mCode->CatchBlocks;m_i++){
					mMem = writeSL128(mMem,mList->Size_Handler);
					u4 msize = mList->Size_Handler;
					if(msize >= 0x80000000){
						msize = 0 - msize;
					}
					if(msize == 0){
						//# Bytecode address of catch-all handler
						mMem =writeL128(mMem,mList->catch_all);
					}else{
						DexCatchItemL* mItem = mList->Item;
					//	Parse("[+]DexCodeL　SetMemory msize:%08x",msize);
						for(int m_j = 0;m_j < msize; m_j++){
						//	Parse("[+]DexCodeL　SetMemory m_j:%08x",m_j);
						//	Parse("[+]DexCodeL　SetMemory mItem:%08x",mItem);
							mMem = writeL128(mMem,mItem->Type);
							mMem = writeL128(mMem,mItem->Byte_Addr);
							mItem = mItem->next;
						}
					//	Parse("[+]DexCodeL　SetMemory Size_Handler:%08x",mList->Size_Handler);
						if(mList->Size_Handler >= 0x80000000){
							//# Bytecode address of catch-all handler
							mMem = writeL128(mMem,mList->catch_all);
						}
					}
					mList = mList->next;
				}
			}
			mMem = (u1*)((u4)(mMem+3)&(~3));
			mCode = mCode->next;
		}
	}
/*
************************************************************
*				SetMemory
*老函数 新版不会使用
************************************************************
*/
	void SetMemory(str_ModMem* inMem){
		Parse("[+]DexCodeL　SetMemory:%08x,Size:%08x",inMem->Offset,inMem->Length);
		u1* mMem = inMem->Addr;
		u4	mMemStart = (u4)mMem;
		DexCodeL* mCode = Code;
		while(mCode != NULL){
//			Parse("[+]DexCodeL　SetMemory mCode");
			mCode->BS.SetFileOffset((u4)mMem-mMemStart+inMem->Offset);
			//复制数据到内存
			*(u2*)mMem =mCode->Size_Reg;mMem+=2;
			*(u2*)mMem =mCode->Size_inArg;mMem+=2;
			*(u2*)mMem =mCode->Size_outArg;mMem+=2;
			*(u2*)mMem =mCode->Size_Tries;mMem+=2;
			*(u4*)mMem =mCode->DebugInfo;mMem+=4;
			*(u4*)mMem =mCode->Size_Code;mMem+=4;
			//获取代码宽度,对齐宽度
			u4 mCodeSize = mCode->Size_Code*2;
			memcpy(mMem,mCode->CodeInfo,mCodeSize);
			mMem += mCodeSize;
			mMem = (u1*)((u4)(mMem+3)&(~3));
			//判断TRY
			DexTryL* mTry = mCode->Try;
//			Parse("[+]DexCodeL　SetMemory Try");
			for(int m_i = 0;m_i < mCode->Size_Tries;m_i++){
				*(u4*)mMem = mTry->StartAddr; mMem += 4;
				*(u2*)mMem = mTry->Insncount; mMem += 2;
				*(u2*)mMem = mTry->Offset_Handler; mMem += 2;
				mTry =mTry->next;
			}
			//判断Catch
		//	Parse("[+]DexCodeL　SetMemory Catch:%08x",mCode->Size_Tries);
			// # Number of catch handler blocks: 0x2
			if(mCode->Size_Tries != 0){
				mMem = writeL128(mMem,mCode->CatchBlocks);
				DexCatchListL* mList =	mCode->Catch;
				for(int m_i = 0;m_i < mCode->CatchBlocks;m_i++){
					mMem = writeSL128(mMem,mList->Size_Handler);
					u4 msize = mList->Size_Handler;
					if(msize >= 0x80000000){
						msize = 0 - msize;
					}
					if(msize == 0){
						//# Bytecode address of catch-all handler
						mMem =writeL128(mMem,mList->catch_all);
					}else{
						DexCatchItemL* mItem = mList->Item;
					//	Parse("[+]DexCodeL　SetMemory msize:%08x",msize);
						for(int m_j = 0;m_j < msize; m_j++){
						//	Parse("[+]DexCodeL　SetMemory m_j:%08x",m_j);
						//	Parse("[+]DexCodeL　SetMemory mItem:%08x",mItem);
							mMem = writeL128(mMem,mItem->Type);
							mMem = writeL128(mMem,mItem->Byte_Addr);
							mItem = mItem->next;
						}
					//	Parse("[+]DexCodeL　SetMemory Size_Handler:%08x",mList->Size_Handler);
						if(mList->Size_Handler >= 0x80000000){
							//# Bytecode address of catch-all handler
							mMem = writeL128(mMem,mList->catch_all);
						}
					}
					mList = mList->next;
				}
			}
			mMem = (u1*)((u4)(mMem+3)&(~3));
			//判断TRY
			mCode = mCode->next;
		}
	}
};

#endif
