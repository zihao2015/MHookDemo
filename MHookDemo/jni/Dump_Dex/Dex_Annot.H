#ifndef __Dex_Annot_H_
#define __Dex_Annot_H_
#include "Dex_Encoded.H"
/*
 *
*关系对应图
*	Annotation Dir(kDexTypeAnnotationsDirectoryItem)(级别大)
*		指向
*		annotation set ref list(kDexTypeAnnotationSetRefList)
*			指向
* 			annotation set(kDexTypeAnnotationSetItem)
* 				指向
*				annotation(kDexTypeAnnotationItem)(级别小)
*					指向
* 					annotation element
*/
//annotation element
typedef struct Annot_Element{
	struct Annot_Element* next;
	Base	BS;
	u4		Type;
	u4		Value;
	u4		ValueLen;

}Annot_Element;
//Annotation
typedef struct Annot{
	struct Annot* next;
	Base	BS;
	Annot_Element* Element;

	u4		Visibility;
	u4		Type;
	u4 		Count;
	u4		EleValue;
	u4		Ele_Type;
	size_t	Ele_Size;

}Annot;
//Annot_Set			kDexTypeAnnotationSetItem
typedef struct Annot_Set_Item{
	Annot_Set_Item* next;
	u4	Offset_Annot;
}Annot_Set_Item;
typedef struct Annot_Set{
	struct Annot_Set* next;
	Base	BS;
	Annot_Set_Item* Item;
	u4		Size;
}Annot_Set;
//Annot_SRL 			kDexTypeAnnotationSetRefList
typedef struct Annot_SRL_Item{
	struct Annot_SRL_Item* next;
	u4	Offset;
}Annot_SRL_Item;
typedef struct Annot_SRL{
	struct Annot_SRL* next;
	Base	BS;
	u4		Size;
	u4		Offset_AnnotSet;
}Annot_SRL;
//AnnotDir			kDexTypeAnnotationsDirectoryItem
typedef struct AnnotDir_Item{
	struct AnnotDir_Item* next;
	Base BS;

	u4	Idx;
	u4	Offset_Annot;
}AnnotDir_Item;
typedef struct AnnotDir{
	struct AnnotDir* next;
	Base BS;
	u4 	Offset_Class;
	u4	Size_Feilds;
	u4	Size_Methods;
	u4	Size_Params;
	AnnotDir_Item*	item;
}AnnotDir;
/*
************************************************************
*						Dex_Annot
*解析DexFile 下的Annot(Annot、AnnotSet、AnnotDir)
************************************************************
*/
class Dex_Annot : DexUtil{
public:
	AnnotDir* 	anDir = NULL;
	Annot_SRL*	anSRL = NULL;
	Annot_Set*	anSet = NULL;
	Annot* 		annot = NULL;
/*
************************************************************
*				DexAnnot_mEmElele
*此方法可能使用递归初始化函数，
*解析后的Mem地址，可使用老地址加上Annot_Element->BS.MemSize获取
*
************************************************************
*/
	Annot_Element* DexAnnot_Element(DexFile* inDex,void* inAddr,size_t inSize){
		u1* mAddr =(u1*)inAddr;
		Annot_Element* Ele = NULL;
		for(int m_i = 0;m_i < inSize;m_i++){
			Annot_Element* mEleItem = (Annot_Element*)Alloc(sizeof(Annot_Element));
			mEleItem->BS.Offset_Load = SubAddr2(mAddr,inDex->pHeader);
			mAddr = readLeb128(mAddr,&mEleItem->Type);
			mEleItem->ValueLen =getValueLen(mEleItem->Type);
			switch(mEleItem->ValueLen){
			case 4:
				mEleItem->Value+= *(mAddr+3)<<24;
			case 3:
				mEleItem->Value += *(mAddr+2)<<16;
			case 2:
				mEleItem->Value += *(mAddr+1)<<8;
			case 1:
				mEleItem->Value += *mAddr&0xFF;
				mAddr+=mEleItem->ValueLen;
			}
			mEleItem->BS.MemSize = SubAddr3(mAddr,inDex->pHeader,mEleItem->BS.Offset_Load);
			Ele = (Annot_Element*)AddToLast(Ele,mEleItem);
		}
		return Ele;
	}
/*
************************************************************
*				Dex_Annot
*从DexFile中读取Annot类
************************************************************
*/
	void DexAnnot(DexFile* inDex,Dex_Maps* maps){
		mapsItem* _mapsItem = maps->getTypeOffset(kDexTypeAnnotationItem);
		if(_mapsItem == NULL){
			Parse("[-]No Have Annot");
			return ;
		}
		//设置头地址
		u1* mAnnotAddr =(u1*)AddAddr2(inDex->pHeader,_mapsItem->offset);
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			Annot* mAnnot = (Annot*)Alloc(sizeof(Annot));
			//设置偏移加载地址
			mAnnot->BS.Offset_Load =  SubAddr2(mAnnotAddr,inDex->pHeader);
			//获取visibility、type,count
			mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->Visibility);
			mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->Type);
			mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->Count);
			mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->EleValue);
			u4	mType = 0 ;
			mAnnotAddr = readLeb128(mAnnotAddr,&mType);
			if(kDexAnnotationArray == mType){
				mAnnot->Ele_Type = mType;
				mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->Ele_Size);
				mAnnot->Element = DexAnnot_Element(inDex,mAnnotAddr,mAnnot->Ele_Size);
				mAnnotAddr += GetSize(mAnnot->Element);
			}else{
				Parse("DexAnnot 未出现Array");
			}
			mAnnot->BS.MemSize = SubAddr3(mAnnotAddr,mAnnot->BS.Offset_Load,inDex->pHeader);
			annot = (Annot*)AddToLast(annot,mAnnot);
		}
	}
