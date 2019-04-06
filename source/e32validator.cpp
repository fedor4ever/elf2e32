#include <cstdio>
#include <assert.h>

#include "e32common.h"
#include "e32parser.h"
#include "e32validator.h"
#include "message.h"

#ifndef RETURN_FAILURE
#define RETURN_FAILURE(_r) return (fprintf(stderr, "line %d\n", __LINE__),_r)
#endif // RETURN_FAILURE

int32_t ValidateE32Image(const char *buffer, uint32_t size)
{
    E32Validator *v = new E32Validator(buffer, size);
    int32_t res = v->ValidateE32Image();
    delete v;
    return res;
}

E32Validator::E32Validator(const char *buffer, uint32_t size):
    iBufSize(size)
{
    iParser = new E32Parser(nullptr, buffer);
}

E32Validator::~E32Validator()
{
    delete iParser;
}

int32_t E32Validator::ValidateE32Image()
{
    iHdr = iParser->GetFileLayout();
    iHdrV = iParser->GetE32HdrV();

    int32_t r = ValidateHeader();
	if(r!=KErrNone)
		return r;

	r = ValidateRelocations(iHdr->iCodeRelocOffset,iHdr->iCodeSize);
	if(r!=KErrNone)
		return r;
	r = ValidateRelocations(iHdr->iDataRelocOffset,iHdr->iDataSize);
	if(r!=KErrNone)
		return r;

	r = ValidateImports();

	return r;
}

