// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2018 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors: Strizhniou Fiodar - fix build and runtime errors, refactoring.
//
// Description:
// Implementation of e32 image creation and dump for elf2e32 tool
// @internalComponent
// @released
//
//

// get E32ImageHeader class...
#define INCLUDE_E32IMAGEHEADER_IMPLEMENTATION
#define RETURN_FAILURE(_r) return (fprintf(stderr, "line %d\n", __LINE__),_r)

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#ifndef __LINUX__
    #include <io.h>
#endif
#include <time.h>
#include <stdio.h>

#include "h_ver.h"
#include "e32flags.h"
#include "checksum.h"
#include "pl_elfimage.h"
#include "pl_symbol.h"
#include "e32imagefile.h"
#include "errorhandler.h"
#include "pl_elfimports.h"
#include "elffilesupplied.h"
#include "parametermanager.h"
#include "pl_elflocalrelocation.h"

using namespace std;

struct E32RelocPageDesc {
    uint32_t aOffset;
    uint32_t aSize;
};

void CreateRelocations(ElfRelocations::Relocations & aRelocations, char * & aRelocs, size_t & aRelocsSize);
size_t RelocationsSize(ElfRelocations::Relocations & aRelocs);
uint16 GetE32RelocType(ElfRelocation * aReloc);

template <class T>
inline T Align(T v, size_t s)
{
	unsigned int inc = s-1;
	unsigned int mask = ~inc;
	unsigned int val = (unsigned int)v;
	unsigned int res = (val+inc) & mask;
	return (T)res;
}

/**
Constructor for TVersion class.
@internalComponent
@released
*/
TVersion::TVersion(TInt8 aMajor, TInt8 aMinor, TInt16 aBuild):
	iMajor(aMajor), iMinor(aMinor), iBuild(aBuild) {}


/**
Constructor for E32ImageChunkDesc class.
@internalComponent
@released
*/
E32ImageChunkDesc::E32ImageChunkDesc(const char * aData, size_t aSize, size_t aOffset, const char * aDoc):
	iData(aData), iSize(aSize), iOffset(aOffset), iDoc(aDoc)
{}

E32ImageChunkDesc::~E32ImageChunkDesc() {}

/**
This function writes its data in the buffer.
@param aPlace - a location in the buffer
@internalComponent
@released
*/
void E32ImageChunkDesc::Init(char * aPlace)
{
	memcpy(aPlace+iOffset, iData, iSize);
}

/**
Destructor for E32ImageChunks class.
@internalComponent
@released
*/
E32ImageChunks::~E32ImageChunks()
{
 	if(!iChunks.empty())
 	{
 		for(auto x: iChunks) delete x;
 	}
}

/**
This function creates and adds a chunk into a list.
@param aData - input buffer
@param asize - size of the input buffer
@param aOffset - byte offset of this chunk from the 1 byte in e32 image file.
@param aDoc - name of the chunk
@internalComponent
@released
*/
void E32ImageChunks::AddChunk(const char * aData, size_t aSize, size_t aOffset, const char * aDoc)
{
	E32ImageChunkDesc * aChunk = new E32ImageChunkDesc(aData, aSize, aOffset, aDoc);
	iChunks.push_back(aChunk);
	iOffset += Align(aSize, sizeof(TUint32));
}

/**
This function returns the list of chunks.
@internalComponent
@released
*/
ChunkList & E32ImageChunks::GetChunks()
{
	return iChunks;
}

/**
This function returns the current offset pointing to the last chunk that
was added into the list of chunks.
@internalComponent
@released
*/
size_t E32ImageChunks::GetOffset()
{
	return iOffset;
}

/**
This function sets the current offset pointing to the last chunk that
was added into the list of chunks.
@internalComponent
@released
*/
void E32ImageChunks::SetOffset(size_t aOffset)
{
	iOffset = aOffset;
}

void E32ImageChunks::SectionsInfo()
{
    for(auto x: iChunks)
    {
        printf("Added Chunks has size: %06zx for section: %s at address: %08zx\n", x->iSize, x->iDoc, x->iOffset);
    }
}

/** @brief Disasm section content
If lenth > 0 prints specified section size
*/
void E32ImageChunks::DisasmChunk(uint16_t index, uint32_t length, uint32_t pos)
{
    E32ImageChunkDesc *tmp = iChunks[index];
    if(!length) length = tmp->iSize;
    printf("Disassembled section: %s at addr: %08zx\n",tmp->iDoc, tmp->iOffset+pos);
    printf("Has data:\n");
    for(uint32_t i = 0, sep = 0; i < length; i++, sep++)
    {
        size_t k = *(uint8_t *)(tmp->iData + i + pos);

        printf("%02zx", k);
        if(sep == 16)
        {
            sep = 0;
            cout << "\n";
        }
        if((sep == 4)||(sep == 8)||(sep == 12)) cout << "   ";
    }
}
/**
Constructor for E32ImageFile class.
@internalComponent
@released
*/
E32ImageFile::E32ImageFile(ElfImage * aElfImage = nullptr, ElfFileSupplied *aUseCase = nullptr,
           ParameterManager * aManager = nullptr, E32ExportTable *aTable = nullptr) :
	iElfImage(aElfImage),
	iUseCase(aUseCase),
	iManager(aManager),
	iTable(aTable)
	{}

