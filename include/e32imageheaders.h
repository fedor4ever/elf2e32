/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/



#ifndef __LAUNCHERE32IMAGEHEADERS_H__
#define __LAUNCHERE32IMAGEHEADERS_H__

#include <portable.h>


/** Bit input stream. Good for reading bit streams for packed, compressed or huffman
	data algorithms.
*/
class TBitInput
	{
public:
	IMPORT_C TBitInput();
	IMPORT_C TBitInput(const TUint8* aPtr, TInt aLength, TInt aOffset=0);
	IMPORT_C void Set(const TUint8* aPtr, TInt aLength, TInt aOffset=0);
//
	IMPORT_C TUint ReadL();
	IMPORT_C TUint ReadL(TInt aSize);
	IMPORT_C TUint HuffmanL(const TUint32* aTree);
private:
	virtual void UnderflowL();
private:
	TInt iCount;
	TUint iBits;
	TInt iRemain;
	const TUint32* iPtr;
	};

/** Huffman code toolkit.

	This class builds a huffman encoding from a frequency table and builds
	a decoding tree from a code-lengths table

	The encoding generated is based on the rule that given two symbols s1 and s2, with
	code length l1 and l2, and huffman codes h1 and h2:

		if l1<l2 then h1<h2 when compared lexicographically
		if l1==l2 and s1<s2 then h1<h2 ditto

	This allows the encoding to be stored compactly as a table of code lengths
*/
class Huffman
	{
public:
	enum {KMaxCodeLength=27};
	enum {KMetaCodes=KMaxCodeLength+1};
	enum {KMaxCodes=0x8000};
public:
	IMPORT_C static void HuffmanL(const TUint32 aFrequency[],TInt aNumCodes,TUint32 aHuffman[]);
	IMPORT_C static void Encoding(const TUint32 aHuffman[],TInt aNumCodes,TUint32 aEncodeTable[]);
	IMPORT_C static void Decoding(const TUint32 aHuffman[],TInt aNumCodes,TUint32 aDecodeTree[],TInt aSymbolBase=0);
	IMPORT_C static TBool IsValid(const TUint32 aHuffman[],TInt aNumCodes);
//
	IMPORT_C static void ExternalizeL(TBitOutput& aOutput,const TUint32 aHuffman[],TInt aNumCodes);
	IMPORT_C static void InternalizeL(TBitInput& aInput,TUint32 aHuffman[],TInt aNumCodes);
	};


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
class RFile;
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
	inline TProcessPriority ProcessPriority() const;
	inline TUint32 ExceptionDescriptor() const;
	TInt IntegrityCheck(TInt aFileSize);
	E32ImageHeader(E32ImageHeader*& aHdr, RFile& aFile);
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

// export description type
const TUint	KImageHdr_ExpD_NoHoles			=0x00;	// no holes, all exports present
const TUint	KImageHdr_ExpD_FullBitmap		=0x01;	// full bitmap present
const TUint	KImageHdr_ExpD_SparseBitmap8	=0x02;	// sparse bitmap present, granularity 8
const TUint	KImageHdr_ExpD_Xip				=0xff;	// XIP file


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

class E32ImportBlock
	{
public:
	inline const E32ImportBlock* NextBlock(TUint aImpFmt) const;
	inline TInt Size(TUint aImpFmt) const;
	inline const TUint* Imports() const;	// import list if present
public:
	TUint32	iOffsetOfDllName;	// offset of name of dll importing from
	TInt	iNumberOfImports;	// no of imports from this dll
//	TUint	iImport[iNumberOfImports];	// list of imported ordinals, omitted in PE2 import format
	};

inline TInt E32ImportBlock::Size(TUint aImpFmt) const
	{
	TInt r = sizeof(E32ImportBlock);
	if (aImpFmt!=KImageImpFmt_PE2)
		r += iNumberOfImports * sizeof(TUint);
	return r;
	}

inline const E32ImportBlock* E32ImportBlock::NextBlock(TUint aImpFmt) const
	{
	const E32ImportBlock* next = this + 1;
	if (aImpFmt!=KImageImpFmt_PE2)
		next = (const E32ImportBlock*)( (TUint8*)next + iNumberOfImports * sizeof(TUint) );
	return next;
	}

inline const TUint* E32ImportBlock::Imports() const
	{
	return (const TUint*)(this + 1);
	}

class E32ImportSection
	{
public:
	TInt iSize;					// size of this section
//	E32ImportBlock[iDllRefTableCount];
	};

class E32RelocSection
	{
public:
	TInt iSize;					// size of this relocation section
	TInt iNumberOfRelocs;		// number of relocations in this section
	};


typedef TUint8* (*TMemoryMoveFunction)(TAny* aTrg,const TAny* aSrc,TInt aLength);

