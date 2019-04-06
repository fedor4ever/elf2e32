// Copyright (c) 2018 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Contributors:
//
// Description:
// Implementation of the Class E32Parser for the elf2e32 tool
// @internalComponent
// @released
//
//

#include <fstream>
#include <cstring>
#include <cstdlib>

#include "message.h"
#include "e32common.h"
#include "e32parser.h"
#include "errorhandler.h"

using std::fstream;

int32_t Adjust(int32_t size);


/** \brief ctor for E32Parser class
 *
 * That class accepts filename if filebuf not specified
 *
 * \param filename - E32 image file for parsing
 * \param fileBuf - E32 image in memory for parsing
 *
 */
E32Parser::E32Parser(const char* filename, const char* fileBuf):
    iFileName(filename), iBufferedFile((char*)fileBuf)
{}

E32Parser::~E32Parser()
{
    if(iFileName)
        delete iBufferedFile;
}

void E32Parser::ReadFile()
{
    if(iBufferedFile)
        return;

    fstream fs(iFileName, fstream::binary | fstream::in);
    if(!fs)
        throw Elf2e32Error(FILEOPENERROR, iFileName);
    fs.seekg(0, fs.end);
    iE32Size = fs.tellg();
    fs.seekg(0, fs.beg);

    iBufferedFile = new char[iE32Size]();
    fs.read(iBufferedFile, iE32Size);
    fs.close();
}

/** \brief Init function for class
 *
 * Should be called after ctor and before other functions for read and analyze E32 image.
 *
 * \return E32ImageHeader* - pointer to first field in parsed file
 *
 */
E32ImageHeader* E32Parser::GetFileLayout()
{
    if(!iFileName && !iBufferedFile)
        return nullptr;

    if(iBufferedFile)
        iFileName = nullptr;

    ReadFile();

    if((iBufferedFile[1] == 'E')&&(iBufferedFile[2] == 'L')&&(iBufferedFile[3] == 'F'))
        throw Elf2e32Error(MISMATCHTARGET, iFileName);

    iHdr = (E32ImageHeader*)iBufferedFile;
    size_t pos = sizeof(E32ImageHeader);

/// TODO (Administrator#1#09/09/18): Stop here detection for pre-8 binaries
    iHdrJ = (E32ImageHeaderJ*)(iBufferedFile + pos);

    DecompressImage();
/// TODO (Administrator#1#09/11/18): Stop here detection for pre-9 binaries

    pos += sizeof(E32ImageHeaderJ);
    iHdrV = (E32ImageHeaderV*)(iBufferedFile + pos);

    ParseExportBitMap();
    return iHdr;
}

void InflateUnCompress(unsigned char* source, int sourcesize,unsigned char* dest, int destsize);
int DecompressPages(uint8_t* bytes, std::ifstream& is);