/**
This function generates the E32 image.
@internalComponent
@released
*/
void E32ImageFile::GenerateE32Image()
{
	if( iManager->SymNamedLookup() ){
		ProcessSymbolInfo();
	}
	ProcessImports();
	ProcessRelocations();
	ConstructImage();
	//PrintAddrInfo(0x58);
}

void E32ImageFile::PrintAddrInfo(uint32_t addr)
{
    if(addr < iHdr->iCodeOffset)
    {
        if(addr < sizeof(E32ImageHeader))
        {
            cout << "That address belongs to ";
            if( offsetof(class E32ImageHeader, iExportDirOffset) == addr)
                cout << "E32ImageHeader.iExportDirOffset.\n";
        }
        else if( (addr > sizeof(E32ImageHeader)) && (addr <
                        (sizeof(E32ImageHeader) + sizeof(E32ImageHeaderComp))) )
            cout << "That address placed in E32ImageHeaderComp Section\n";
        else
            cout << "That address placed in E32ImageHeaderV Section\n";
    }
}


/**
This function processes the import map by looking into the dso files
from which the symbols are imported. It also fetches the ordinal numbers
for the corresponding symbols.
@internalComponent
@released
*/
void E32ImageFile::ProcessImports()
{
	string strTab;
	vector<int> strTabOffsets;
	int numDlls = 0;
	int numImports = 0;
	bool namedLookup = iManager->SymNamedLookup();

	ElfImports::ImportLibs importLibs = iElfImage->GetImports();

	// First set up the string table and record offsets into string table of each
	// LinkAs name.
	for (auto p: importLibs)
	{
		ElfImports::RelocationList & relocs = p.second;
		char* aLinkAs = relocs[0]->iVerRecord->iLinkAs;

		strTabOffsets.push_back(strTab.size()); //
		string s = aLinkAs;
		strTab.insert(strTab.end(),s.begin(),s.end());
		strTab.insert(strTab.end(),0);
		numDlls++;
		numImports += relocs.size();
	}

	iNumDlls = numDlls;
	iNumImports = numImports;

	// Now we can figure out the size of everything
	size_t importSectionSize = sizeof(E32ImportSection) +
            (sizeof(E32ImportBlock) * numDlls) +
            (sizeof(uint32_t) * numImports);

	vector<Elf32_Word> aImportSection;

	// This is the 'E32ImportSection' header - fill with 0 for the moment
	aImportSection.push_back(0); // E32ImportSection::iSize = 0

	if( namedLookup ) {
		// These are the 0th ordinals imported into the import table, one
		// entry for each DLL.
		importSectionSize += (sizeof(uint32_t) * numDlls);
	}
	// Now fill in the E32ImportBlocks
	int idx = 0;
	for (auto p: importLibs)
	{
		ElfImports::RelocationList & imports = p.second;
		string dsoName = imports[0]->iVerRecord->iSOName;

		//const char * aDSO = FindDSO((*p).first);
		string aDSO = FindDSO(dsoName);

		aImportSection.push_back(strTabOffsets[idx] + importSectionSize);
		int nImports = imports.size();

		// Take the additional 0th ordinal import into account
		if( namedLookup ) nImports++;

		aImportSection.push_back(nImports);

		ElfImage aElfImage(aDSO);
		aElfImage.ProcessElfFile();

		for(auto aReloc: imports)
		{
			char * aSymName = iElfImage->GetSymbolName(aReloc->iSymNdx);
			unsigned int aOrdinal = aElfImage.GetSymbolOrdinal(aSymName);

			//check the reloc refers to Code Segment
			try
			{
				if (iElfImage->SegmentType(aReloc->iAddr) != ESegmentRO)
					throw Elf2e32Error(ILLEGALEXPORTFROMDATASEGMENT, aSymName, iElfImage->iElfInput);
			}
			/**This catch block introduced here is to avoid deleting partially constructed object(s).
			Otherwise global catch block will delete the partially constructed object(s) and the tool will crash.
			*/
			catch(ErrorHandler& aError)
			{
				aError.Report();
				exit(EXIT_FAILURE);
			}

			Elf32_Word aRelocOffset = iElfImage->GetRelocationOffset(aReloc);
			aImportSection.push_back(aRelocOffset);

			Elf32_Word * aRelocPlace = iElfImage->GetRelocationPlace(aReloc);
//todo: wtf??:: empty conditions???
			if (aOrdinal > 0xFFFF)
			{
			}
			if (aReloc->iAddend > 0xFFFF)
			{
			}
			* aRelocPlace = (aReloc->iAddend<<16) | aOrdinal;
		}

		if( namedLookup ) {
			TUint aImportTabEntryPos = aImportSection.size();
			// Keep track of the location of the entry
			iImportTabLocations.push_back(aImportTabEntryPos);
			// Put the entry as 0 now, which shall be updated
			aImportSection.push_back(0);
		}
		idx++;
	}

	assert(importSectionSize == aImportSection.size() * sizeof(Elf32_Word));

	size_t totalSize = Align(importSectionSize + strTab.size(), sizeof(Elf32_Word));

	// Fill in the section header now we have the correct value.
	aImportSection[0] = totalSize;

	// Now construct the unified section
	iImportSectionSize = totalSize;
	iImportSection = (uint32 *)new char[totalSize]();
	memcpy(iImportSection, (void *)&aImportSection.at(0), importSectionSize);
	char * buf = ((char *)iImportSection) + importSectionSize;
	memcpy(buf, strTab.data(), strTab.size());
}