uint32_t GetUidChecksum(uint32_t uid1, uint32_t uid2, uint32_t uid3);
uint32_t Crc32(const void * aPtr, uint32_t aLength);
int32_t E32Validator::ValidateHeader()
{
    iHdr = iParser->GetFileLayout();

    const uint32_t KMaxDesSize = 0x0fffffffu; // maximum size of descriptor
	if(iBufSize == 0)
    {
		// file size unknown, set to maximum valid so rest of validation works...
		iBufSize = KMaxDesSize;
    }

    if(iBufSize > KMaxDesSize)
		RETURN_FAILURE(KErrCorrupt); // file size negative or too big

	// check file is big enough to contain this header...
	size_t hdrSize = sizeof(E32ImageHeader);
	if(iBufSize < hdrSize)
		RETURN_FAILURE(KErrCorrupt);

	uint32_t hdrfmt = HdrFmtFromFlags(iHdr->iFlags);
    if( hdrfmt == KImageHdrFmt_J)
    {
        hdrSize += sizeof(E32ImageHeaderJ);
        if(iBufSize < hdrSize)
            RETURN_FAILURE(KErrCorrupt);
    }

    if( hdrfmt == KImageHdrFmt_V)
    {
        hdrSize += sizeof(E32ImageHeaderV) + sizeof(E32ImageHeaderJ);
        if(iBufSize < hdrSize)
            RETURN_FAILURE(KErrCorrupt);
    }

	// check header format version...
	// KImageHdrFmt_J, KImageHdrFmt_Original, KImageHdrFmt_V
	if( (hdrfmt != KImageHdrFmt_V) && (hdrfmt != KImageHdrFmt_J) &&
        (hdrfmt != KImageHdrFmt_Original) )
    {
		RETURN_FAILURE(KErrNotSupported);
    }


	// check header size...
	if(iHdr->iCodeOffset > iBufSize)
		RETURN_FAILURE(KErrCorrupt); // Fuzzer can't trigger this because Loader will fail earlier when reading header from file

	// check iCpuIdentifier...
	Cpu cpu = (Cpu)iHdr->iCpuIdentifier;
	bool isARM = (cpu==ArmV4Cpu || cpu==ArmV5Cpu || cpu==ArmV6Cpu);
	iPointerAlignMask = isARM ? 3 : 0; // mask of bits which must be zero for aligned pointers/offsets

	uint32_t checksum = GetUidChecksum(iHdr->iUid1, iHdr->iUid2, iHdr->iUid3);
    if(checksum != iHdr->iUidChecksum)
		RETURN_FAILURE(KErrCorrupt);

    // check iSignature...
	if( *(uint32_t*)(iHdr->iSignature) != 0x434f5045) // 'EPOC'
    {
        char* f = iParser->GetBufferedImage();
        if((f[1] == 'E')&&(f[2] == 'L')&&(f[3] == 'F'))
            Message::GetInstance()->ReportMessage(ERROR, MISMATCHTARGET, "validation buffer");
		RETURN_FAILURE(KErrCorrupt);
    }

	// check iHeaderCrc...
	uint32_t orig_crc = iHdr->iHeaderCrc;
	iHdr->iHeaderCrc = KImageCrcInitialiser;
	uint32_t crc = Crc32(iHdr, iHdr->iCodeOffset);
	iHdr->iHeaderCrc = orig_crc;
	if(crc!=orig_crc)
		RETURN_FAILURE(KErrCorrupt);

	// check iModuleVersion...
	uint32_t mv = iHdr->iModuleVersion;
	if(mv>=0x80000000u || (mv&0x0000ffffu)>0x8000u)
		RETURN_FAILURE(KErrNotSupported);

    // check iCompressionType and get uncompressed size...
    uint32_t compression = iHdr->iCompressionType;
    uint32_t uncompressedSize = iBufSize;

    if(compression!=KFormatNotCompressed)
    {
        uncompressedSize = iParser->UncompressedFileSize();
		if(compression != KUidCompressionDeflate && compression != KUidCompressionBytePair)
	        RETURN_FAILURE(KErrNotSupported);  // unknown compression method
		if(uncompressedSize < iHdr->iCodeOffset)
			RETURN_FAILURE(KErrCorrupt); // size overflowed 32 bits
    }

	// check sizes won't overflow the limit for a descriptor (many Loader uses won't like that).
	if(uncompressedSize > KMaxDesSize)
		RETURN_FAILURE(KErrCorrupt);

	// check KImageDll in iFlags...
	if(iHdr->iFlags&KImageDll)
    {
		if(iHdr->iUid1!=KDynamicLibraryUidValue)
			RETURN_FAILURE(KErrNotSupported);
    }
	else if(iHdr->iUid1!=KExecutableImageUidValue)
		RETURN_FAILURE(KErrNotSupported);

	// check iFlags for ABI and entry point types...
	if(isARM)
    {
        if((iHdr->iFlags&KImageEptMask)!=KImageEpt_Eka2)
            RETURN_FAILURE(KErrNotSupported);
    /// TODO (Administrator#1#11/03/18): Enable checks for other arm targets too!
        #if defined(__EABI__)
        if((iFlags&KImageABIMask)!=KImageABI_EABI)
            RETURN_FAILURE(KErrNotSupported);
        #elif defined(__GCC32__)
        if((iFlags&KImageABIMask)!=KImageABI_GCC98r2)
            RETURN_FAILURE(KErrNotSupported);
        #endif
    }
	else{
		if(iHdr->iFlags&KImageEptMask)
			RETURN_FAILURE(KErrNotSupported); // no special entry point type allowed on non-ARM targets
		if(iHdr->iFlags&KImageABIMask)
			RETURN_FAILURE(KErrNotSupported);
    }

	// check iFlags for import format...
	if((iHdr->iFlags&KImageImpFmtMask)>KImageImpFmt_PE2)
		RETURN_FAILURE(KErrNotSupported);

	// check iHeapSizeMin...
	if(iHdr->iHeapSizeMin<0)
		RETURN_FAILURE(KErrCorrupt);

	// check iHeapSizeMax...
	if(iHdr->iHeapSizeMax<iHdr->iHeapSizeMin)
		RETURN_FAILURE(KErrCorrupt);

	// check iStackSize...
	if(iHdr->iStackSize<0)
		RETURN_FAILURE(KErrCorrupt);

	// check iBssSize...
	if(iHdr->iBssSize<0)
		RETURN_FAILURE(KErrCorrupt);

	// check iEntryPoint...
	if(iHdr->iEntryPoint>=iHdr->iCodeSize)
		RETURN_FAILURE(KErrCorrupt);
	if(iHdr->iEntryPoint+KCodeSegIdOffset+sizeof(uint32_t)>iHdr->iCodeSize)
		RETURN_FAILURE(KErrCorrupt);
	if(iHdr->iEntryPoint&iPointerAlignMask)
		RETURN_FAILURE(KErrCorrupt); // not aligned

	// check iCodeBase...
	if(iHdr->iCodeBase&3)
		RETURN_FAILURE(KErrCorrupt); // not aligned

	// check iDataBase...
	if(iHdr->iDataBase&3)
		RETURN_FAILURE(KErrCorrupt); // not aligned

	// check iDllRefTableCount...
	if(iHdr->iDllRefTableCount<0)
		RETURN_FAILURE(KErrCorrupt);
	if(iHdr->iDllRefTableCount && !iHdr->iImportOffset)
        RETURN_FAILURE(KErrCorrupt); // we link to DLLs but have no import data

	// check iCodeOffset and iCodeSize specify region in file...
	uint32_t codeStart = iHdr->iCodeOffset;
	uint32_t codeEnd = iHdr->iCodeSize+codeStart;
	if(codeEnd < codeStart)
		RETURN_FAILURE(KErrCorrupt);
//	if(codeStart<headerSize)
//		RETURN_FAILURE(KErrCorrupt); // can't happen because headerSize is defined as iCodeOffset (codeStart)
	if(codeEnd>uncompressedSize)
		RETURN_FAILURE(KErrCorrupt);

	// check iDataOffset and iDataSize specify region in file...
	uint32_t dataStart = iHdr->iDataOffset;
	uint32_t dataEnd = iHdr->iDataSize + dataStart;
	if(dataEnd<dataStart)
		RETURN_FAILURE(KErrCorrupt);
    // no data...
	if(!dataStart && dataEnd)
    {
        RETURN_FAILURE(KErrCorrupt);
    }
	else if(dataStart){
        if(!dataEnd)
            RETURN_FAILURE(KErrCorrupt);
		if(dataStart<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(dataEnd>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((dataStart-codeStart)&iPointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // data not aligned with respect to code
    }

	// check total data size isn't too bit...
	uint32_t totalDataSize = iHdr->iDataSize + iHdr->iBssSize;
	if(totalDataSize>0x7fff0000)
		RETURN_FAILURE(KErrNoMemory);

	// check iExportDirOffset and iExportDirCount specify region in code part...
	if(uint32_t(iHdr->iExportDirCount)>65535)
		RETURN_FAILURE(KErrCorrupt); // too many exports
	if(iHdr->iExportDirCount)
    {
		uint32_t exportsStart = iHdr->iExportDirOffset;
		uint32_t exportsEnd = exportsStart + iHdr->iExportDirCount*sizeof(uint32_t);
		if(iHdr->iFlags&KImageNmdExpData)
			exportsStart -= sizeof(uint32_t); // allow for 0th ordinal
		if(exportsEnd<exportsStart)
			RETURN_FAILURE(KErrCorrupt);
		if(exportsStart<codeStart)
			RETURN_FAILURE(KErrCorrupt);
		if(exportsEnd>codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if((exportsStart-codeStart)&iPointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within code section
    }

	// check iTextSize...
	if(iHdr->iTextSize>iHdr->iCodeSize)
		RETURN_FAILURE(KErrCorrupt);

	// check iImportOffset...
	uint32_t start = iHdr->iImportOffset;
	if(start)
    {
		uint32_t end = start+sizeof(E32ImportSection); // minimum valid size
		if(end<start)
			RETURN_FAILURE(KErrCorrupt);
		if(start<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(end>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((start-codeEnd)&iPointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within 'rest of data'
    }

	// check iCodeRelocOffset...
	start = iHdr->iCodeRelocOffset;
	if(start)
    {
		uint32_t end = start+sizeof(E32RelocSection); // minimum valid size
		if(end<start)
			RETURN_FAILURE(KErrCorrupt);
		if(start<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(end>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((start-codeEnd)&iPointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within 'rest of data'
    }

	// check iDataRelocOffset...
	start = iHdr->iDataRelocOffset;
	if(start)
    {
		uint32_t end = start+sizeof(E32RelocSection); // minimum valid size
		if(end<start)
			RETURN_FAILURE(KErrCorrupt);
		if(start<codeEnd)
			RETURN_FAILURE(KErrCorrupt);
		if(end>uncompressedSize)
			RETURN_FAILURE(KErrCorrupt);
		if((start-codeEnd)&iPointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // not aligned within 'rest of data'
    }

	// check exception descriptor...
	uint32_t excDesc = iParser->GetE32HdrV()->iExceptionDescriptor;
	if(excDesc&1) // if valid...
		if(excDesc>=iHdr->iCodeSize)
			RETURN_FAILURE(KErrCorrupt);

	int32_t r = ValidateExportDescription();
	if(r!=KErrNone)
		RETURN_FAILURE(r);

	// done...
	return KErrNone;
}

int32_t E32Validator::ValidateExportDescription() const
{
    // check export description...
    uint32_t edSize = iHdrV->iExportDescSize + sizeof(iHdrV->iExportDescSize) + sizeof(iHdrV->iExportDescType);
    edSize = (edSize+3)&~3;
    uint32_t edEnd = offsetof(E32ImageHeaderV,iExportDescSize) + edSize +
            sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ);
    if(edEnd != iHdr->iCodeOffset)
        RETURN_FAILURE(KErrCorrupt);

    // size of bitmap of exports...
    uint32_t bitmapSize = (iHdr->iExportDirCount+7) >> 3;

    // check export description bitmap...
    switch(iHdrV->iExportDescType)
    {
        case KImageHdr_ExpD_NoHoles:
            // no bitmap to check...
            E32IMAGEHEADER_TRACE(("ValidateExportDescription NoHoles\n"));
            return KErrNone;

        case KImageHdr_ExpD_FullBitmap:
            // full bitmap present...
            E32IMAGEHEADER_TRACE(("ValidateExportDescription FullBitmap\n"));
            if(bitmapSize!=iHdrV->iExportDescSize)
                RETURN_FAILURE(KErrCorrupt);
            return KErrNone;

        case KImageHdr_ExpD_SparseBitmap8: {
            // sparse bitmap present...
            E32IMAGEHEADER_TRACE(("ValidateExportDescription SparseBitmap8\n"));

            // get size of meta-bitmap...
            uint32_t metaBitmapSize = (bitmapSize+7) >> 3;
            if(metaBitmapSize>iHdrV->iExportDescSize)
                RETURN_FAILURE(KErrCorrupt); // doesn't fit

            uint32_t totalSize = metaBitmapSize;

            // scan meta-bitmap counting extra bytes which should be present...
            const uint8_t* metaBitmap = iHdrV->iExportDesc;
            const uint8_t* metaBitmapEnd = metaBitmap + metaBitmapSize;
            while(metaBitmap<metaBitmapEnd)
            {
                uint32_t bits = *metaBitmap++;
                do {
                    if(bits&1)
                        ++totalSize; // another byte is present in bitmap
                }
                while(bits>>=1);
            }

            if(totalSize!=iHdrV->iExportDescSize)
                RETURN_FAILURE(KErrCorrupt);
            return KErrNone;
            }

        default:
            E32IMAGEHEADER_TRACE(("ValidateExportDescription ?\n"));
            RETURN_FAILURE(KErrNotSupported);
    }
}

/** \brief Check consistency for relocations in E32Image
 *
 * \param offset Offset to reloc section.
       Should be E32ImageHeader::iCodeRelocOffset or E32ImageHeader::iDataRelocOffset.
 * \param sectionSize Size for relocations.
       Should be E32ImageHeader::iCodeSize or E32ImageHeader::iDataSize.
 * \return KErrNone if relocation is valid, else KErrCorrupt.
 *
 */

int32_t E32Validator::ValidateRelocations(uint32_t offset, uint32_t sectionSize)
{
    if(!offset)
		return KErrNone; // no relocations

    // read section header (ValidateHeader has alread checked this is OK)...
	E32RelocSection* sectionHeader = iParser->GetRelocSection(offset);
	int32_t size = sectionHeader->iSize;
	int32_t relocsRemaining = sectionHeader->iNumberOfRelocs;
	E32IMAGEHEADER_TRACE(("E32RelocSection 0x%x %d\n",size,relocsRemaining));
	if(size&3)
		RETURN_FAILURE(KErrCorrupt); // not multiple of word size

	// calculate buffer range for block data...
	uint8_t* p = (uint8_t*)(sectionHeader+1);  // start of first block
	uint8_t* sectionEnd = p+size;
	uint8_t* bufferEnd = p + (iBufSize - iHdr->iCodeSize) - 1;

	if(sectionEnd<p)
		RETURN_FAILURE(KErrCorrupt); // math overflow

	if(sectionEnd>bufferEnd)
		RETURN_FAILURE(KErrCorrupt); // overflows buffer

    // process each block...
	while(p!=sectionEnd)
    {
		E32RelocBlock* block = (E32RelocBlock*)p;

		// get address of first entry in this block...
		uint16_t* entryPtr = (uint16_t*)(block+1);
		if((uint8_t*)entryPtr<(uint8_t*)block || (uint8_t*)entryPtr>sectionEnd)
			RETURN_FAILURE(KErrCorrupt);  // overflows relocation section

		// read block header...
		uint32_t pageOffset = block->iPageOffset;
		uint32_t blockSize = block->iBlockSize;
		E32IMAGEHEADER_TRACE(("E32RelocSection block offset: 0x%x\t size: 0x%x\n",pageOffset,blockSize));
		if(pageOffset&0xfff)
			RETURN_FAILURE(KErrCorrupt); // not page aligned
		if(blockSize<sizeof(E32RelocBlock))
			RETURN_FAILURE(KErrCorrupt); // blockSize must be at least that of the header just read
		if(blockSize&3)
			RETURN_FAILURE(KErrCorrupt); // not word aligned

		// caculate end of entries in this block...
		uint16_t* entryEnd = (uint16_t*)(p+blockSize);
		if(entryEnd<entryPtr)
			RETURN_FAILURE(KErrCorrupt); // math overflow
		if(entryEnd>(uint16_t*)sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // overflows relocation section

		// process each entry in this block...
		while(entryPtr<entryEnd)
        {
			uint32_t entry = *entryPtr++;
//			E32IMAGEHEADER_TRACE(("E32RelocSection entry 0x%04x\n",entry));
			if(!entry)
				continue;

			// check relocation type...
			uint32_t entryType = entry&0xf000;
			if(entryType!=KTextRelocType && entryType!=KDataRelocType && entryType!=KInferredRelocType)
				RETURN_FAILURE(KErrCorrupt);

			// check relocation is within section being relocated...
			uint32_t offset = pageOffset+(entry&0x0fff);
			if(offset>=sectionSize || offset+4>sectionSize)
				RETURN_FAILURE(KErrCorrupt); // not within section
			if(offset&iPointerAlignMask)
				RETURN_FAILURE(KErrCorrupt); // not aligned correctly

			// count each relocation processed...
			--relocsRemaining;
        }

		// next sub block...
		p = (uint8_t*)entryEnd;
    }

	// check number of relocations in section header is correct...
	E32IMAGEHEADER_TRACE(("E32RelocSection relocsRemaining=%d\n",relocsRemaining));
	if(relocsRemaining)
		RETURN_FAILURE(KErrCorrupt); // incorrect number of entries

	return KErrNone;
}

int32_t E32Validator::ValidateImports() const
{
    if(!iHdr->iImportOffset)
		return KErrNone; // no imports

    // buffer pointer to read relocation from...
    uint8_t* buf = (uint8_t*)iParser->GetBufferedImage();
	uint8_t* bufferEnd = buf + iBufSize; //last byte of E32Image

    // read section header (ValidateHeader has alread checked this is OK)...
	E32ImportSection* sectionHeader = iParser->GetImportSection();
	E32IMAGEHEADER_TRACE(("E32ImportSection 0x%x\n", sectionHeader->iSize));

	// check section lies within buffer...
	uint8_t* sectionStart = (uint8_t*)sectionHeader; // start of first import block
	uint8_t* sectionEnd = sectionStart + sectionHeader->iSize;
	if( sectionEnd < sectionStart )
		RETURN_FAILURE(KErrCorrupt); // math overflow or not big enough to contain header
	if(sectionEnd > bufferEnd)
		RETURN_FAILURE(KErrCorrupt); // overflows buffer

	// process each import block...
	uint32_t numDeps = iHdr->iDllRefTableCount;
	uint32_t biggestImportCount = 0;
	uint32_t totalImports = 0;
	uint32_t importFormat = iHdr->iFlags&KImageImpFmtMask;

	uint8_t* p = (uint8_t*)(sectionHeader + 1);
    while(numDeps--)
    {
		// get block header...
		const E32ImportBlock* block = (E32ImportBlock*)p;
		p = (uint8_t*)(block+1);
		if(p<(uint8_t*)block || p>sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // overflows buffer

		E32IMAGEHEADER_TRACE(("E32ImportBlock 0x%x %d",block->iOffsetOfDllName,block->iNumberOfImports));

		// check import dll name is within section...
		uint8_t* name = sectionStart+block->iOffsetOfDllName;
		if(name<sectionStart || name>=sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // not within import section
		while(*name++ && name<sectionEnd)
			{}
		if(name[-1])
			RETURN_FAILURE(KErrCorrupt); // name overflows section
		E32IMAGEHEADER_TRACE(("E32ImportBlock %s",sectionStart+block->iOffsetOfDllName));

		// process import count...
		uint32_t numberOfImports = block->iNumberOfImports;
		if(numberOfImports>=0x80000000u/sizeof(uint32_t))
			RETURN_FAILURE(KErrCorrupt); // size doesn't fit into a signed integer
		if(numberOfImports>biggestImportCount)
			biggestImportCount = numberOfImports;
		totalImports += numberOfImports;

		// process import data...

		// PE2 doesn't have any more data...
		if(importFormat==KImageImpFmt_PE2)
			continue;

        // get import data range...
		uint32_t* imports = (uint32_t*)p;
		uint32_t* importsEnd = imports+numberOfImports;
		if(importsEnd<imports)
			RETURN_FAILURE(KErrCorrupt); // math overflow. Fuzzer can't trigger this because needs buf to be in to be >0x80000000
		if(importsEnd>(uint32_t*)sectionEnd)
			RETURN_FAILURE(KErrCorrupt); // overflows buffer

		if(importFormat==KImageImpFmt_ELF)
        {
			// check imports are in code section...
			uint32_t limit = iHdr->iCodeSize-sizeof(uint32_t);
			while(imports<importsEnd)
            {
				uint32_t i = *imports++;
				if(i>limit)
					RETURN_FAILURE(KErrCorrupt);
				if(i&iPointerAlignMask)
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

		// move pointer on to next block...
		p = (uint8_t*)block->NextBlock(importFormat);
    }

	// done processing imports; for PE derived files now check import address table (IAT)...
	if(importFormat==KImageImpFmt_PE || importFormat==KImageImpFmt_PE2)
    {
		if(totalImports>=0x80000000u/sizeof(uint32_t))
			RETURN_FAILURE(KErrCorrupt); // size doesn't fit into a signed integer
		uint32_t iat = iHdr->iTextSize; // import address table
		if(iat&iPointerAlignMask)
			RETURN_FAILURE(KErrCorrupt); // Fuzzer can't trigger this because PE imports are for X86 which doesn't have alignment restrictions
		uint32_t iatEnd = iat+sizeof(uint32_t)*totalImports;
		if(iatEnd<iat || iatEnd>iHdr->iCodeSize)
			RETURN_FAILURE(KErrCorrupt); // import address table overflows code part of file
		E32IMAGEHEADER_TRACE(("E32ImportSection IAT offsets 0x%x..0x%x", iat, iatEnd));
    }

	return KErrNone;
}