void E32Parser::DecompressImage()
{
    if(!iFileName || !iHdr->iCompressionType) // require file for decompression!!!
        return;

    uint32_t buf_size = iHdrJ->iUncompressedSize;
    iE32Size = Adjust(buf_size + iHdr->iCodeOffset);

    if(iHdr->iCompressionType == KUidCompressionDeflate)
    {
        char *decompress = new char[buf_size]();
        uint32_t destsize = buf_size;
        uint32_t remainder = iE32Size - iHdr->iCodeOffset;
        InflateUnCompress( (unsigned char*)(iBufferedFile+iHdr->iCodeOffset),
                          remainder, (unsigned char*)(decompress), destsize);

        char *newBuf = new char[iE32Size]();
        memcpy(newBuf, iBufferedFile, iHdr->iCodeOffset);
        memcpy(newBuf + iHdr->iCodeOffset, decompress, buf_size);
        memset(iBufferedFile, -1, iHdr->iCodeOffset); // will see if use old header
        delete iBufferedFile;
        iBufferedFile = nullptr;
        iBufferedFile = newBuf;

        if (destsize != buf_size)
            Message::GetInstance()->ReportMessage(WARNING, HUFFMANINCONSISTENTSIZEERROR);
    }
    else if(iHdr->iCompressionType ==KUidCompressionBytePair)
    {
        // that bytepair decompression doesn't support in-memory operation =(
        // so rereading file =(

        size_t offset = iHdr->iCodeOffset;
        size_t fullsize = iHdr->iCodeOffset + iHdrJ->iUncompressedSize;
        delete iBufferedFile;
        iBufferedFile = nullptr;
        iBufferedFile = new char[fullsize]();
        std::ifstream is(iFileName, std::ifstream::in | std::ifstream::binary);
        if(!is.is_open())
            throw Elf2e32Error(FILEOPENERROR, iFileName);

        is.read(iBufferedFile, offset);

        // Read and decompress code part of the image
        unsigned int uncompressedCodeSize = DecompressPages((uint8_t *)(iBufferedFile + offset), is);

		// Read and decompress data part of the image
		offset+=uncompressedCodeSize;
		unsigned int uncompressedDataSize = DecompressPages((uint8_t *)(iBufferedFile + offset), is);

		is.close();

		if (uncompressedCodeSize + uncompressedDataSize != buf_size)
			Message::GetInstance()->ReportMessage(WARNING, BYTEPAIRINCONSISTENTSIZEERROR);
    }
    else
        throw Elf2e32Error(UNKNOWNCOMPRESSION);

    iHdr = (E32ImageHeader*)iBufferedFile;
    size_t pos = sizeof(E32ImageHeader);
    iHdrJ = (E32ImageHeaderJ*)(iBufferedFile + pos);
}

TExceptionDescriptor* E32Parser::GetExceptionDescriptor() const
{
    uint32_t xd = iHdrV->iExceptionDescriptor;
    xd &= ~1;
    return (TExceptionDescriptor *)(iBufferedFile + iHdr->iCodeOffset + xd);
}

E32ImageHeaderJ* E32Parser::GetE32HdrJ() const
{
    return iHdrJ;
}

E32ImageHeaderV* E32Parser::GetE32HdrV() const
{
    return iHdrV;
}

char* E32Parser::GetBufferedImage() const
{
    return iBufferedFile;
}


uint32_t HdrFmtFromFlags(uint32_t aFlags)
{
    if (aFlags&KImageHdrFmtMask)
        return aFlags & KImageHdrFmtMask;
    if (aFlags&KImageOldJFlag)
        return KImageHdrFmt_J;
    return KImageHdrFmt_Original;
}

uint32_t ImpFmtFromFlags(uint32_t aFlags)
{
    return aFlags & KImageImpFmtMask;
}

int32_t E32Parser::UncompressedFileSize() const
{
    if(iHdr->iCompressionType==0)
        return -1;          // not compressed
    else
        return iHdrJ->iUncompressedSize + iHdr->iCodeOffset;
}

size_t RoundUp(size_t value, size_t rounding)
{
    size_t res = value%rounding;
    res = rounding - res;
    return res + value;
}

size_t E32Parser::GetFileSize() const
{
    return iE32Size;
}

E32RelocSection* E32Parser::GetRelocSection(uint32_t offSet)
{
    return (E32RelocSection*)(iBufferedFile + offSet);
}

E32ImportSection *E32Parser::GetImportSection() const
{
    return (E32ImportSection*)(iBufferedFile + iHdr->iImportOffset);
}

char* E32Parser::GetImportAddressTable() const
{
    return (iBufferedFile + iHdr->iCodeOffset + iHdr->iTextSize);
}

char* E32Parser::GetDLLName(uint32_t OffsetOfDllName) const
{
    return (iBufferedFile + iHdr->iImportOffset + OffsetOfDllName);
}