/**
This function checks if a DSO file exists.
@param aPath - DSO file name.
@internalComponent
@released
*/
bool ProbePath(string & aPath)
{
	fstream input(aPath);
	bool r = input.is_open();
    input.close();
    return r;
}

/**
This function searches for a DSO in the libpath specified.
@param aName - DSO file name
@internalComponent
@released
*/
string E32ImageFile::FindDSO(string aName)
{
	string aDSOName(aName);
	string aDSOPath(aName);

	if (ProbePath(aDSOName))
		return aDSOName;

	ParameterManager::LibSearchPaths & paths = iManager->LibPath();
	for (auto x: paths)
	{
		string path(x);
		aDSOPath.erase();
		aDSOPath.insert(aDSOPath.end(), path.begin(), path.end());
		aDSOPath.insert(aDSOPath.end(), directoryseparator);
		aDSOPath.insert(aDSOPath.end(), aDSOName.begin(), aDSOName.end());
		if (ProbePath(aDSOPath))
		{
			return aDSOPath;
		}
	}
	throw Elf2e32Error(DSONOTFOUNDERROR, aDSOPath);
}

/**
This function processes Code and Data relocations.
@internalComponent
@released
*/
void E32ImageFile::ProcessRelocations()
{
	CreateRelocations(iElfImage->GetCodeRelocations(), iCodeRelocs, iCodeRelocsSize);
	CreateRelocations(iElfImage->GetDataRelocations(), iDataRelocs, iDataRelocsSize);
}

/**
This function creates Code and Data relocations from the corresponding
ELF form to E32 form.
@internalComponent
@released
*/
void CreateRelocations(ElfRelocations::Relocations & aRelocations, char * & aRelocs, size_t & aRelocsSize)
{
	size_t rsize = RelocationsSize(aRelocations);
	if (rsize)
	{
		aRelocsSize = Align(rsize + sizeof(E32RelocSection), sizeof(uint32));

		uint32 aBase = (*aRelocations.begin())->iSegment->p_vaddr;
		//add for cleanup to be done later..
		aRelocs = new char[aRelocsSize]();
		E32RelocSection * e32reloc = (E32RelocSection * )aRelocs;

		uint16 * data = (uint16 *)(e32reloc + 1);
		E32RelocPageDesc * startofblock = (E32RelocPageDesc *)data;

		int page = -1;
		int pagesize = sizeof(E32RelocPageDesc);
		for (auto r: aRelocations)
		{
			ElfLocalRelocation * aReloc = r;
			int p = aReloc->iAddr & 0xfffff000;
			if (page != p)
			{
				if (pagesize%4 != 0)
				{
					*data++ = 0;
					pagesize += sizeof(uint16);
				}
				if (page == -1) page = p;
				startofblock->aOffset = page - aBase;
				startofblock->aSize = pagesize;
				pagesize = sizeof(E32RelocPageDesc);
				page = p;
				startofblock = (E32RelocPageDesc *)data;
				data = (uint16 *)(startofblock + 1);
			}
			uint16 relocType = aReloc->Fixup();
			*data++ = (uint16)((aReloc->iAddr & 0xfff) | relocType);
			pagesize += sizeof(uint16);
		}
		if (pagesize%4 != 0)
		{
			*data++ = 0;
			pagesize += sizeof(uint16);
		}
		startofblock->aOffset = page - aBase;
		startofblock->aSize = pagesize;
		((E32RelocSection *)aRelocs)->iNumberOfRelocs = aRelocations.size();
		((E32RelocSection *)aRelocs)->iSize = rsize;
	}
}

/**
This function calculates the relocation taking into consideration
the page boundaries if they are crossed. The relocations are
sorted.
@param aRelocs - relocations found in the Elf file.
@internalComponent
@released
*/
size_t RelocationsSize(ElfRelocations::Relocations & relocs)
{
	size_t bytecount = 0;
	int page = -1;
	for(auto x: relocs)
	{
		int p = x->iAddr & 0xfffff000;
		if (page != p)
			{
			if (bytecount%4 != 0)
				bytecount += sizeof(uint16);
			bytecount += sizeof(E32RelocPageDesc); // page, block size
			page = p;
			}
		bytecount += sizeof(uint16);
		}
	if (bytecount%4 != 0)
		bytecount += sizeof(uint16);
	return bytecount;
}

/**
This function returns the E32 interpretation for an Elf relocation type.
@param rel - relocation entry.
@internalComponent
@released
*/
uint16 GetE32RelocType(ElfRelocation * rel)
{
	ESegmentType aSegType = rel->iSegmentType; // iReader->SegmentType(rel->iSymbol->st_value);
	switch (aSegType)
	{
	case ESegmentRO:
		return KTextRelocType;
	case ESegmentRW:
		return KDataRelocType;
	default:
		break;
	};

	// maybe this should be an error
	return KInferredRelocType;
}

/**
This function constructs the E32 image.
@internalComponent
@released
*/
void E32ImageFile::ConstructImage()
{
	InitE32ImageHeader();
	ComputeE32ImageLayout();
	SetE32ImgHdrFields();
	AllocateE32Image();
}

/**
This function calculates the timestamp.
@param aTime
@internalComponent
@released
*/
Int64 timeToInt64(TInt aTime)
{
	aTime-=(30*365*24*60*60+7*24*60*60);	// seconds since midnight Jan 1st, 2000
	Int64 daysTo2000AD=730497;
	Int64 t=daysTo2000AD*24*3600+aTime;	// seconds since 0000
	t=t+3600;								// BST (?)
	return t*1000000;						// milliseconds
}