/*
************************************************************
*				Dex_AnnotDir
*从DexFile中读取AnnotDir类
************************************************************
*/
	void Dex_AnnotDir(DexFile* inDex,Dex_Maps* maps){
		//首先判断MAPS是否存在kDexTypeAnnotationsDirectoryItem段
		mapsItem* _mapsItem = maps->getTypeOffset(kDexTypeAnnotationsDirectoryItem);
		if(_mapsItem == NULL){
			Parse("[+]No Have Annot DIR");
			return ;
		}
		u4* mAnnoDir =(u4*)AddAddr2(inDex->pHeader,_mapsItem->offset);
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			AnnotDir* mAnDir = (AnnotDir*)Alloc(sizeof(AnnotDir));
			//设置加载地址
			mAnDir->BS.Offset_Load = SubAddr2(mAnnoDir,inDex->pHeader);
			//获取结构体数据
			mAnDir->Offset_Class = *mAnnoDir++;
			mAnDir->Size_Feilds = *mAnnoDir++;
			mAnDir->Size_Methods = *mAnnoDir++;
			mAnDir->Size_Params = *mAnnoDir++;
			//Fields+Methods+Param
			size_t mSize =mAnDir->Size_Feilds+mAnDir->Size_Methods+mAnDir->Size_Params;
			for(int m_j = 0;m_j< mSize;m_j++){
				AnnotDir_Item* mItem = (AnnotDir_Item*)Alloc(sizeof(AnnotDir_Item));
				mItem->BS.Offset_Load =  SubAddr2(mAnnoDir,inDex->pHeader);
				mItem->Idx =*mAnnoDir++;
				mItem->Offset_Annot =*mAnnoDir++;
				mAnDir->item = (AnnotDir_Item*)AddToLast(mAnDir->item,mItem);
			}
			//设置内存宽度、将数据
			mAnDir->BS.MemSize = SubAddr3(mAnnoDir,inDex->pHeader,mAnDir->BS.Offset_Load);
			anDir = (AnnotDir*)AddToLast(anDir,mAnDir);
		}
	}
