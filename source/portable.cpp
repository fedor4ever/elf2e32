// Copyright (c) 2017 Strizhniou Fiodar.
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors: Strizhniou Fiodar - fix build and runtime errors.
//
// Description:
//


#include <string.h>
#include <iostream>
#include <portable.h>
#include "checksum.h"

/**
Validate this image header.

@param aFileSize				Total size of the file from which this header was created.
@param[out] aUncompressedSize	Returns the total size that the file data would be once decompressed.

@return KErrNone if no errors detected;
		KErrCorrupt if errors found;
		KErrNotSupported if image format not supported on this platform.
*/
TInt E32ImageHeaderV::ValidateHeader(TInt aFileSize, TUint32& aUncompressedSize) const
{
	const TUint KMaxDesSize = 0x0fffffffu; // maximum size of descriptor
	if(aFileSize==-1)
		{
		// file size unknown, set to maximum valid so rest of validation works...
		aFileSize = KMaxDesSize;
		}
	if(TUint(aFileSize)>KMaxDesSize)
		RETURN_FAILURE(KErrCorrupt); // file size negative or too big

	aUncompressedSize = 0;

	// check file is big enough to contain this header...
	if(aFileSize<(TInt)sizeof(*this))
		RETURN_FAILURE(KErrCorrupt);

	// check header format version...
	if((iFlags&KImageHdrFmtMask)!=KImageHdrFmt_V)
		RETURN_FAILURE(KErrNotSupported);

	// check header size...
	TUint headerSize = iCodeOffset;
	if(headerSize>TUint(aFileSize))
		RETURN_FAILURE(KErrCorrupt); // Fuzzer can't trigger this because Loader will fail earlier when reading header from file

	// check iCpuIdentifier...
	TCpu cpu = (TCpu)iCpuIdentifier;
	bool isARM = (cpu==ECpuArmV4 || cpu==ECpuArmV5 || cpu==ECpuArmV6);

	TUint32 pointerAlignMask = isARM ? 3 : 0;	// mask of bits which must be zero for aligned pointers/offsets

	// check iUid1,iUid2,iUid3,iUidChecksum...
	TUidType uids = *(const TUidType*)&iUid1;
	TCheckedUid chkuid(uids);
	const TUint32* pChkUid = (const TUint32*)&chkuid; // need hackery to verify the UID checksum since everything is private
	if(pChkUid[3]!=iUidChecksum)
		RETURN_FAILURE(KErrCorrupt);

	// check iSignature...
	if(iSignature!=0x434f5045) // 'EPOC'
		RETURN_FAILURE(KErrCorrupt);

	// check iHeaderCrc...
	TUint32 supplied_crc = iHeaderCrc;
	((E32ImageHeaderV*)this)->iHeaderCrc = KImageCrcInitialiser;
	uint32_t crc = 0;
	Crc32(crc, this, headerSize);
	((E32ImageHeaderV*)this)->iHeaderCrc = supplied_crc;
	if(crc!=supplied_crc)
		RETURN_FAILURE(KErrCorrupt);

	// check iModuleVersion...
	TUint32 mv = iModuleVersion;
	if(mv>=0x80000000u || (mv&0x0000ffffu)>0x8000u)
		RETURN_FAILURE(KErrNotSupported);

	// check iCompressionType and get uncompressed size...
	TUint compression = iCompressionType;
	TUint uncompressedSize = aFileSize;
	if(compression!=KFormatNotCompressed)
		{
		if(compression!=KUidCompressionDeflate && compression!=KUidCompressionBytePair)
	        RETURN_FAILURE(KErrNotSupported);  // unknown compression method
		uncompressedSize = headerSize+iUncompressedSize;
		if(uncompressedSize<headerSize)
			RETURN_FAILURE(KErrCorrupt); // size overflowed 32 bits
		}

	// check sizes won't overflow the limit for a descriptor (many Loader uses won't like that).
	if(uncompressedSize>KMaxDesSize)
		RETURN_FAILURE(KErrCorrupt);

	// check KImageDll in iFlags...
	if(iFlags&KImageDll)
		{
		if(iUid1!=TUint32(KDynamicLibraryUidValue))
			RETURN_FAILURE(KErrNotSupported);
		}
	else if(iUid1!=TUint32(KExecutableImageUidValue))
		RETURN_FAILURE(KErrNotSupported);

	// check iFlags for ABI and entry point types...
	if(isARM)
		{
		if((iFlags&KImageEptMask)!=KImageEpt_Eka2)
			RETURN_FAILURE(KErrNotSupported);
		#if defined(__EABI__)
			if((iFlags&KImageABIMask)!=KImageABI_EABI)
				RETURN_FAILURE(KErrNotSupported);
		#elif defined(__GCC32__)
			if((iFlags&KImageABIMask)!=KImageABI_GCC98r2)
				RETURN_FAILURE(KErrNotSupported);
		#endif
		}
	else
		{
		if(iFlags&KImageEptMask)
			RETURN_FAILURE(KErrNotSupported); // no special entry point type allowed on non-ARM targets
		if(iFlags&KImageABIMask)
			RETURN_FAILURE(KErrNotSupported);
		}

	// check iFlags for import format...
	if((iFlags&KImageImpFmtMask)>KImageImpFmt_PE2)
		RETURN_FAILURE(KErrNotSupported);

	// check iHeapSizeMin...
	if(iHeapSizeMin<0)
		RETURN_FAILURE(KErrCorrupt);

	// check iHeapSizeMax...
	if(iHeapSizeMax<iHeapSizeMin)
		RETURN_FAILURE(KErrCorrupt);

	// check iStackSize...
	if(iStackSize<0)
		RETURN_FAILURE(KErrCorrupt);

	// check iBssSize...
	if(iBssSize<0)
		RETURN_FAILURE(KErrCorrupt);

	// check iEntryPoint...
	if(iEntryPoint>=TUint(iCodeSize))
		RETURN_FAILURE(KErrCorrupt);
	if(iEntryPoint+KCodeSegIdOffset+sizeof(TUint32)>TUint(iCodeSize))
		RETURN_FAILURE(KErrCorrupt);
	if(iEntryPoint&pointerAlignMask)
		RETURN_FAILURE(KErrCorrupt); // not aligned

	// check iCodeBase...
	if(iCodeBase&3)
		RETURN_FAILURE(KErrCorrupt); // not aligned

	// check iDataBase...
	if(iDataBase&3)
		RETURN_FAILURE(KErrCorrupt); // not aligned

	// check iDllRefTableCount...
	if(iDllRefTableCount<0)
		RETURN_FAILURE(KErrCorrupt);
	if(iDllRefTableCount)
		{
		if(!iImportOffset)
			RETURN_FAILURE(KErrCorrupt); // we link to DLLs but have no import data
		}

	// check iCodeOffset and iCodeSize specify region in file...
	TUint codeStart = iCodeOffset;
	TUint codeEnd = codeStart+iCodeSize;
	if(codeEnd<codeStart)
		RETURN_FAILURE(KErrCorrupt);
//	if(codeStart<headerSize)
//		RETURN_FAILURE(KErrCorrupt); // can't happen because headerSize is defined as iCodeOffset (codeStart)
	if(codeEnd>uncompressedSize)
		RETURN_FAILURE(KErrCorrupt);

	// check iDataOffset and iDataSize specify region in file...
	TUint dataStart = iDataOffset;
	TUint dataEnd = dataStart+iDataSize;
	if(dataEnd<dataStart)
		RETURN_FAILURE(KErrCorrupt);
	if(!dataStart)
		{
		// no data...
		if(dataEnd)
			RETURN_FAILURE(KErrCorrupt);
		}
	else
		{
		if(dataStart<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(dataEnd>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((dataStart-codeStart)&pointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // data not aligned with respect to code
		}


	// check total data size isn't too bit...
	TUint totalDataSize = iDataSize+iBssSize;
	if(totalDataSize>0x7fff0000)
		RETURN_FAILURE(KErrNoMemory);

	// check iExportDirOffset and iExportDirCount specify region in code part...
	if(TUint(iExportDirCount)>65535)
		RETURN_FAILURE(KErrCorrupt); // too many exports
	if(iExportDirCount)
		{
		TUint exportsStart = iExportDirOffset;
		TUint exportsEnd = exportsStart+iExportDirCount*sizeof(TUint32);
		if(iFlags&KImageNmdExpData)
			exportsStart -= sizeof(TUint32); // allow for 0th ordinal
		if(exportsEnd<exportsStart)
			RETURN_FAILURE(KErrCorrupt);
		if(exportsStart<codeStart)
			RETURN_FAILURE(KErrCorrupt);
		if(exportsEnd>codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if((exportsStart-codeStart)&pointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within code section
		}

	// check iTextSize...
	if(TUint(iTextSize)>TUint(iCodeSize))
		RETURN_FAILURE(KErrCorrupt);

	// check iImportOffset...
	TUint start = iImportOffset;
	if(start)
		{
		TUint end = start+sizeof(E32ImportSection); // minimum valid size
		if(end<start)
			RETURN_FAILURE(KErrCorrupt);
		if(start<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(end>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((start-codeEnd)&pointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within 'rest of data'
		}

	// check iCodeRelocOffset...
	start = iCodeRelocOffset;
	if(start)
		{
		TUint end = start+sizeof(E32RelocSection); // minimum valid size
		if(end<start)
			RETURN_FAILURE(KErrCorrupt);
		if(start<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(end>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((start-codeEnd)&pointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within 'rest of data'
		}

	// check iDataRelocOffset...
	start = iDataRelocOffset;
	if(start)
		{
		TUint end = start+sizeof(E32RelocSection); // minimum valid size
		if(end<start)
			RETURN_FAILURE(KErrCorrupt);
		if(start<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(end>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((start-codeEnd)&pointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within 'rest of data'
		}

	// check exception descriptor...
	if(iExceptionDescriptor&1) // if valid...
		if(iExceptionDescriptor>=TUint(iCodeSize))
			RETURN_FAILURE(KErrCorrupt);

	TInt r = ValidateExportDescription();
	if(r!=KErrNone)
		RETURN_FAILURE(r);

	// done...
	aUncompressedSize = uncompressedSize;
	return KErrNone;
}

/**
Validate a whole executable image.

This runs all of the other validation methods in turn.

@param aBufferStart	Start of buffer containing the data after the header part of an image file.
@param aBufferSize	Size of data at aBufferStart.

@return KErrNone if image is valid, else KErrCorrupt or KErrNotSupported.
*/
TInt E32ImageHeaderV::ValidateWholeImage(void* aBufferStart, TUint aBufferSize) const
	{
	TUint32 dummyUncompressedSize;
	TInt r = ValidateHeader(TotalSize()+aBufferSize,dummyUncompressedSize);
	if(r!=KErrNone)
		return r;

	TInt endOfCodeOffset = iCodeSize;
	void* restOfFileData = ((TUint8*)aBufferStart)+endOfCodeOffset;
	TInt restOfFileSize = aBufferSize-endOfCodeOffset;

	E32RelocSection* dummy;
	r = ValidateRelocations(restOfFileData,restOfFileSize,iCodeRelocOffset,iCodeSize,dummy);
	if(r!=KErrNone)
		return r;
	r = ValidateRelocations(restOfFileData,restOfFileSize,iDataRelocOffset,iDataSize,dummy);
	if(r!=KErrNone)
		return r;

	TUint biggestImportCount;
	r = ValidateImports(restOfFileData,restOfFileSize,biggestImportCount);
	if(r!=KErrNone)
		return r;

	return r;
	}

/**
Validate a relocation section.

@param aBufferStart				Start of buffer containing the data after the code part in the image file.
@param aBufferSize				Size of data at aBufferStart.
@param aRelocationInfoOffset	File offset for relocation section. (#iCodeRelocOffset or #iDataRelocOffset.)
@param aRelocatedSectionSize	Size of section being relocated. (#iCodeSize or #iDataSize.)
@param[out] aRelocationSection	Set to the start of the relocation section in the given buffer.

@return KErrNone if relocation section is valid, else KErrCorrupt.
*/
TInt E32ImageHeaderV::ValidateRelocations(void* aBufferStart, TUint aBufferSize, TUint aRelocationInfoOffset, TUint aRelocatedSectionSize, E32RelocSection*& aRelocationSection) const
	{
	aRelocationSection = nullptr;
	if(!aRelocationInfoOffset)
		return KErrNone; // no relocations

	// get alignment requirements...
	TCpu cpu = (TCpu)iCpuIdentifier;
	bool isARM = (cpu==ECpuArmV4 || cpu==ECpuArmV5 || cpu==ECpuArmV6);
	TUint32 pointerAlignMask = isARM ? 3 : 0;	// mask of bits which must be zero for aligned pointers/offsets

	// buffer pointer to read relocation from...
	TUint8* bufferStart = (TUint8*)aBufferStart;
	TUint8* bufferEnd = bufferStart+aBufferSize;
	TUint baseOffset = iCodeOffset+iCodeSize; // file offset for aBufferStart
	TUint8* sectionStart = (bufferStart+aRelocationInfoOffset-baseOffset);
	TUint8* p = sectionStart;

	// read section header (ValidateHeader has alread checked this is OK)...
	E32RelocSection* sectionHeader = (E32RelocSection*)p;
	TUint size = sectionHeader->iSize;
	TUint relocsRemaining = sectionHeader->iNumberOfRelocs;
	E32IMAGEHEADER_TRACE(("E32RelocSection 0x%x %d",size,relocsRemaining));
	if(size&3)
		RETURN_FAILURE(KErrCorrupt); // not multiple of word size

	// calculate buffer range for block data...
	p = (TUint8*)(sectionHeader+1);  // start of first block
	TUint8* sectionEnd = p+size;
	if(sectionEnd<p)
		RETURN_FAILURE(KErrCorrupt); // math overflow
	if(sectionEnd>bufferEnd)
		RETURN_FAILURE(KErrCorrupt); // overflows buffer

	// process each block...
	while(p!=sectionEnd)
		{
		E32RelocBlock* block = (E32RelocBlock*)p;

		// get address of first entry in this block...
		TUint16* entryPtr = (TUint16*)(block+1);
		if((TUint8*)entryPtr<(TUint8*)block || (TUint8*)entryPtr>sectionEnd)
			RETURN_FAILURE(KErrCorrupt);  // overflows relocation section

		// read block header...
		TUint pageOffset = block->iPageOffset;
		TUint blockSize = block->iBlockSize;
		E32IMAGEHEADER_TRACE(("E32RelocSection block 0x%x 0x%x",pageOffset,blockSize));
		if(pageOffset&0xfff)
			RETURN_FAILURE(KErrCorrupt); // not page aligned
		if(blockSize<sizeof(E32RelocBlock))
			RETURN_FAILURE(KErrCorrupt); // blockSize must be at least that of the header just read
		if(blockSize&3)
			RETURN_FAILURE(KErrCorrupt); // not word aligned

		// caculate end of entries in this block...
		TUint16* entryEnd = (TUint16*)(p+blockSize);
		if(entryEnd<entryPtr)
			RETURN_FAILURE(KErrCorrupt); // math overflow
		if(entryEnd>(TUint16*)sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // overflows relocation section

		// process each entry in this block...
		while(entryPtr<entryEnd)
			{
			TUint entry = *entryPtr++;
			E32IMAGEHEADER_TRACE(("E32RelocSection entry 0x%04x",entry));
			if(!entry)
				continue;

			// check relocation type...
			TUint entryType = entry&0xf000;
			if(entryType!=KTextRelocType && entryType!=KDataRelocType && entryType!=KInferredRelocType)
				RETURN_FAILURE(KErrCorrupt);

			// check relocation is within section being relocated...
			TUint offset = pageOffset+(entry&0x0fff);
			if(offset>=aRelocatedSectionSize || offset+4>aRelocatedSectionSize)
				RETURN_FAILURE(KErrCorrupt); // not within section
			if(offset&pointerAlignMask)
				RETURN_FAILURE(KErrCorrupt); // not aligned correctly

			// count each relocation processed...
			--relocsRemaining;
			}

		// next sub block...
		p = (TUint8*)entryEnd;
		}

	// check number of relocations in section header is correct...
	E32IMAGEHEADER_TRACE(("E32RelocSection relocsRemaining=%d",relocsRemaining));
	if(relocsRemaining)
		RETURN_FAILURE(KErrCorrupt); // incorrect number of entries

	aRelocationSection = sectionHeader;
	return KErrNone;
	}

/**
Validate an import section.

For PE format imports, this also verifies that the Import Address Table fits within the code
part of the image.

@param aBufferStart				Start of buffer containing the data after the code part in the image file.
@param aBufferSize				Size of data at aBufferStart.
@param[out] aBiggestImportCount	Largest number of imports the image has from any single dependency.

@return KErrNone if section is valid (or absent), else KErrCorrupt.
*/
TInt E32ImageHeaderV::ValidateImports(void* aBufferStart, TUint aBufferSize, TUint& aBiggestImportCount) const
	{
	if(!iImportOffset)
		{
		aBiggestImportCount = 0;
		return KErrNone; // no imports
		}

	// get alignment requirements...
	TCpu cpu = (TCpu)iCpuIdentifier;
	bool isARM = (cpu==ECpuArmV4 || cpu==ECpuArmV5 || cpu==ECpuArmV6);
	TUint32 pointerAlignMask = isARM ? 3 : 0;	// mask of bits which must be zero for aligned pointers/offsets

	// buffer pointer to read imports from...
	TUint8* bufferStart = (TUint8*)aBufferStart;
	TUint8* bufferEnd = bufferStart+aBufferSize;
	TUint baseOffset = iCodeOffset+iCodeSize; // file offset for aBufferStart
	TUint8* sectionStart = (bufferStart+iImportOffset-baseOffset);
	TUint8* p = sectionStart;

	// read section header (ValidateHeader has alread checked this is OK)...
	E32ImportSection* sectionHeader = (E32ImportSection*)p;
	TUint size = sectionHeader->iSize;
	E32IMAGEHEADER_TRACE(("E32ImportSection 0x%x",size));

	// check section lies within buffer...
	p = (TUint8*)(sectionHeader+1);  // start of first import block
	TUint8* sectionEnd = sectionStart+size;
	if(sectionEnd<p)
		RETURN_FAILURE(KErrCorrupt); // math overflow or not big enough to contain header
	if(sectionEnd>bufferEnd)
		RETURN_FAILURE(KErrCorrupt); // overflows buffer

	// process each import block...
	TUint numDeps = iDllRefTableCount;
	TUint biggestImportCount = 0;
	TUint totalImports = 0;
	TUint importFormat = iFlags&KImageImpFmtMask;
	while(numDeps--)
		{
		// get block header...
		E32ImportBlock* block = (E32ImportBlock*)p;
		p = (TUint8*)(block+1);
		if(p<(TUint8*)block || p>sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // overflows buffer

		E32IMAGEHEADER_TRACE(("E32ImportBlock 0x%x %d",block->iOffsetOfDllName,block->iNumberOfImports));

		// check import dll name is within section...
		TUint8* name = sectionStart+block->iOffsetOfDllName;
		if(name<sectionStart || name>=sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // not within import section
		while(*name++ && name<sectionEnd)
			{}
		if(name[-1])
			RETURN_FAILURE(KErrCorrupt); // name overflows section
		E32IMAGEHEADER_TRACE(("E32ImportBlock %s",sectionStart+block->iOffsetOfDllName));

		// process import count...
		TUint numberOfImports = block->iNumberOfImports;
		if(numberOfImports>=0x80000000u/sizeof(TUint32))
			RETURN_FAILURE(KErrCorrupt); // size doesn't fit into a signed integer
		if(numberOfImports>biggestImportCount)
			biggestImportCount = numberOfImports;
		totalImports += numberOfImports;

		// process import data...

		// PE2 doesn't have any more data...
		if(importFormat==KImageImpFmt_PE2)
			continue;

		// get import data range...
		TUint32* imports = (TUint32*)p;
		TUint32* importsEnd = imports+numberOfImports;
		if(importsEnd<imports)
			RETURN_FAILURE(KErrCorrupt); // math overflow. Fuzzer can't trigger this because needs aBufferStart to be in to be >0x80000000
		if(importsEnd>(TUint32*)sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // overflows buffer

		// move pointer on to next block...
		p = (TUint8*)importsEnd;

		if(importFormat==KImageImpFmt_ELF)
			{
			// check imports are in code section...
			TUint32 limit = iCodeSize-sizeof(TUint32);
			while(imports<importsEnd)
				{
				TUint32 i = *imports++;
				if(i>limit)
					RETURN_FAILURE(KErrCorrupt);
				if(i&pointerAlignMask)
					RETURN_FAILURE(KErrCorrupt); // not word aligned
				}
			}
		else if(importFormat==KImageImpFmt_PE)
			{
			// import data is not used, so don't bother checking it
			}
		else
			{
			RETURN_FAILURE(KErrCorrupt); // bad import format, Fuzzer can't trigger this because import format checked by header validation
			}

		// next block...
		p = (TUint8*)block->NextBlock(importFormat);
		}

	// done processing imports; for PE derived files now check import address table (IAT)...
	if(importFormat==KImageImpFmt_PE || importFormat==KImageImpFmt_PE2)
		{
		if(totalImports>=0x80000000u/sizeof(TUint32))
			RETURN_FAILURE(KErrCorrupt); // size doesn't fit into a signed integer
		TUint importAddressTable = iTextSize; // offset for IAT
		if(importAddressTable&pointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // Fuzzer can't trigger this because PE imports are for X86 which doesn't have alignment restrictions
		TUint importAddressTableEnd = importAddressTable+sizeof(TUint32)*totalImports;
		if(importAddressTableEnd<importAddressTable || importAddressTableEnd>TUint(iCodeSize))
			RETURN_FAILURE(KErrCorrupt); // import address table overflows code part of file
		E32IMAGEHEADER_TRACE(("E32ImportSection IAT offsets 0x%x..0x%x",importAddressTable,importAddressTableEnd));
		}

	aBiggestImportCount = biggestImportCount;
	return KErrNone;
	}


//===============================================================
TCapabilitySet::TCapabilitySet(TCapability aCapability1, TCapability aCapability2)
{
    SetEmpty();
    AddCapability(aCapability1);
    AddCapability(aCapability2);
}

void TCapabilitySet::SetEmpty()
{
    memset(iCaps,0,sizeof(iCaps));
}

/*
Add a single capability to the set.
If the capability is not supported by this OS version then it is not added and
the set is left unchanged.
@see TCapabilitySet::SetAllSupported()
@param aCapability Capability to add.
*/
void TCapabilitySet::AddCapability(TCapability aCapability)
	{
	if((TUint32)aCapability<(TUint32)ECapability_Limit)
		{
		TInt index = aCapability>>3;
		TUint8 mask = (TUint8)(1<<(aCapability&7));
		mask &= ((TUint8*)&AllSupportedCapabilities)[index];
		((TUint8*)iCaps)[index] |= mask;
		}
	}

/**
Remove a single capability from the set, if it is present.
@param aCapability Capability to remove.
*/
void TCapabilitySet::RemoveCapability(TCapability aCapability)
	{
	if((TUint32)aCapability<(TUint32)ECapability_Limit)
		{
		TInt index = aCapability>>3;
		TUint8 mask = (TUint8)(1<<(aCapability&7));
		((TUint8*)iCaps)[index] &= ~mask;
		}
	}

/**
Perform a union of this capability set with another.
The result replaces the content of 'this'.
@param aCapabilities A cpability set
*/
void TCapabilitySet::Union(const TCapabilitySet& aCapabilities)
	{
	for(TInt n = (ECapability_Limit-1)>>5; n>=0; n--)
		iCaps[n] |= aCapabilities.iCaps[n];
	}

/**
Perform an intersection of this capability set with another.
The result replaces the content of 'this'.
@param aCapabilities A capability set
*/
void TCapabilitySet::Intersection(const TCapabilitySet& aCapabilities)
	{
	for(TInt n = (ECapability_Limit-1)>>5; n>=0; n--)
		iCaps[n] &= aCapabilities.iCaps[n];
	}

/**
Remove a set of capabilities from this set.
@param aCapabilities The set of capabilities to remove
*/
void TCapabilitySet::Remove(const TCapabilitySet& aCapabilities)
	{
	for(TInt n = (ECapability_Limit-1)>>5; n>=0; n--)
		iCaps[n] &= ~aCapabilities.iCaps[n];
	}

/**
Test if a single capability is present in the set.
The capability ECapability_None is always treated as being present.
@param aCapability The capability to test
@return 1 if the capability is present, 0 if it is not.
*/
bool TCapabilitySet::HasCapability(TCapability aCapability) const
	{
	if((TUint32)aCapability<(TUint32)ECapability_Limit)
		return (((TUint8*)iCaps)[aCapability>>3]>>(aCapability&7))&1;
	// coverity[dead_error_condition]
	if(aCapability==ECapability_None)
		return true;
	return false;  // Handles illegal argument and ECapability_Denied
	}

/**
Test if all the capabilities in a given set are present in this set
@param aCapabilities The capability set to test
@return A non-zero value if all the capabilities are present, zero otherwise.
*/
bool TCapabilitySet::HasCapabilities(const TCapabilitySet& aCapabilities) const
	{
	TUint32 checkFail=0;
	for(TInt n = (ECapability_Limit-1)>>5; n>=0; n--)
		checkFail |= aCapabilities.iCaps[n]&~iCaps[n];
	return checkFail?0:1;
	}

// Documented in header file
bool TCapabilitySet::NotEmpty() const
	{
	TUint32 notEmpty=0;
	for(TInt n = (ECapability_Limit-1)>>5; n>=0; n--)
		notEmpty |= iCaps[n];
	return notEmpty;
	}

//ECapability_None is assumed to be -1 in the internals of TSecurityPolicy
static_assert(ECapability_None == -1, "ECapability_None not -1");