/**
This function returns the E32 image header size.
@internalComponent
@released
*/
size_t E32ImageFile::GetE32ImageHeaderSize()
{
	return sizeof(E32ImageHeaderV);
}

/**
This function returns the extended E32 image header size.
@internalComponent
@released
*/
size_t E32ImageFile::GetExtendedE32ImageHeaderSize()
{
	return iHdrSize;
}

/**
This function sets the extended E32 image header size.
@internalComponent
@released
*/
void E32ImageFile::SetExtendedE32ImageHeaderSize(size_t aSize)
{
	iHdrSize = aSize;
}

/**
This function initialises the E32 image header fields.
@internalComponent
@released
*/
void E32ImageFile::InitE32ImageHeader()
{
	iHdr = iUseCase->AllocateE32ImageHeader();
	E32Flags *flg = new E32Flags(iManager);

	iHdr->iUid1 = 0;
	iHdr->iUid2 = 0;
	iHdr->iUid3 = 0;
	iHdr->iHeaderCrc = 0;
	iHdr->iSignature = 0x434f5045u;
	iHdr->iModuleVersion = 0x00010000u;
	iHdr->iCompressionType = 0;
	iHdr->iToolsVersion = TVersion(MajorVersion, MinorVersion, Build);
	Int64 ltime = timeToInt64(time(nullptr));
	iHdr->iTimeLo=(uint32)ltime;
	iHdr->iTimeHi=(uint32)(ltime>>32);
	iHdr->iFlags=flg->Run();
	// Confusingly, CodeSize means everything except writable data
	iHdr->iCodeSize = 0;
	iHdr->iDataSize = iElfImage->GetRWSize();
	iHdr->iHeapSizeMin = 0;
	iHdr->iHeapSizeMax = 0;
	iHdr->iStackSize = 0;
	iHdr->iBssSize = iElfImage->GetBssSize();
	iHdr->iEntryPoint = 0;
	iHdr->iCodeBase = iElfImage->GetROBase();
	iHdr->iDataBase = iElfImage->GetRWBase();
	iHdr->iDllRefTableCount = iNumDlls;
	iHdr->iExportDirOffset = 0;
	iHdr->iExportDirCount = iUseCase->GetNumExports();
	iHdr->iTextSize = iElfImage->GetROSize();
	iHdr->iCodeOffset = 0;
	iHdr->iDataOffset = 0;
	iHdr->iImportOffset = 0;
	iHdr->iCodeRelocOffset = 0;
	iHdr->iDataRelocOffset = 0;
	iHdr->iProcessPriority = (uint16)EPriorityForeground;
	iHdr->iUncompressedSize = 0;
	iHdr->iS.iSecureId = 0;
	iHdr->iS.iVendorId = 0;
	iHdr->iExceptionDescriptor = 0;
	iHdr->iSpare2 = 0;

	iHdr->iExportDescSize = iUseCase->GetExportDescSize();
	iHdr->iExportDescType = iUseCase->GetExportDescType();
	if (iHdr->iExportDescSize == 0) iHdr->iExportDesc[0] = 0;

	delete flg;
}

/**
This function creates the E32 image layout.
Insert call to E32ImageChunks::DisasmChunk() here if see content nedeed
@internalComponent
@released
*/
void E32ImageFile::ComputeE32ImageLayout()
{
	// E32Image header
	iChunks.AddChunk((char *)iHdr, Align(GetExtendedE32ImageHeaderSize(), sizeof(uint32)), 0, "Image Header");

	uint32 endOfHeader = iChunks.GetOffset();

		// Code section
	iHdr->iCodeOffset = endOfHeader;
	iChunks.AddChunk(iElfImage->GetRawROSegment(), iElfImage->GetROSize(), iHdr->iCodeOffset, "Code Section");

	// Exports Next - then we can set up CodeSize
	// Call out to the use case so it can decide how we do this
	// record exporttable offset for default case

	bool aSymLkupEnabled = iManager->SymNamedLookup();
	// The export table is required either when:
	//	a. there are exported symbols
	//	b. symbol lookup is enabled - because this table also indicates the dependencies
	bool aExportTableNeeded = (iHdr->iExportDirCount || aSymLkupEnabled) ? 1 : 0;

	if ( aExportTableNeeded && iTable->AllocateP()){
        iHdr->iExportDirOffset = iChunks.GetOffset() + 4;
		iChunks.AddChunk((char *)iTable->GetExportTable(),
                iTable->GetExportTableSize(), iChunks.GetOffset(), "Export Table");
    }

	// Symbol info next
	if( aSymLkupEnabled ){
		E32EpocExpSymInfoHdr* aSymHdrInfo = (E32EpocExpSymInfoHdr*)CreateSymbolInfo(iChunks.GetOffset());
		if( aSymHdrInfo )
			iChunks.AddChunk( (char*)aSymHdrInfo, aSymHdrInfo->iSize, iChunks.GetOffset(), "Symbol Info" );
	}

	// CodeSize is current offset - endof header offset
	iHdr->iTextSize = iHdr->iCodeSize = iChunks.GetOffset() - endOfHeader;

	// Data section
	if (iElfImage->GetRWSize())
	{
		iHdr->iDataOffset = iChunks.GetOffset();
		iChunks.AddChunk(iElfImage->GetRawRWSegment(), iElfImage->GetRWSize(), iHdr->iDataOffset, "Data Section");
	}

	// Import Section
	if (iImportSectionSize)
	{
		iHdr->iImportOffset = iChunks.GetOffset();
		iChunks.AddChunk((char *)iImportSection, iImportSectionSize, iHdr->iImportOffset, "Import Section");
	}

	// Code relocs
	if (iCodeRelocsSize)
	{
		iHdr->iCodeRelocOffset = iChunks.GetOffset();
		iChunks.AddChunk(iCodeRelocs, iCodeRelocsSize, iHdr->iCodeRelocOffset, "Code Relocs");
	}

	// Data relocs
	if (iDataRelocsSize)
	{
		iHdr->iDataRelocOffset = iChunks.GetOffset();
		iChunks.AddChunk(iDataRelocs, iDataRelocsSize, iHdr->iDataRelocOffset, "Data Relocs");
	}

	iLayoutDone = true;
}