/*
************************************************************
*				Ann_SRL
*从DexFile中读取Ann_SRL类
************************************************************
*/
	void Dex_Ann_SRL(DexFile* inDex,Dex_Maps* maps){
		//首先判断MAPS是否存在kDexTypeAnnotationSetRefList段
		mapsItem* _mapsItem = maps->getTypeOffset(kDexTypeAnnotationSetRefList);
		if(_mapsItem == NULL){
			Parse("[+]No Have Annot SRL");
			return ;
		}
		//设置起始地址
		u4* mAnnoSRL =(u4*)SubAddr2(inDex->pHeader,_mapsItem->offset);
		//遍历mAnSRL
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			Annot_SRL* mAnSRL = (Annot_SRL*)Alloc(sizeof(Annot_SRL));
			//设置加载地址
			mAnSRL->BS.Offset_Load =SubAddr2(mAnnoSRL,inDex->pHeader);
			mAnSRL->Size = *mAnnoSRL;mAnnoSRL++;
			//遍历mAnSRL Item
			for(int m_j = 0;m_j< mAnSRL->Size;m_j++){
				Annot_SRL_Item* mItem = (Annot_SRL_Item*)Alloc(sizeof(Annot_SRL_Item));
				mItem->Offset  =*mAnnoSRL;mAnnoSRL++;
				mItem->next = (Annot_SRL_Item*)AddToLast(mItem->next,mItem);
			}
			mAnSRL->BS.MemSize = SubAddr3(mAnnoSRL,inDex->pHeader,mAnSRL->BS.Offset_Load);
			anSRL = (Annot_SRL*)AddToLast(anSRL,mAnSRL);
		}
	}
/*
************************************************************
*				Ann_Set
*从DexFile中读取Ann_Set类
************************************************************
*/
	void Dex_Ann_Set(DexFile* inDex,Dex_Maps* maps){
		//首先判断MAPS是否存在kDexTypeAnnotationSetRefList段
		mapsItem* _mapsItem = maps->getTypeOffset(kDexTypeAnnotationSetItem);
		if(_mapsItem == NULL){
			Parse("[+]No Have Annot Set");
			return ;
		}
		//设置起始地址
		u4* mAnnoSet =(u4*)AddAddr2(inDex->pHeader,_mapsItem->offset);
		//遍历mAnSet
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			Annot_Set* mAnSet = (Annot_Set*)Alloc(sizeof(Annot_Set));
			//设置加载地址
			mAnSet->BS.Offset_Load =SubAddr2(mAnnoSet,inDex->pHeader);
			mAnSet->Size = *mAnnoSet;mAnnoSet++;
			//遍历mAnSRL Item
			for(int m_j = 0;m_j< mAnSet->Size;m_j++){
				Annot_Set_Item* mItem = (Annot_Set_Item*)Alloc(sizeof(Annot_Set_Item));
				mItem->Offset_Annot =*mAnnoSet;mAnnoSet++;
				mAnSet->Item = (Annot_Set_Item*)AddToLast(mAnSet->Item,mItem);
			}
			mAnSet->BS.MemSize =SubAddr3(mAnnoSet,inDex->pHeader,mAnSet->BS.Offset_Load);
			anSet = (Annot_Set*)AddToLast(anSet,mAnSet);
		}
	}
/*
************************************************************
*				Dex_Annot
*从Dex解析Annot,需要Dex_Maps定位内存地址.
*需要解析AnnotItem -> AnnotSet -> _AnnotSet*
*Annot中包含
*kDexTypeAnnotationSetRefList、kDexTypeAnnotationSetItem、kDexTypeAnnotationItem、kDexTypeAnnotationsDirectoryItem
************************************************************
*/
	Dex_Annot(DexFile* inDex,Dex_Maps* maps){
		Parse("[+]Dex_Annot SRL");
		Dex_Ann_SRL(inDex,maps);
		Parse("[+]DexAnnot");
		DexAnnot(inDex,maps);
		Parse("[+]Dex_AnnotSet");
		Dex_Ann_Set(inDex,maps);
		Parse("[+]Dex_AnnotDir");
		Dex_AnnotDir(inDex,maps);
		Parse("[+]Dex_Over");
	}

