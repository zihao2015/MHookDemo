#ifndef __DexClassCode_H_
#define __DexClassCode_H_
#include "DexCode.H"
//__________________________________________________________
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
class Dex_ClassCode : DexUtil{
public:
	DexClassData* ClassD = NULL;
/*
************************************************************
*				Dex_ClassCode
*解析ClassCode
************************************************************
*/
	Dex_ClassCode(DexFile* inDex,Dex_Maps* inmaps,Dex_Code* code){
		DEXLOG("[Parse]Dex_ClassCode");
		//MethodNum为添加，Code个数
		mapsItem* inMapClassCode =inmaps->getTypeItem(kDexTypeClassDataItem);
		int MethodNum = 0;
		//定位头地址
		u1* mCodeAddr =(u1*)((u4)inDex->pHeader + inMapClassCode->offset);
		//遍历全部的DexClassData
		for(int m_i =0;m_i < inMapClassCode->size;m_i++){
//			Parse("Dex_ClassCode %d",m_i);
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
					code->Add(inDex,mMethod->codeOff);
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
	void DatatoFile(Mod_Mem* inMem,Dex_Maps* inmaps,Dex_Code* code){
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
				if(code != NULL){
					mAddr = writeL128(mAddr,code->LoadToFile(mMethod->codeOff));
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
*				SetMemory
*1.输出数据
************************************************************
*/
	void SetMemory(str_ModMem* inMem,Dex_Code* code){
		DEXLOG("[+]ClassCode SetMemory Offset:0x%08x",inMem->Offset);
		DexClassData* mList = ClassD;
		u1* mAddr = inMem->Addr;
		while(mList != NULL){
			//
			mList->BS.Offset_File = inMem->Offset +(u4)mAddr - (u4)inMem->Addr;
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
				mAddr = writeL128(mAddr,code->LoadToFile(mMethod->codeOff));
				mMethod = mMethod->next;
			}
			mList = mList->next;
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
		DexClassData* mList = ClassD;
		while(mList != NULL){
			if(mList->BS.Offset_Load == inLoad){
//				DEXLOG("Dex_ClassDef %08x %08x",inLoad,mList->BS.Offset_File);
				return mList->BS.Offset_File;
			}
			mList = mList->next;
		}
		DEXLOG("Dex_ClassDef LoadToFile %08x",inLoad);
		return inLoad;
	}
/*
************************************************************
*				getCodeLen
*获取Code长度
************************************************************
*/
	size_t getCodeLen(){
		size_t mSize = 0;
		DexClassData* mList = ClassD;
		while(mList != NULL){
			mSize += mList->BS.MemSize;
			mList = mList->next;
		}
		return mSize;
	}
};


#endif