/**
This function returns the byte offset in the E32 image from where the
export table starts.
@internalComponent
@released
*/
size_t E32ImageFile::GetExportOffset()
{
	return iChunks.GetOffset() + 4;
}

/**
This function returns E32 image size.
@internalComponent
@released
*/
size_t E32ImageFile::GetE32ImageSize()
{
	assert(iLayoutDone);
	return iChunks.GetOffset();
}

/**
This function creates the export bitmap also replacing the absent symbols
with the entry point functions.
@internalComponent
@released
*/
void E32ImageFile::CreateExportBitMap()
{
	int nexp = iUseCase->GetNumExports();
	size_t memsz = (nexp + 7) >> 3;
	iExportBitMap = new uint8[memsz];
	memset(iExportBitMap, 0xff, memsz);
	// skip header
	uint32 * exports = ((uint32 *)iUseCase->GetExportTable()) + 1;
	uint32 absentVal = EntryPointOffset() + iElfImage->GetROBase();
	iMissingExports = 0;
	for (int i=0; i<nexp; ++i)
	{
		if (exports[i] == absentVal)
		{
			iExportBitMap[i>>3] &= ~(1u << (i & 7));
			++iMissingExports;
		}
	}
}

/**
This function creates export desription for the absent symbols.
@internalComponent
@released
*/
void E32ImageFile::AddExportDescription()
{
	CreateExportBitMap();
	if (iMissingExports == 0)
		return;	// nothing to do

	int nexp = iUseCase->GetNumExports();
	size_t memsz = (nexp + 7) >> 3;	// size of complete bitmap
	size_t mbs = (memsz + 7) >> 3;	// size of meta-bitmap
	size_t nbytes = 0;
	unsigned int i;
	for (i=0; i<memsz; ++i){
		if (iExportBitMap[i] != 0xff) ++nbytes; // number of groups of 8
	}
	uint8 edt = KImageHdr_ExpD_FullBitmap;
	uint32 extra_space = memsz - 1;
	if (mbs + nbytes < memsz)
	{
		edt = KImageHdr_ExpD_SparseBitmap8;
		extra_space = mbs + nbytes - 1;
	}
	extra_space = (extra_space + sizeof(uint32) - 1) &~ (sizeof(uint32) - 1);

	iHdr->iExportDescType = edt;
	if (edt == KImageHdr_ExpD_FullBitmap)
	{
	    assert(memsz > 65536);
		iHdr->iExportDescSize = memsz;
		iHdr->iExportDesc[0] = iExportBitMap[0];
		uint8 * aDesc = new uint8[extra_space]();
		memcpy(aDesc, &iExportBitMap[1], memsz-1);
		iChunks.AddChunk((char *)aDesc,extra_space, iChunks.GetOffset(), "Export Description");
	}
	else
	{
	    assert(((mbs + nbytes) > 65536));
		iHdr->iExportDescSize = mbs + nbytes;
		uint8 * aBuf = new uint8[extra_space + 1]();
		TUint8* mptr = aBuf;
		TUint8* gptr = mptr + mbs;
		for (i=0; i<memsz; ++i)
		{
			if (iExportBitMap[i] != 0xff)
			{
				mptr[i>>3] |= (1u << (i&7));
				*gptr++ = iExportBitMap[i];
			}
		}
		iHdr->iExportDesc[0] = aBuf[0];
		uint8 * aDesc = new uint8[extra_space];
		memcpy(aDesc, aBuf+1, extra_space);
		delete [] aBuf;
		iChunks.AddChunk((char *)aDesc,extra_space, iChunks.GetOffset(), "Export Description");
	}
}

bool E32ImageFile::AllowDllData()
{
    if(!iManager->HasDllData())
        return false;

    ETargetType type = iManager->TargetTypeName();

    switch(type)
    {
        case EDll: case EPolyDll: case EExe: case EExexp: case EStdExe:
            return true;
        default:
            return false;
    }
	return false;
}