const TInt KDeflateLengthMag=8;
const TInt KDeflateDistanceMag=12;
const TInt KDeflateMinLength=3;
const TInt KDeflateMaxLength=KDeflateMinLength-1 + (1<<KDeflateLengthMag);
const TInt KDeflateMaxDistance=(1<<KDeflateDistanceMag);
const TInt KDeflateDistCodeBase=0x200;
const TUint KDeflateHashMultiplier=0xAC4B9B19u;
const TInt KDeflateHashShift=24;
const TInt KInflateWindowSize=0x8000;


class TEncoding
	{
public:
	enum {ELiterals=256,ELengths=(KDeflateLengthMag-1)*4,ESpecials=1,EDistances=(KDeflateDistanceMag-1)*4};
	enum {ELitLens=ELiterals+ELengths+ESpecials};
	enum {EEos=ELiterals+ELengths};
public:
	TUint32 iLitLen[ELitLens];
	TUint32 iDistance[EDistances];
	};

const TInt KDeflationCodes=TEncoding::ELitLens+TEncoding::EDistances;

NONSHARABLE_CLASS(CInflater) : public CBase
	{
public:
	enum {EBufSize = 0x800, ESafetyZone=8};
public:
	static CInflater* NewLC(TBitInput& aInput);
	~CInflater();
	TInt ReadL(TUint8* aBuffer,TInt aLength, TMemoryMoveFunction aMemMovefn);
	TInt SkipL(TInt aLength);
private:
	CInflater(TBitInput& aInput);
	void ConstructL();
	void InitL();
	TInt InflateL();
private:
	TBitInput* iBits;
	const TUint8* iRptr;
	TInt iLen;
	const TUint8* iAvail;
	const TUint8* iLimit;
	TEncoding* iEncoding;
	TUint8* iOut;
	};

void DeflateL(const TUint8* aBuf, TInt aLength, TBitOutput& aOutput);


NONSHARABLE_CLASS(TFileInput) : public TBitInput
	{
 	enum {KBufSize=KInflateWindowSize};
public:
	TFileInput(RFile& aFile);
	void Cancel();
private:
	void UnderflowL();
private:
	RFile& iFile;
	TRequestStatus iStat;
	TUint8* iReadBuf;
	TPtr8 iPtr;
	TUint8 iBuf1[KBufSize];
	TUint8 iBuf2[KBufSize];
	};

class TFileNameInfo
	{
public:
	enum
    	{
    	EIncludeDrive=1,
    	EIncludePath=2,
    	EIncludeBase=4,
    	EIncludeVer=8,
    	EForceVer=16,
    	EIncludeUid=32,
    	EForceUid=64,
    	EIncludeExt=128,
    	EIncludeDrivePath=EIncludeDrive|EIncludePath,
    	EIncludeBaseExt=EIncludeBase|EIncludeExt,
    	EIncludeDrivePathBaseExt=EIncludeDrive|EIncludePath|EIncludeBase|EIncludeExt,
    	};
	enum
		{
		EAllowUid=1,
		EAllowPlaceholder=2,
		EAllowDecimalVersion=4,
		};
public:
	TFileNameInfo();
	TInt Set(const TDesC8& aFileName, TUint aFlags);
	void Dump() const;
public:
	inline TInt DriveLen() const {return iPathPos;}
	inline TInt PathLen() const {return iBasePos-iPathPos;}
	inline TInt BaseLen() const {return iVerPos-iBasePos;}
	inline TInt VerLen() const {return iUidPos-iVerPos;}
	inline TInt UidLen() const {return iExtPos-iUidPos;}
	inline TInt ExtLen() const {return iLen-iExtPos;}
	inline TPtrC8 Drive() const {return TPtrC8(iName, iPathPos);}
	inline TPtrC8 Path() const {return TPtrC8(iName+iPathPos, iBasePos-iPathPos);}
	inline TPtrC8 DriveAndPath() const {return TPtrC8(iName, iBasePos);}
	inline TPtrC8 Base() const {return TPtrC8(iName+iBasePos, iVerPos-iBasePos);}
	inline TPtrC8 VerStr() const {return TPtrC8(iName+iVerPos, iUidPos-iVerPos);}
	inline TPtrC8 UidStr() const {return TPtrC8(iName+iUidPos, iExtPos-iUidPos);}
	inline TPtrC8 Ext() const {return TPtrC8(iName+iExtPos, iLen-iExtPos);}
	inline TUint32 Version() const {return iVersion;}
	inline TUint32 Uid() const {return iUid;}
	void GetName(TDes8& aName, TUint aFlags) const;
public:
	const TText8* iName;
	TInt iPathPos;
	TInt iBasePos;
	TInt iVerPos;
	TInt iUidPos;
	TInt iExtPos;
	TInt iLen;
	TUint32 iVersion;
	TUint32 iUid;
	};


#endif // __LAUNCHERE32IMAGEHEADERS_H__
