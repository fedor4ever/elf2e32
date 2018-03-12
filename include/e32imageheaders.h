/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* Copyright (c) 2017-2018 Strizhniou Fiodar
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors: Strizhniou Fiodar - fix build and runtime errors.
*
* Description:
*
*/



#ifndef __LAUNCHERE32IMAGEHEADERS_H__
#define __LAUNCHERE32IMAGEHEADERS_H__

#include <portable.h>


enum TCpu
	{
	ECpuUnknown=0, ECpuX86=0x1000, ECpuArmV4=0x2000, ECpuArmV5=0x2001, ECpuArmV6=0x2002, ECpuMCore=0x4000
	};

const TInt KOrdinalBase=1;
const TUint KImageDll				= 0x00000001u;
const TUint KImageNoCallEntryPoint	= 0x00000002u;
const TUint KImageFixedAddressExe	= 0x00000004u;
const TUint KImageOldJFlag			= 0x00000008u;	// so we can run binaries built with pre 2.00 tools (hdrfmt=0)
const TUint KImageOldElfFlag		= 0x00000010u;	// so we can run binaries built with pre 2.00 tools (hdrfmt=0)
const TUint KImageABIMask			= 0x00000018u;	// only if hdr fmt not zero
const TInt	KImageABIShift			= 3;
const TUint	KImageABI_GCC98r2		= 0x00000000u;	// for ARM
const TUint	KImageABI_EABI			= 0x00000008u;	// for ARM
const TUint KImageEptMask			= 0x000000e0u;	// entry point type
const TInt	KImageEptShift			= 5;
const TUint KImageEpt_Eka1			= 0x00000000u;
const TUint KImageEpt_Eka2			= 0x00000020u;
const TUint KImageHdrFmtMask		= 0x0f000000u;
const TInt	KImageHdrFmtShift		= 24;
const TUint KImageHdrFmt_Original	= 0x00000000u;	// without compression support
const TUint KImageHdrFmt_J			= 0x01000000u;	// with compression support
const TUint KImageHdrFmt_V			= 0x02000000u;	// with versioning support
const TUint KImageImpFmtMask		= 0xf0000000u;
const TInt	KImageImpFmtShift		= 28;
const TUint KImageImpFmt_PE			= 0x00000000u;	// PE-derived imports
const TUint KImageImpFmt_ELF		= 0x10000000u;	// ELF-derived imports
const TUint KImageImpFmt_PE2		= 0x20000000u;	// PE-derived imports without redundant copy of import ordinals
const TUint KImageHWFloatMask		= 0x00f00000u;
const TInt	KImageHWFloatShift		= 20;
const TUint	KImageHWFloat_None		= EFpTypeNone << KImageHWFloatShift; // No hardware floating point used
const TUint KImageHWFloat_VFPv2		= EFpTypeVFPv2 << KImageHWFloatShift; // ARM VFPv2 floating point used

const TUint KMyFormatNotCompressed=0;
const TUint KMyUidCompressionDeflate=0x101F7AFC;

const TUint32 KImageCrcInitialiser	= 0xc90fdaa2u;
/*
const TUint16 KReservedRelocType        = (TUint16)0x0000;
const TUint16 KTextRelocType            = (TUint16)0x1000;
const TUint16 KDataRelocType            = (TUint16)0x2000;
const TUint16 KInferredRelocType        = (TUint16)0x3000;
*/


struct SSecurityInfo {
    TUint32 iSecureId;
    TUint32 iVendorId;
    SCapabilitySet iCaps;   // Capabilities re. platform security
    };

