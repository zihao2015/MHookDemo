#ifndef __Dex_Annot_H_
#define __Dex_Annot_H_
#include "Dex_Encoded.H"
/*
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
//annotation element list
typedef struct Annot_List{
	struct Annot_List* next;
	Base	BS;
	u4		Ann_Ele;
	u4		Ele_Type;
	u4		Ele_Size;
	u4		Ele_Value;
	u4		EleValue_Len;
	Annot_Element*  Item;
};
//Annotation
typedef struct Annot{
	struct Annot* next;
	Base	BS;
	Annot_Element* Element;
	u4		Visibility;
	u4		Type;
	u4 		Count;
	Annot_List*	List;
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
	Annot_SRL_Item* iTem;
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
	Annot_SRL*	SRL = NULL;
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
//			Parse("%08x：%08x",mEleItem->Type,mEleItem->Value);
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
		if(NULL == maps->getTypeItem(kDexTypeAnnotationItem)){
			DEXLOG("[Parse]No Have Annot");
			return ;
		}
		//获取基本信息
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		mapsItem* _mapsItem = maps->getTypeItem(kDexTypeAnnotationItem);
		DEXLOG("[Parse]Annot %08x %08x",_mapsItem->offset,_mapsItem->size);
		//设置头地址
		u1* mAnnotAddr =(u1*)AddAddr2(pHeader,_mapsItem->offset);
		//遍历Annot
		Annot* _Annot =NULL;
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			Annot* mAnnot = (Annot*)Alloc(sizeof(Annot));
			//设置偏移加载地址
			mAnnot->BS.SetLoad((u4)pHeader,(u4)mAnnotAddr);
			//获取visibility、type、count
			mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->Visibility);
			mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->Type);
			mAnnotAddr = readLeb128(mAnnotAddr,&mAnnot->Count);
//			Parse("DexAnnot %08x：%08x %08x %08x",m_i,mAnnot->Visibility,mAnnot->Type,mAnnot->Count);
			for(int m_j = 0;m_j < mAnnot->Count;m_j++){
				Annot_List* mList =(Annot_List*)Alloc(sizeof(Annot_List));
				u4	mType = 0 ;
				u4	mStartType =(u4)mAnnotAddr;
				//首先判断是否为Array
				mAnnotAddr = readLeb128(mAnnotAddr,&mList->Ann_Ele);
				mAnnotAddr = readLeb128(mAnnotAddr,&mList->Ele_Type);
				if(kDexAnnotationArray == mList->Ele_Type){				//Array
					Annot_Element* mEle = NULL;
					mList->Ele_Type = mType;
					mAnnotAddr = readLeb128(mAnnotAddr,&mList->Ele_Size);
//					Parse("DexAnnot 出现Array %08x",mList->Ele_Size);
					mList->Item  = DexAnnot_Element(inDex,mAnnotAddr,mList->Ele_Size);
					mAnnotAddr += GetSize(mList->Item);
				}else{
					//NoArray
//					Parse("DexAnnot 未出现Array %08x",mList->Ele_Type);
					mList->EleValue_Len = getValueLen(mList->Ele_Type);
					switch(mList->EleValue_Len){
					case 4:
						mList->Ele_Value += *(mAnnotAddr+3)<<24;
					case 3:
						mList->Ele_Value += *(mAnnotAddr+2)<<16;
					case 2:
						mList->Ele_Value += *(mAnnotAddr+1)<<8;
					case 1:
						mList->Ele_Value += *mAnnotAddr & 0xFF;
						mAnnotAddr+=mList->EleValue_Len;
					}
				}
				mAnnot->List = (Annot_List*)AddToLast(mAnnot->List,mList);
			}
			mAnnot->BS.MemSize = SubAddr3(mAnnotAddr,mAnnot->BS.Offset_Load,pHeader);
			//将_item添加到列表中，AddToLast非常耗时
			if(_Annot == NULL){
				annot = (Annot*)AddToLast(annot,mAnnot);
				_Annot = (Annot*)GetLastLink(annot);
			}else{
				_Annot = (Annot*)AddLink(_Annot,mAnnot);
			}
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
		if(NULL == maps->getTypeItem(kDexTypeAnnotationsDirectoryItem)){
			DEXLOG("[Parse]No Have Annot DIR");
			return ;
		}
		//获取基本信息
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		mapsItem* _mapsItem = maps->getTypeItem(kDexTypeAnnotationsDirectoryItem);
		DEXLOG("[Parse]Annot %08x %08x",_mapsItem->offset,_mapsItem->size);
		//设置头地址
		u4* mAnnoDir =(u4*)AddAddr2(pHeader,_mapsItem->offset);
		//遍历AnnotDir
		AnnotDir* _anDir = NULL;
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			AnnotDir* mAnDir = (AnnotDir*)Alloc(sizeof(AnnotDir));
			//设置加载地址
			mAnDir->BS.SetLoad((u4)pHeader,(u4)mAnnoDir);
			//获取结构体数据
			mAnDir->Offset_Class = *mAnnoDir++;
			mAnDir->Size_Feilds = *mAnnoDir++;
			mAnDir->Size_Methods = *mAnnoDir++;
			mAnDir->Size_Params = *mAnnoDir++;
			//Fields+Methods+Param
			size_t mSize =mAnDir->Size_Feilds+mAnDir->Size_Methods+mAnDir->Size_Params;
			for(int m_j = 0;m_j< mSize;m_j++){
				AnnotDir_Item* mItem = (AnnotDir_Item*)Alloc(sizeof(AnnotDir_Item));
				mItem->BS.Offset_Load =  SubAddr2(mAnnoDir,pHeader);
				mItem->Idx =*mAnnoDir++;
				mItem->Offset_Annot =*mAnnoDir++;
				mAnDir->item = (AnnotDir_Item*)AddToLast(mAnDir->item,mItem);
			}
			//设置内存宽度、将数据
			mAnDir->BS.MemSize = SubAddr3(mAnnoDir,pHeader,mAnDir->BS.Offset_Load);
			//将_item添加到列表中，AddToLast非常耗时
			if(_anDir == NULL){
				anDir = (AnnotDir*)AddToLast(anDir,mAnDir);
				_anDir = (AnnotDir*)GetLastLink(annot);
			}else{
				_anDir = (AnnotDir*)AddLink(_anDir,mAnDir);
			}
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
		if(NULL == maps->getTypeItem(kDexTypeAnnotationSetRefList)){
			DEXLOG("[Parse]Map中不存在SRL");
			return ;
		}
		//获取基本信息
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		mapsItem* _mapsItem = maps->getTypeItem(kDexTypeAnnotationSetRefList);
		DEXLOG("[Parse]Ann_SRL %08x %08x",_mapsItem->offset,_mapsItem->size);
		//设置起始地址
		u4* mAnnoSRL =(u4*)AddAddr2(pHeader,_mapsItem->offset);
		//遍历mAnSRL
		Annot_SRL* _SRL = NULL;
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			Annot_SRL* mAnSRL = (Annot_SRL*)Alloc(sizeof(Annot_SRL));
			//设置加载地址
			mAnSRL->BS.SetLoad((u4)pHeader,(u4)mAnnoSRL);
			mAnSRL->Size = *mAnnoSRL;mAnnoSRL++;
			//遍历mAnSRL Item
			for(int m_j = 0;m_j< mAnSRL->Size;m_j++){
				Annot_SRL_Item* mItem = (Annot_SRL_Item*)Alloc(sizeof(Annot_SRL_Item));
				mItem->Offset  =*mAnnoSRL;mAnnoSRL++;
				mAnSRL->iTem = (Annot_SRL_Item*)AddToLast(mAnSRL->iTem,mItem);
			}
			mAnSRL->BS.MemSize = SubAddr3(mAnnoSRL,pHeader,mAnSRL->BS.Offset_Load);
			//将_item添加到列表中，AddToLast非常耗时
			if(_SRL == NULL){
				SRL = (Annot_SRL*)AddToLast(SRL,mAnSRL);
				_SRL = (Annot_SRL*)GetLastLink(SRL);
			}else{
				_SRL = (Annot_SRL*)AddLink(_SRL,mAnSRL);
			}
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
		if(NULL == maps->getTypeItem(kDexTypeAnnotationSetItem)){
			DEXLOG("[Parse]No Have Annot Set");
			return ;
		}
		//获取基本信息
		DexHeader*  pHeader = (DexHeader*)inDex->pHeader;
		mapsItem* _mapsItem = maps->getTypeItem(kDexTypeAnnotationSetItem);
		DEXLOG("[Parse]Ann_SRL %08x %08x",_mapsItem->offset,_mapsItem->size);
		//设置起始地址
		u4* mAnnoSet =(u4*)AddAddr2(pHeader,_mapsItem->offset);
		//遍历mAnSet
		Annot_Set* _AnSet = NULL;
		for(int m_i = 0;m_i < _mapsItem->size;m_i++){
			Annot_Set* mAnSet = (Annot_Set*)Alloc(sizeof(Annot_Set));
			//设置加载地址
			mAnSet->BS.SetLoad((u4)pHeader,(u4)mAnnoSet);
			//遍历mAnSRL Item
			for(int m_j = 0;m_j< mAnSet->Size;m_j++){
				Annot_Set_Item* mItem = (Annot_Set_Item*)Alloc(sizeof(Annot_Set_Item));
				mItem->Offset_Annot =*mAnnoSet;mAnnoSet++;
				mAnSet->Item = (Annot_Set_Item*)AddToLast(mAnSet->Item,mItem);
			}
			mAnSet->BS.MemSize =SubAddr3(mAnnoSet,pHeader,mAnSet->BS.Offset_Load);
			//将_item添加到列表中，AddToLast非常耗时
			if(_AnSet == NULL){
				anSet = (Annot_Set*)AddToLast(anSet,mAnSet);
				_AnSet = (Annot_Set*)GetLastLink(anSet);
			}else{
				_AnSet = (Annot_Set*)AddLink(_AnSet,mAnSet);
			}
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
		DEXLOG("[Parse]Dex_Annot SRL");
		Dex_Ann_SRL(inDex,maps);
		DEXLOG("[Parse]DexAnnot");
		DexAnnot(inDex,maps);
		DEXLOG("[Parse]Dex_AnnotSet");
		Dex_Ann_Set(inDex,maps);
		DEXLOG("[Parse]Dex_AnnotDir");
		Dex_AnnotDir(inDex,maps);
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
//				DEXLOG("[File]AnnotElementToData 传入Array");
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
			DEXLOG("[File]没有存在Annot_Item数据,直接推出!");
			return ;
		}
		//获取Annot模块内存，没有就新建
		DEXLOG("[File]Annot");
		str_ModMem* modList = inMem->getByName("AnnotItem");
		if(NULL == modList){
			modList = inMem->newNameMem("AnnotItem",AlignAdd(GetSize(annot),0x04));
			DEXLOG("[File]Annot_Item %08x %08x ",modList->Offset,modList->Length);
		}
		//设置MAPS
		inmaps->getTypeItem(kDexTypeAnnotationItem)->offset = modList->Offset;
		//初始化数据
		Annot* m_Annot = annot;
		u1* annot_Addr = (u1*)modList->Addr;
		//设置Annot
		DEXLOG("[File]Annot Code");
		while(NULL != m_Annot){
			//设置偏移

			m_Annot->BS.Offset_File = AddAddr2(modList->Offset,SubAddr2(annot_Addr,modList->Addr));
			memcpy(annot_Addr,(u1*)m_Annot->BS.GetLoad(),m_Annot->BS.MemSize);
			annot_Addr +=m_Annot->BS.MemSize;
			/*
			annot_Addr = writeL128(annot_Addr,m_Annot->Visibility);
			annot_Addr = writeL128(annot_Addr,m_Annot->Type);
			annot_Addr = writeL128(annot_Addr,m_Annot->Count);
			Annot_List* mList = m_Annot->List;/**/
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
		if(NULL == SRL){
			DEXLOG("[File]没有存在Annot SRL数据,直接推出!");
			return ;
		}
		//获取AnnotSRL模块内存，没有就新建
		str_ModMem* modList =inMem->getByName("AnnotSRL");
		if(NULL == modList){
			modList = inMem->newNameMem("AnnotSRL",AlignAdd(GetSize(SRL),0x04));
			DEXLOG("[File]AnnotSRL %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeItem(kDexTypeAnnotationSetRefList)->offset = modList->Offset;
		//
		Annot_SRL* mAnSRL = SRL;
		u4* mMemStart = (u4*)modList->Addr;
		while(NULL != mAnSRL){
			mAnSRL->BS.Offset_File =(u4)mMemStart;
			*mMemStart =mAnSRL->Size;mMemStart++;
			Annot_SRL_Item* mItem = mAnSRL->iTem;
			while(NULL != mItem){
				*mMemStart =LoadToFile(mItem->Offset);mMemStart++;
				mItem = mItem->next;
			}
			mAnSRL = mAnSRL->next;
		}
	}
/*
************************************************************
*					AnnotSetToData
*1.将输出到inMem块内存中，
************************************************************
*/
	void AnnotSetToData(Mod_Mem* inMem,Dex_Maps* inmaps){
		if(NULL == anSet){
			DEXLOG("没有存在Annot Set数据,直接推出!");
			return ;
		}
		//获取AnnotSet模块内存，没有就新建
		DEXLOG("AnnotSetToData");
		str_ModMem* modList =inMem->getByName("AnnotSet");
		if(NULL == modList){
			modList = inMem->newNameMem("AnnotSet",AlignAdd(GetSize(anSet),0x04));
			DEXLOG("Annot Set %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeItem(kDexTypeAnnotationSetItem)->offset = modList->Offset;
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
			DEXLOG("没有存在Annot Dir数据,直接推出!");
			return ;
		}
		//获取AnnotDir模块内存，没有就新建
		DEXLOG("AnnotDirToData");
		str_ModMem* modList =inMem->getByName("Annot_Dir");
		if(NULL == modList){
			modList = inMem->newNameMem("Annot_Dir",AlignAdd(GetSize(anDir),0x04));
			DEXLOG("Annot Dir %08x %08x %08x",modList->Addr,modList->Length,modList->Offset);
		}
		//设置MAPS
		inmaps->getTypeItem(kDexTypeAnnotationsDirectoryItem)->offset = modList->Offset;
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
		Annot_SRL* mSRLItem = SRL;
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
		Annot_SRL* mItem = SRL;
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