/**
This function sets the fields of the E32 image.
@internalComponent
@released
*/
void E32ImageFile::SetE32ImgHdrFields()
{
    E32ImageHeader *tmp = iManager->GetE32Header();
	// Arrange a header for this E32 Image
	iHdr->iCpuIdentifier = (uint16)ECpuArmV5;

	bool isDllp = iUseCase->ImageIsDll();
	if (isDllp)
	{
		iHdr->iFlags |= KImageDll;
		if(!AllowDllData())
        {
            auto z = iElfImage->iExports->GetExports(false);

            for(auto x: z)
            {
                if(x->CodeDataType() == SymbolTypeData)
                {
                    cout << "Found global symbol(s):\n";
                    break;
                }
            }

            for(auto x: z)
            {
                if(x->CodeDataType() == SymbolTypeData)
                    cout << "\t" << x->SymbolName() << "\n";
            }
            if (iHdr->iDataSize)
                throw Elf2e32Error(DLLHASINITIALISEDDATAERROR, iManager->ElfInput());
            if (iHdr->iBssSize)
                throw Elf2e32Error(DLLHASUNINITIALISEDDATAERROR, iManager->ElfInput());
        }

	}

	iHdr->iHeapSizeMin = iManager->HeapCommittedSize();
	iHdr->iHeapSizeMax = iManager->HeapReservedSize();
	iHdr->iStackSize = iManager->StackCommittedSize();


	iHdr->iEntryPoint = EntryPointOffset();

	EEntryPointStatus r = ValidateEntryPoint();
	if (r == EEntryPointCorrupt)
		throw Elf2e32Error(ENTRYPOINTCORRUPTERROR, iManager->ElfInput());
	else if (r == EEntryPointNotSupported)
		throw Elf2e32Error(ENTRYPOINTNOTSUPPORTEDERROR, iManager->ElfInput());

	SetUpExceptions();

    iHdr->iUid1=tmp->iUid1;
	iHdr->iUid2=tmp->iUid2;
	iHdr->iUid3=tmp->iUid3;

	SSecurityInfo *info = iManager->GetSSecurityInfo();
	iHdr->iS.iSecureId = info->iSecureId;
	iHdr->iS.iVendorId = info->iVendorId;

	iHdr->iS.iCaps = iManager->Capability();

	SetPriority(isDllp);
	SetFixedAddress(isDllp);

	iHdr->iModuleVersion = iManager->Version();
	iHdr->iCompressionType = iManager->CompressionMethod();
	UpdateHeaderCrc();
}

/**
This function returns the entry point of the E32 image .
@internalComponent
@released
*/
uint32_t E32ImageFile::EntryPointOffset()
{
	return iElfImage->EntryPointOffset();
}

/**
This function validates the entry point of the E32 image .
@internalComponent
@released
*/
E32ImageFile::EEntryPointStatus E32ImageFile::ValidateEntryPoint()
{
	uint32 epOffset = iHdr->iEntryPoint;
	if (epOffset & 3)
		return EEntryPointOK;	// if entry point not 4 byte aligned, must be old style
	uint32 fileOffset = epOffset + iElfImage->iCodeSegmentHdr->p_offset;
	if (fileOffset+4 > iChunks.GetOffset())
		return EEntryPointCorrupt;	// entry point is past the end of the file??
	int ept = 0;			// old style if first instruction not recognised
	uint8 * p = ELF_ENTRY_PTR(uint8, iElfImage->iElfHeader, fileOffset + 4);
	uint32 x = *--p;
	x<<=8;
	x|=*--p;
	x<<=8;
	x|=*--p;
	x<<=8;
	x|=*--p;
	if ((x & 0xffffff00) == 0xe31f0000)
	{
		// starts with tst pc, #n - new entry point
		ept = (x & 0xff) + 1;
	}
	if (ept>7)
		return EEntryPointNotSupported;
	iHdr->iFlags |= (ept<<KImageEptShift);
	return EEntryPointOK;
}

/**
This function sets the exciption descriptor in the E32 image .
@internalComponent
@released
*/
void E32ImageFile::SetUpExceptions()
{
	const char aExDescName[] = "Symbian$$CPP$$Exception$$Descriptor";
	Elf32_Sym * aSym = iElfImage->LookupStaticSymbol(aExDescName);
	if (aSym)
	{
		uint32 aSymVaddr = aSym->st_value;
		uint32 aROBase = iElfImage->GetROBase();
		uint32 aROSize = iElfImage->GetROSize();
		//check its in RO segment
		if (aSymVaddr < aROBase || aSymVaddr >= (aROBase + aROSize))
		{
			throw Elf2e32Error(EXCEPTIONDESCRIPTOROUTSIDEROERROR, iManager->ElfInput());
		}
		// Set bottom bit so 0 in header slot means an old binary.
		// The decriptor is always aligned on a 4 byte boundary.
		iHdr->iExceptionDescriptor = (aSymVaddr - aROBase) | 0x00000001;
	}
}

/**
This function sets the priority of the E32 exe.
@internalComponent
@released
*/
void E32ImageFile::SetPriority(bool isDllp)
{
	if (iManager->Priority())
	{
		if (isDllp)
		{
			cerr << "Warning: Cannot set priority of a DLL." << endl;
		}
		else
			iHdr->iProcessPriority = (unsigned short)iManager->Priority();
	}
}

/**
This function sets the fixed address flag of the E32 image .
@internalComponent
@released
*/
void E32ImageFile::SetFixedAddress(bool isDllp)
{
	if (iManager->FixedAddress())
	{
		if (isDllp)
		{
			cerr << "Warning: Cannot set fixed address for DLL." << endl;
		}
		else
			iHdr->iFlags|=KImageFixedAddressExe;
	}
	else
		iHdr->iFlags&=~KImageFixedAddressExe;
}