E32EpocExpSymInfoHdr* E32Parser::GetEpocExpSymInfoHdr() const
{
    uint32_t* expTable = (uint32_t*)(iBufferedFile + iHdr->iExportDirOffset);
    uint32_t* zeroethOrd = expTable - 1;
    return (E32EpocExpSymInfoHdr*)(iBufferedFile + iHdr->iCodeOffset + *zeroethOrd - iHdr->iCodeBase);
}

uint32_t E32Parser::BSSOffset()
{
    return iHdr->iCodeSize + iHdr->iDataSize;
}

/**
This function creates the export description after reading the E32 image file
@internalComponent
@released
*/
int32_t E32Parser::GetExportDescription()
{
	uint32_t fm = HdrFmtFromFlags(iHdr->iFlags);
	if (fm < KImageHdrFmt_V && iMissingExports)
		return KErrCorrupt;

    if(!iHdrV)
        iHdrV = GetE32HdrV();

	if (iHdrV->iExportDescType == KImageHdr_ExpD_NoHoles)
		return iMissingExports ? KErrCorrupt : KErrNone;

	int32_t nexp = iHdr->iExportDirCount;
	int32_t memsz = (nexp + 7) >> 3;	// size of complete bitmap
	int32_t mbs = (memsz + 7) >> 3;	// size of meta-bitmap
	int32_t eds = iHdrV->iExportDescSize;

	if (iHdrV->iExportDescType == KImageHdr_ExpD_FullBitmap)
	{
		if (eds != memsz)
			return KErrCorrupt;
		if (memcmp(iHdrV->iExportDesc, iExportBitMap, eds) == 0)
			return KErrNone;
		return KErrCorrupt;
	}

	if (iHdrV->iExportDescType != KImageHdr_ExpD_SparseBitmap8)
		return KErrNotSupported;

	int32_t nbytes = 0;
	for (int32_t i=0; i<memsz; ++i)
		if (iExportBitMap[i] != 0xff)
			++nbytes;				// number of groups of 8

	int32_t exp_extra = mbs + nbytes;
	if (eds != exp_extra)
		return KErrCorrupt;

	const uint8_t* mptr = iHdrV->iExportDesc;
	const uint8_t* gptr = mptr + mbs;
	for (int32_t i=0; i<memsz; ++i)
	{
		uint32_t mbit = mptr[i>>3] & (1u << (i&7));
		if (iExportBitMap[i] != 0xff)
		{
			if (!mbit || *gptr++ != iExportBitMap[i])
				return KErrCorrupt;
		}
		else if (mbit)
			return KErrCorrupt;
	}

	return KErrNone;
}

void E32Parser::ParseExportBitMap()
{
	int32_t nexp = iHdr->iExportDirCount;
	int32_t memsz = (nexp + 7) >> 3;
	iExportBitMap = new uint8_t[memsz];
	memset(iExportBitMap, 0xff, memsz);
	uint32_t* exports = (uint32_t*)(iBufferedFile + iHdr->iExportDirOffset);
	uint32_t absoluteEntryPoint = iHdr->iEntryPoint + iHdr->iCodeBase;
	uint32_t impfmt = ImpFmtFromFlags(iHdr->iFlags);
	uint32_t hdrfmt = HdrFmtFromFlags(iHdr->iFlags);
	uint32_t absentVal = (impfmt == KImageImpFmt_ELF) ? absoluteEntryPoint : iHdr->iEntryPoint;

	iMissingExports = 0;

	for (int32_t i=0; i<nexp; i++)
	{
		if (exports[i] == absentVal)
		{
//			iExportBitMap[i>>3] &= ~(1u << (i & 7));
			++iMissingExports;
		}
	}

	if (hdrfmt < KImageHdrFmt_V && iMissingExports)
	{
		fprintf(stderr, "Bad exports\n");
		exit(999);
	}
}

int32_t Adjust(int32_t size)
{
    return ((size+0x3)&0xfffffffc);
}