class E32ImageHeader
	{
public:
	inline static TUint ABIFromFlags(TUint aFlags)
		{
		if (aFlags&KImageHdrFmtMask)
			return aFlags & KImageABIMask;
		if (aFlags&KImageOldElfFlag)
			return KImageABI_EABI;
		return KImageABI_GCC98r2;
		}
/**
Extract entrypoint format from aFlags.
*/
	inline static TUint EptFromFlags(TUint aFlags)
		{
		if (aFlags&KImageHdrFmtMask)
			return aFlags & KImageEptMask;
		if (aFlags&KImageOldJFlag)
			return KImageEpt_Eka2;
		return KImageEpt_Eka1;
		}
	inline static TUint HdrFmtFromFlags(TUint aFlags)
		{
		if (aFlags&KImageHdrFmtMask)
			return aFlags & KImageHdrFmtMask;
		if (aFlags&KImageOldJFlag)
			return KImageHdrFmt_J;
		return KImageHdrFmt_Original;
		}
	inline static TUint ImpFmtFromFlags(TUint aFlags)
		{
		if (aFlags&KImageHdrFmtMask)
			return aFlags & KImageImpFmtMask;
		if (aFlags&KImageOldElfFlag)
			return KImageImpFmt_ELF;
		return KImageImpFmt_PE;
		}
	inline TUint32 CompressionType() const
		{
		if (HdrFmtFromFlags(iFlags) >= KImageHdrFmt_J)
			return iCompressionType;
		return 0;
		}
	inline TUint32 ModuleVersion() const
		{
		if ((iFlags & KImageHdrFmtMask) >= KImageHdrFmt_V)
			return iModuleVersion;
		return 0x00000000u;
		}
	inline TInt TotalSize() const;
/**
	Return total size of file after decompression, or -1 if file not compressed.
*/
	inline TInt UncompressedFileSize() const;
	inline TUint HeaderFormat() const
		{ return HdrFmtFromFlags(iFlags); }
	inline TUint EntryPointFormat() const
		{ return EptFromFlags(iFlags); }
	inline TUint ImportFormat() const
		{ return ImpFmtFromFlags(iFlags); }
	inline TUint ABI() const
		{ return ABIFromFlags(iFlags); }
	inline void GetSecurityInfo(SSecurityInfo& aInfo) const;
	inline TCpu CpuIdentifier() const;
/**
	Return #iProcessPriority.
*/
	inline TProcessPriority ProcessPriority() const;
	inline TUint32 ExceptionDescriptor() const;
	TInt IntegrityCheck(TInt aFileSize);
	//inline TInt ValidateHeader(TInt aFileSize, TUint32& aUncompressedSize) const;
public:
	TUint32	iUid1;
	TUint32	iUid2;
	TUint32	iUid3;
	TUint32 iUidChecksum;
	TUint iSignature;			// 'EPOC'
	TUint32	iHeaderCrc;			// CRC-32 of entire header
	TUint32 iModuleVersion;		// Version number for this executable (used in link resolution)
	TUint32 iCompressionType;	// Type of compression used (UID or 0 for none)
	TVersion iToolsVersion;		// Version of PETRAN/ELFTRAN which generated this file
	TUint32 iTimeLo;
	TUint32 iTimeHi;
	TUint iFlags;				// 0 = exe, 1 = dll, 2 = fixed address exe
	TInt iCodeSize;				// size of code, import address table, constant data and export dir
	TInt iDataSize;				// size of initialised data
	TInt iHeapSizeMin;
	TInt iHeapSizeMax;
	TInt iStackSize;
	TInt iBssSize;
	TUint iEntryPoint;			// offset into code of entry point
	TUint iCodeBase;			// where the code is linked for
	TUint iDataBase;			// where the data is linked for
	TInt iDllRefTableCount;		// filling this in enables E32ROM to leave space for it
	TUint iExportDirOffset;		// offset into the file of the export address table
	TInt iExportDirCount;
	TInt iTextSize;				// size of just the text section, also doubles as the offset for the iat w.r.t. the code section
	TUint iCodeOffset;			// file offset to code section, also doubles as header size
	TUint iDataOffset;			// file offset to data section
	TUint iImportOffset;		// file offset to import section
	TUint iCodeRelocOffset;		// relocations for code and const
	TUint iDataRelocOffset;		// relocations for data
	TUint16 iProcessPriority;	// executables priority
	TUint16 iCpuIdentifier;		// 0x1000 = X86, 0x2000 = ARM
	};

class E32ImageHeaderV : public E32ImageHeader
	{
public:
	SSecurityInfo iS;

	// Use iSpare1 as offset to Exception Descriptor
	TUint32 iExceptionDescriptor;   // Offset in bytes from start of code section to Exception Descriptor, bit 0 set if valid
	TUint32 iSpare2;
	TUint16	iExportDescSize;	// size of bitmap section
	TUint8	iExportDescType;	// type of description of holes in export table
	TUint8	iExportDesc[1];		// description of holes in export table - extend
	TUint32 iUncompressedSize;	// Uncompressed size of file
								// For J format this is file size - sizeof(E32ImageHeader)
								//  and this is included as part of the compressed data :-(
								// For other formats this is file size - total header size
	};

// export description type E32ImageHeaderV::iExportDescType
const TUint	KImageHdr_ExpD_NoHoles			=0x00;	///< no holes, all exports present
const TUint	KImageHdr_ExpD_FullBitmap		=0x01;	///< full bitmap present at E32ImageHeaderV::iExportDesc
const TUint	KImageHdr_ExpD_SparseBitmap8	=0x02;	///< sparse bitmap present at E32ImageHeaderV::iExportDesc, granularity 8
const TUint	KImageHdr_ExpD_Xip				=0xff;	///< XIP file