TUint TE32ImageUids::Check()
{
    return ((checkSum(((TUint8*)this)+1)<<16)|checkSum(this));
}
/**
Constructor for TE32ImageUids.
@internalComponent
@released
*/
TE32ImageUids::TE32ImageUids(TUint32 aUid1, TUint32 aUid2, TUint32 aUid3)
{
	iUids[0]=aUid1;
	iUids[1]=aUid2;
	iUids[2]=aUid3;
}

/**
Default constructor for TUidType class.
@internalComponent
@released
*/
TUidType::TUidType()
{
	memset(this, 0, sizeof(TUidType));
}

/**
Constructor for TUidType class.
@internalComponent
@released
*/
TUidType::TUidType(TUid aUid1,TUid aUid2,TUid aUid3)
{
	iUid[0]=aUid1;
	iUid[1]=aUid2;
	iUid[2]=aUid3;
}

// needed by E32ImageHeaderV::ValidateHeader...
TCheckedUid::TCheckedUid(const TUidType& aUidType)
{
    iType = aUidType;
    iCheck = ((TE32ImageUids*)this)->Check();
}

/**
This function updates the CRC of the E32 Image.
@internalComponent
@released
*/
void E32ImageFile::UpdateHeaderCrc()
{
	TE32ImageUids u(iHdr->iUid1, iHdr->iUid2, iHdr->iUid3);
	iHdr->iUidChecksum = u.Check();
	TInt hdrsz = GetExtendedE32ImageHeaderSize();
	iHdr->iUncompressedSize = iChunks.GetOffset() - Align(GetExtendedE32ImageHeaderSize(), sizeof(uint32));
	iHdr->iHeaderCrc = KImageCrcInitialiser;
	iHdr->iHeaderCrc = Crc32(iHdr, hdrsz);
}

/**
This function creates a buffer and writes all the data into the buffer.
@internalComponent
@released
*/
int32_t ValidateE32Image(const char *buffer, uint32_t size);
void E32ImageFile::AllocateE32Image()
{
	size_t imageSize = GetE32ImageSize();
	iE32Image = new char[imageSize]();

	ChunkList aChunkList = iChunks.GetChunks();
	for(auto p: aChunkList)
	{
		p->Init(iE32Image);
	}

	E32ImageHeaderV* header = (E32ImageHeaderV*)iE32Image;
	TInt headerSize = header->TotalSize();
	if(KErrNone!=header->ValidateWholeImage(iE32Image+headerSize, imageSize - headerSize))
		throw Elf2e32Error(VALIDATIONERROR, iManager->E32ImageOutput());

	if( KErrNone!=ValidateE32Image(iE32Image, imageSize) )
		throw Elf2e32Error(VALIDATIONERROR, iManager->E32ImageOutput());
}

/**
This function deflates the compressed data.
@param bytes
@param size
@param os
@internalComponent
@released
*/
void DeflateCompress(char* bytes, size_t size, ofstream & os);

/**
This function Paged Pack the compressed data.
@param bytes
@param size
@param os
@internalComponent
@released
*/
void CompressPages(TUint8 * bytes, TInt size, ofstream& os);


/**
This function writes into the final E32 image file.
@param aName - E32 image file name
@internalComponent
@released
*/
bool E32ImageFile::WriteImage(const char * aName)
{
	ofstream *os = new ofstream();
	os->open(aName, ofstream::binary|ofstream::out);

	if (os->is_open())
	{
		uint32 compression = iHdr->CompressionType();
		if (compression == KUidCompressionDeflate)
		{
			size_t aHeaderSize = GetExtendedE32ImageHeaderSize();
			size_t aBodySize = GetE32ImageSize() - aHeaderSize;
			os->write(iE32Image, aHeaderSize);
			DeflateCompress(iE32Image + aHeaderSize, aBodySize, *os);
		}
		else if (compression == KUidCompressionBytePair)
		{
			size_t aHeaderSize = GetExtendedE32ImageHeaderSize();
			os->write(iE32Image, aHeaderSize);

			// Compress and write out code part
			int offset = GetExtendedE32ImageHeaderSize();
			CompressPages( (TUint8*)iE32Image + offset, iHdr->iCodeSize, *os);


			// Compress and write out data part
			offset += iHdr->iCodeSize;
			int srcLen = GetE32ImageSize() - offset;

			CompressPages((TUint8*)iE32Image + offset, srcLen, *os);

		}
		else if (compression == 0)
		{
			os->write(iE32Image, GetE32ImageSize()); // image not compressed
		}

	}
	else
	{
		throw Elf2e32Error(FILEOPENERROR, aName);
	}
	os->close();
	if(os!=nullptr)
	{
		delete os;
		os = nullptr;
	}
	return true;
}


/**
Destructor for E32ImageFile class.
@internalComponent
@released
*/
E32ImageFile::~E32ImageFile()
{
	delete [] iData;
	delete [] iExportBitMap;
	delete [] iE32Image;
	delete [] iImportSection;
}

int DecompressPages(TUint8 * bytes, ifstream& is);