/*
************************************************************
*					AnnotToData
*1.将输出到inMem块内存中，
************************************************************
*/
	void* AnnotElementToData(void* inAddr,Annot_Element* inEle){
		if(NULL == inEle){
		//	Parse("没有存在Annot_Element数据,直接推出!");
			return inAddr;
		}
		u1* mAddr = (u1*)inAddr;
		//遍历Annot_Element
		while(NULL != inEle){
			//遍历子段
			mAddr = writeL128(mAddr,inEle->Type);
			switch(inEle->ValueLen){
			//mEmElele
			case  kDexAnnotationArray:
				Parse("AnnotElementToData 传入Array");
			break;
			case 4:
				*(mAddr+3) = (inEle->Value>>24) ;
			case 3:
				*(mAddr+2) = (inEle->Value>>16);
			case 2:

				*(mAddr+1) = (inEle->Value>>8);
			case 1:
				*mAddr = inEle->Value;
				mAddr += inEle->ValueLen;
			}
			inEle = inEle->next;
		}
		return mAddr;
	}
/*
************************************************************
*					AnnotToData
*1.将输出到inMem块内存中，
************************************************************
*/
	void AnnotToData(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == annot){
			Parse("没有存在Annot_Item数据,直接推出!");
			return ;
		}
		//获取Annot模块内存，没有就新建
		Parse("AnnotToData");
		str_ModMem* modList = inMem->getByName("AnnotItem");
		if(NULL == modList){
			modList = inMem->newNameMem( "AnnotItem",GetSize(annot));
			Parse("Annot_Item %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeOffset(kDexTypeAnnotationItem)->offset = modList->Offset;
		//初始化数据
		Annot* m_Annot = annot;
		u1* annot_Addr = (u1*)modList->Addr;
		//设置Annot
		while(NULL != m_Annot){
			//设置偏移
			u1* next_Addr = (u1*)annot_Addr;
			m_Annot->BS.Offset_File = AddAddr2(modList->Offset,SubAddr2(annot_Addr,modList->Addr));
			annot_Addr = writeL128(annot_Addr,m_Annot->Visibility);
			annot_Addr = writeL128(annot_Addr,m_Annot->Type);
			annot_Addr = writeL128(annot_Addr,m_Annot->Count);
			annot_Addr = writeL128(annot_Addr,m_Annot->EleValue);
			if(kDexAnnotationArray == m_Annot->Ele_Type){
				annot_Addr = writeL128(annot_Addr,m_Annot->Ele_Type);
				annot_Addr = writeL128(annot_Addr,m_Annot->Ele_Size);
				annot_Addr = (u1*)AnnotElementToData(annot_Addr,m_Annot->Element);
			}else{
				Parse("AnnotToData 输出数据并非Array");
			}
//			annot_Addr = m_Annot->BS.MemSize + next_Addr;
			m_Annot = m_Annot->next;
		}
	}
/*
************************************************************
*					AnnotSRLToData
*1.将输出到inMem块内存中，
************************************************************
*/
	void AnnotSRLToData(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == anSRL){
			Parse("没有存在Annot SRL数据,直接推出!");
			return ;
		}
		//获取AnnotSRL模块内存，没有就新建
		str_ModMem* modList =inMem->getByName("AnnotSRL");
		if(NULL == modList){
			modList = inMem->newNameMem("AnnotSRL",GetSize(anSRL));
			Parse("AnnotSRL %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeOffset(kDexTypeAnnotationSetRefList)->offset = modList->Offset;
	}
/*
************************************************************
*					AnnotSetToData
*1.将输出到inMem块内存中，
************************************************************
*/
	void AnnotSetToData(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == anSet){
			Parse("没有存在Annot Set数据,直接推出!");
			return ;
		}
		//获取AnnotSet模块内存，没有就新建
		Parse("AnnotSetToData");
		str_ModMem* modList =inMem->getByName("AnnotSet");
		if(NULL == modList){
			modList = inMem->newNameMem("AnnotSet",GetSize(anSet));
			Parse("Annot Set %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeOffset(kDexTypeAnnotationSetItem)->offset = modList->Offset;
		//设置Annot_Set
		Annot_Set* mAnSet = anSet;
		u4* mAnnotSet = (u4*)modList->Addr;
		while(NULL != mAnSet){
			mAnSet->BS.Offset_File = AddAddr2(modList->Offset,SubAddr2(mAnnotSet,modList->Addr));
			*mAnnotSet++ = mAnSet->Size;
			Annot_Set_Item* mItem = mAnSet->Item;
			//设置Annot_Set Item
			while(NULL != mItem){
				*mAnnotSet++ = LoadToFile(mItem->Offset_Annot);
				mItem = mItem->next;
			}
			mAnSet = mAnSet->next;
		}
	}
/*
************************************************************
*					AnnotDir
*1.将输出到inMem块内存中，
************************************************************
*/
	void AnnotDirToData(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == anDir){
			Parse("没有存在Annot Dir数据,直接推出!");
			return ;
		}
		//获取AnnotDir模块内存，没有就新建
		Parse("AnnotDirToData");
		str_ModMem* modList =inMem->getByName("Annot_Dir");
		if(NULL == modList){
			modList = inMem->newNameMem("Annot_Dir",GetSize(anDir));
			Parse("Annot Dir %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeOffset(kDexTypeAnnotationsDirectoryItem)->offset = modList->Offset;
		//
		AnnotDir* m_AnnotDir = anDir;
		u4* annotDir_Addr = (u4*)modList->Addr;
		while(NULL != m_AnnotDir){
			m_AnnotDir->BS.Offset_File = AddAddr2(modList->Offset,SubAddr2(annotDir_Addr,modList->Addr));
			*annotDir_Addr++ = LoadToFile(m_AnnotDir->Offset_Class);
			*annotDir_Addr++ = m_AnnotDir->Size_Feilds;
			*annotDir_Addr++ = m_AnnotDir->Size_Methods;
			*annotDir_Addr++ = m_AnnotDir->Size_Params;
			AnnotDir_Item* mItem = m_AnnotDir->item;
			while(NULL != mItem){
				mItem->BS.Offset_File =  AddAddr2(modList->Offset,SubAddr2(annotDir_Addr,modList->Addr));
				*annotDir_Addr++ = mItem->Idx;
				*annotDir_Addr++ = LoadToFile(mItem->Offset_Annot);
				mItem =mItem->next;
			}
			m_AnnotDir = m_AnnotDir->next;
		}
	}
/*
************************************************************
*				getCodeLen
*将加载偏移地址设置为文件偏移
************************************************************
*/
	size_t LoadToFile(size_t inLoad){
		if(inLoad == 0)return 0;
		Annot* m_Annot= annot;
		while(m_Annot != NULL){
			if(m_Annot->BS.Offset_Load == inLoad){
				return m_Annot->BS.Offset_File;
			}
			m_Annot = m_Annot->next;
		}
		Annot_SRL* mSRLItem = anSRL;
		while(mSRLItem != NULL){
			if(mSRLItem->BS.Offset_Load == inLoad){
				return mSRLItem->BS.Offset_File;
			}
			mSRLItem = mSRLItem->next;
		}
		Annot_Set* mSetList = anSet;
		while(mSetList != NULL){
			if(mSetList->BS.Offset_Load == inLoad){
				return mSetList->BS.Offset_File;
			}
			mSetList = mSetList->next;
		}
		AnnotDir* mDir = anDir;
		while(mDir != NULL){
			if(mDir->BS.Offset_Load == inLoad){
				return mDir->BS.Offset_File;
			}
			mDir = mDir->next;
		}
		return inLoad;
	}
/*
************************************************************
*				getCodeLen
*获取整个大小
************************************************************
*/
	size_t getCodeLen(){
		size_t mSize = 0;
		Annot_SRL* mItem = anSRL;
		while(mItem != NULL){
			mSize += mItem->BS.MemSize;
			mItem =  mItem->next;
		}
		Annot_Set* mSetList = anSet;
		while(mSetList != NULL){
			mSize += mSetList->BS.MemSize;
			mSetList = mSetList->next;
		}
		AnnotDir* mDir = anDir;
		while(mDir != NULL){
			mSize += mDir->BS.MemSize;
			mDir = mDir->next;
		}
		return mSize;
	}
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
		case kDexAnnotationArray:
			return kDexAnnotationArray;
		}
		return mTypeLen+1;
	}
};

#endif