inline TInt E32ImageHeader::TotalSize() const
	{
	if (HeaderFormat() == KImageHdrFmt_J && iCompressionType != 0)
		return sizeof(E32ImageHeaderComp);
	return iCodeOffset;
	}

inline TInt E32ImageHeader::UncompressedFileSize() const
	{
	TUint hdrfmt = HeaderFormat();
	if (hdrfmt == KImageHdrFmt_Original || iCompressionType == 0)
		return -1;			// not compressed
	else if (hdrfmt == KImageHdrFmt_J)
		return ((E32ImageHeaderComp*)this)->iUncompressedSize + sizeof(E32ImageHeader);
	else
		return ((E32ImageHeaderComp*)this)->iUncompressedSize + iCodeOffset;
	}

extern const SSecurityInfo KDefaultSecurityInfo;
inline void E32ImageHeader::GetSecurityInfo(SSecurityInfo& aInfo) const
	{
	if (HeaderFormat() >= KImageHdrFmt_V)
		aInfo = ((E32ImageHeaderV*)this)->iS;
	else
		aInfo = KDefaultSecurityInfo;
	}

inline TCpu E32ImageHeader::CpuIdentifier() const
	{
	if (HeaderFormat() >= KImageHdrFmt_V)
		return (TCpu)iCpuIdentifier;
	return (TCpu)iHeaderCrc;
	}

inline TProcessPriority E32ImageHeader::ProcessPriority() const
	{
	if (HeaderFormat() >= KImageHdrFmt_V)
		return (TProcessPriority)iProcessPriority;
	return *(const TProcessPriority*)&iProcessPriority;
	}

inline TUint32 E32ImageHeader::ExceptionDescriptor() const
	{
	if (HeaderFormat() >= KImageHdrFmt_V)
		{
		TUint32 xd = ((E32ImageHeaderV*)this)->iExceptionDescriptor;
		if ((xd & 1) && (xd != 0xffffffffu))
			return (xd & ~1);
		}
	return 0;
	}

/* TInt E32ImageHeader::ValidateHeader(TInt aFileSize, TUint32& aUncompressedSize) const {
    // check file is big enough for any header...
    if(TUint(aFileSize)<sizeof(*this)) return KErrCorrupt;
    TUint hdrfmt = HeaderFormat();
    if(hdrfmt==KImageHdrFmt_V)
        return ((E32ImageHeaderV*)this)->ValidateHeader(aFileSize,aUncompressedSize);

    return KErrNotSupported; // header format unrecognised
    } */

/**
A block of imports from a single executable.
These structures are conatined in a images Import Section (E32ImportSection).
*/
class E32ImportBlock
	{
public:
	inline const E32ImportBlock* NextBlock(TUint aImpFmt) const;
	inline TInt Size(TUint aImpFmt) const;
	inline const TUint* Imports() const;	// import list if present
public:
	TUint32	iOffsetOfDllName;	///< Offset from start of import section for a NUL terminated executable (DLL or EXE) name.
	TUint	iNumberOfImports;	///< Number of imports from this executable.
//	TUint	iImport[iNumberOfImports];	// list of imported ordinals, omitted in PE2 import format
	};

/**
Return size of this import block.
@param aImpFmt Import format as obtained from image header.
*/
inline TInt E32ImportBlock::Size(TUint aImpFmt) const
	{
	TInt r = sizeof(E32ImportBlock);
	if (aImpFmt!=KImageImpFmt_PE2)
		r += iNumberOfImports * sizeof(TUint);
	return r;
	}

/**
Return pointer to import block which immediately follows this one.
@param aImpFmt Import format as obtained from image header.
*/
inline const E32ImportBlock* E32ImportBlock::NextBlock(TUint aImpFmt) const
	{
	const E32ImportBlock* next = this + 1;
	if (aImpFmt!=KImageImpFmt_PE2)
		next = (const E32ImportBlock*)( (TUint8*)next + iNumberOfImports * sizeof(TUint) );
	return next;
	}

/**
Return address of first import in this block.
For import format KImageImpFmt_ELF, imports are list of code section offsets.
For import format KImageImpFmt_PE, imports are a list of imported ordinals.
For import format KImageImpFmt_PE2, the import list is not present and should not be accessed.
*/
inline const TUint* E32ImportBlock::Imports() const
	{
	return (const TUint*)(this + 1);
	}


#endif // __LAUNCHERE32IMAGEHEADERS_H__