void E32ImageFile::ProcessSymbolInfo()
{
	Elf32_Addr elfAddr = iTable->iExportTableAddress - 4;// This location points to 0th ord.
	// Create a relocation entry for the 0th ordinal.
	ElfLocalRelocation *rel = new ElfLocalRelocation(iElfImage, elfAddr, 0, 0, R_ARM_ABS32,
		nullptr, ESegmentRO, nullptr, false);
	iElfImage->AddToLocalRelocations(rel);

	elfAddr += iTable->GetExportTableSize();// aPlace now points to the symInfo
	uint32 *zerothOrd = (uint32*)iUseCase->GetExportTable();
	*zerothOrd = elfAddr;
	elfAddr += sizeof(E32EpocExpSymInfoHdr);// aPlace now points to the symbol address
											// which is just after the syminfo header.
	if(!iElfImage->iExports)
		return;

	// Donot disturb the internal list sorting.
	ElfExports::Exports exports = iElfImage->iExports->GetExports(false);

//	std::cout << "aList.size() is: " << aList.size() << "\n";

//	int i = 0;
//	for(auto x: aList)
//    {
//        i++;
//        if(!x->iElfSym)
//            std::cout << "ABSENT exported function at pos: " << i << "\n";
//	}

	const char aPad[] = {'\0', '\0', '\0', '\0'};
/** TODO (Administrator#1#04/15/17): The nullptr iElfSym position corresponds to the Absent function in def file */
	for(auto x: exports ) {
		if(!x->iElfSym) continue;

		iSymAddrTab.push_back(x->iElfSym->st_value);
		// The symbol names always start at a 4-byte aligned offset.
		iSymNameOffset = iSymbolNames.size() >> 2;
		iSymNameOffTab.push_back(iSymNameOffset);

		iSymbolNames += x->SymbolName();
		iSymbolNames += '\0';
		TUint nameLen = iSymbolNames.size();
		TUint align = Align(nameLen, sizeof(int));
		align -= nameLen;
		if(align % 4){
			iSymbolNames.append(aPad, align);
		}
		//Create a relocation entry...
		rel = new ElfLocalRelocation(iElfImage, elfAddr, 0, 0, R_ARM_ABS32, nullptr,
			ESegmentRO, x->iElfSym, false);
		elfAddr += sizeof(uint32);
		iElfImage->AddToLocalRelocations(rel);
	}
}

char* E32ImageFile::CreateSymbolInfo(size_t aBaseOffset)
{
	E32EpocExpSymInfoHdr aSymInf;
	uint32 sizeofNames;

	SetSymInfo(aSymInf);
	if( aSymInf.iFlags & 1) {
		sizeofNames = sizeof(uint32);
	}
	else {
		sizeofNames = sizeof(uint16);
	}

	char* aInfo = new char[aSymInf.iSize]();
	memcpy(aInfo, (void*)&aSymInf, sizeof(aSymInf));

	TUint aPos = aSymInf.iSymbolTblOffset;
	memcpy(aInfo+aPos, (void*)&iSymAddrTab.at(0), iSymAddrTab.size()*sizeof(uint32));

	aPos += iSymAddrTab.size()*sizeofNames;
	aPos += iSymNameOffTab.size()*sizeofNames;
	aPos = Align(aPos, sizeof(uint32));

	TInt aOffLen = 2;
	if(aSymInf.iFlags & 1)
		aOffLen=4;
	for(auto x: iSymNameOffTab){
		memcpy( ((void*)(aInfo+aPos)), ((void*)&x), aOffLen);
		aPos += aOffLen;
	}

	aPos = aSymInf.iStringTableOffset;
	memcpy(aInfo+aPos, (void*)&iSymbolNames.at(0), iSymbolNames.size());

    /** TODO (Administrator#5#04/16/17): Unfinished code?!!! */

	// At the end, the dependencies are listed. They remain zeroes and shall be fixed up
	// while relocating.

	// Update the import table to have offsets to ordinal zero entries
	uint32 *aImportTab = iImportSection;

	uint32 aOffset = aBaseOffset - iHdr->iCodeOffset;// This gives the offset of syminfo table base
										// wrt the code section start
	aOffset += aSymInf.iDepDllZeroOrdTableOffset; // This points to the ordinal zero offset table now
	for(auto x: iImportTabLocations) {
		uint32 *aLocation = (aImportTab + x);
		*aLocation = aOffset;
		aOffset += sizeof(uint32);
	}

	return aInfo;
}

void E32ImageFile::SetSymInfo(E32EpocExpSymInfoHdr& aSymInfo)
{
	uint16 aNSymbols = (uint16) iSymAddrTab.size();
	aSymInfo.iSymCount = aNSymbols;

	uint32 symSize = sizeof(E32EpocExpSymInfoHdr);
	symSize += aNSymbols * sizeof(uint32); // Symbol addresses
	TUint aNameTabSz = iSymbolNames.size();
	TInt sizeofNames;

	if( iSymNameOffset < 0xffff) {
		sizeofNames = sizeof(uint16);
		aSymInfo.iFlags &= ~1;//reset the 0th bit
	}
	else {
		sizeofNames = sizeof(uint32);
		aSymInfo.iFlags |= 1;//set the 0th bit
	}
	symSize += Align((aNSymbols * sizeofNames), sizeof(uint32)); // Symbol name offsets
	aSymInfo.iStringTableOffset = symSize;
	symSize += aNameTabSz; // Symbol names in string tab
	aSymInfo.iStringTableSz = aNameTabSz;
	aSymInfo.iDepDllZeroOrdTableOffset = symSize;
	aSymInfo.iDllCount = iNumDlls ;
	symSize += iNumDlls * sizeof(uint32); // Dependency list - ordinal zero placeholder
	aSymInfo.iSize = symSize;
}

