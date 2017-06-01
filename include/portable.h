#ifndef __portable__
#define __portable__

#include <cstddef>
#include <cstdio>
#include <e32capability.h>
#include <cstdint>

static_assert(sizeof(char) == 1, "Wrong char size! Must be 1!");

#define __NO_THROW throw()

#define E32IMAGEHEADER_TRACE(_t)

#ifndef RETURN_FAILURE
#define RETURN_FAILURE(_r) return (fprintf(stderr, "line %d\n", __LINE__),_r)
#endif // RETURN_FAILURE

typedef unsigned char TText8;
typedef int32_t		TInt;
typedef uint32_t	TUint;
typedef uint64_t	Int64;
typedef signed char TInt8;
typedef unsigned char TUint8;
typedef int16_t		TInt16;
typedef uint16_t	TUint16;
typedef int32_t		TInt32;
typedef uint32_t	TUint32;
typedef uint64_t	Uint64;
typedef Uint64		TUint64;

const TInt KErrNone=0;
const TInt KErrGeneral=(-2);
const TInt KErrNoMemory=(-4);
const TInt KErrNotSupported=(-5);
const TInt KErrCorrupt=(-20);
const TInt KErrTooBig=(-40);
//#include <e32const.h>
const TInt KMaxCheckedUid=3;
const TInt KSizeOf_TUID=4; // taken on emulator

const TUint KImageDebuggable  = 0x00000800u; ///< Flag to indicate image is debuggable
const TUint KImageSMPSafe   = 0x00004000u; ///< Flag to indicate image is SMP safe

// Relocation types
const TUint16 KReservedRelocType        = (TUint16)0x0000;
const TUint16 KTextRelocType            = (TUint16)0x1000;
const TUint16 KDataRelocType            = (TUint16)0x2000;
const TUint16 KInferredRelocType        = (TUint16)0x3000;

const TUint KFormatNotCompressed=0;
const TUint KUidCompressionDeflate=0x101F7AFC;
const TUint KUidCompressionBytePair=0x102822AA;

//UID1:
const TUint KDynamicLibraryUidValue=0x10000079;
const TUint KExecutableImageUidValue=0x1000007a; //All executable targets have
//UID2:
const TUint KUidApp              = 0x100039CE; //should be specified by GUI applications only
const TUint KSharedLibraryUidValue=0x1000008d;

const TUint KCodeSegIdOffset = 12;
const TUint KImageCodeUnpaged   = 0x00000100u; ///< Executable image should not be demand paged. Exclusive with KImageCodePaged,
const TUint KImageCodePaged    = 0x00000200u; ///< Executable image should be demand paged. Exclusive with KImageCodeUnpaged
const TUint KImageNmdExpData  = 0x00000400u; ///< Flag to indicate when named symbol export data present in image
const TUint KImageDataUnpaged  = 0x00001000u; ///< Flag to indicate the image should not be data paged. Exclusive with KImageDataPaged.
const TUint KImageDataPaged   = 0x00002000u; ///< Flag to indicate the image should be data paged. Exclusive with KImageDataUnpaged.
const TUint KImageDataPagingMask  = KImageDataUnpaged | KImageDataPaged; ///< Mask for data paging flags.

// export description type E32ImageHeaderV::iExportDescType
const TUint KImageHdr_ExpD_NoHoles          =0x00;  ///< No holes, all exports present.
const TUint KImageHdr_ExpD_FullBitmap       =0x01;  ///< Full bitmap present at E32ImageHeaderV::iExportDesc
const TUint KImageHdr_ExpD_SparseBitmap8    =0x02;  ///< Sparse bitmap present at E32ImageHeaderV::iExportDesc, granularity 8
const TUint KImageHdr_ExpD_Xip              =0xff;  ///< XIP file



//Hardware floating point types.
enum TFloatingPointType {
    /** No hardware floating point. */
    EFpTypeNone=0,
    /** ARM VFPv2 */
    EFpTypeVFPv2=1,
    /** ARM VFPv3 */
    EFpTypeVFPv3=2
    };

enum TCpu {
    ECpuUnknown=0, ECpuX86=0x1000, ECpuArmV4=0x2000, ECpuArmV5=0x2001, ECpuArmV6=0x2002, ECpuMCore=0x4000
    };

const TInt  KOrdinalBase=1;
const TUint KImageDll               = 0x00000001u;
const TUint KImageNoCallEntryPoint  = 0x00000002u;
const TUint KImageFixedAddressExe   = 0x00000004u;
const TUint KImageOldJFlag          = 0x00000008u;  // so we can run binaries built with pre 2.00 tools (hdrfmt=0)
const TUint KImageOldElfFlag        = 0x00000010u;  // so we can run binaries built with pre 2.00 tools (hdrfmt=0)
const TUint KImageABIMask           = 0x00000018u;  // only if hdr fmt not zero
const TInt  KImageABIShift          = 3;
const TUint KImageABI_GCC98r2       = 0x00000000u;  // for ARM
const TUint KImageABI_EABI          = 0x00000008u;  // for ARM
const TUint KImageEptMask           = 0x000000e0u;  // entry point type
const TInt  KImageEptShift          = 5;
const TUint KImageEpt_Eka1          = 0x00000000u;
const TUint KImageEpt_Eka2          = 0x00000020u;
const TUint KImageHdrFmtMask        = 0x0f000000u;
const TInt  KImageHdrFmtShift       = 24;
const TUint KImageHdrFmt_Original   = 0x00000000u;  // without compression support
const TUint KImageHdrFmt_J          = 0x01000000u;  // with compression support
const TUint KImageHdrFmt_V          = 0x02000000u;  // with versioning support
const TUint KImageImpFmtMask        = 0xf0000000u;
const TInt  KImageImpFmtShift       = 28;
const TUint KImageImpFmt_PE         = 0x00000000u;  // PE-derived imports
const TUint KImageImpFmt_ELF        = 0x10000000u;  // ELF-derived imports
const TUint KImageImpFmt_PE2        = 0x20000000u;  // PE-derived imports without redundant copy of import ordinals
const TUint KImageHWFloatMask       = 0x00f00000u;
const TInt  KImageHWFloatShift      = 20;
const TUint KImageHWFloat_None      = EFpTypeNone << KImageHWFloatShift; // No hardware floating point used
const TUint KImageHWFloat_VFPv2     = EFpTypeVFPv2 << KImageHWFloatShift; // ARM VFPv2 floating point used

const TUint KMyFormatNotCompressed=0;
const TUint KMyUidCompressionDeflate=0x101F7AFC;

const TUint32 KImageCrcInitialiser  = 0xc90fdaa2u;
/*
const TUint16 KReservedRelocType        = (TUint16)0x0000;
const TUint16 KTextRelocType            = (TUint16)0x1000;
const TUint16 KDataRelocType            = (TUint16)0x2000;
const TUint16 KInferredRelocType        = (TUint16)0x3000;
*/

class TUid {
    public:
#ifndef __KERNEL_MODE__
//  IMPORT_C TBool operator==(const TUid& aUid) const;
//  IMPORT_C TBool operator!=(const TUid& aUid) const;
//  IMPORT_C TUidName Name() const;
#endif
    static inline TUid Uid(TUint aUid) {
        TUid uid= {aUid};
        return uid;
        }
    static inline TUid Null() {
        return {0};
        }
    public:
        /**
        The 32-bit integer UID value.
        */
#if (__cplusplus >= 201402L)
        TUint32 iUid;
#else
        TInt32 iUid;
#endif
    };

/**
A block of relocations for a single page (4kB) of code/data.

Immediately following this structure are an array of TUint16 values
each representing a single value in the page which is to be relocated.
The lower 12 bits of each entry is the offset, in bytes, from start of this page.
The Upper 4 bits are the relocation type to be applied to the 32-bit value located
at that offset.
 - 1 means relocate relative to code section.
 - 2 means relocate relative to data section.
 - 3 means relocate relative to code or data section; calculate which.

A value of all zeros (0x0000) is ignored. (Used for padding structure to 4 byte alignment).
*/
struct E32RelocBlock {
	TUint32 iPageOffset; ///< Offset, in bytes, for the page being relocated; relative to the section start. Always a multiple of the page size: 4096 bytes.
	TUint32 iBlockSize;  ///< Size, in bytes, for this block structure. Always a multiple of 4.
// TUint16 iEntry[]
    };

struct TUidType {
    TUidType();
    TUidType(TUid aUid1,TUid aUid2,TUid aUid3);
    TUid iUid[KMaxCheckedUid];
    };

const TUid KDynamicLibraryUid= {KDynamicLibraryUidValue};
const TUid KExecutableImageUid= {KExecutableImageUidValue};

class TCheckedUid {
    public:
//  TCheckedUid();
	TCheckedUid(const TUidType& aUidType);
//  TCheckedUid(const TDesC8& aPtr);
//  void Set(const TUidType& aUidType);
//  void Set(const TDesC8& aPtr);
//  TPtrC8 Des() const;
//  const TUidType& UidType() const;
//protected:
//  TUint Check() const;
    private:
        TUidType iType;
        TUint iCheck;
    };

constexpr TInt KCapabilitySetMaxSize = (((TInt)ECapability_HardLimit + 7)>>3);

class TCapabilitySet {
    public:
        inline TCapabilitySet() {}
        inline TCapabilitySet(TCapability aCapability) {
            new TCapabilitySet(aCapability);
            }
        TCapabilitySet(TCapability aCapability1, TCapability aCapability2);
        void SetEmpty();
        inline void Set(TCapability aCapability) {
            new TCapabilitySet(aCapability, aCapability);
            }
        inline void Set(TCapability aCapability1, TCapability aCapability2) {
            new TCapabilitySet(aCapability1, aCapability2);
            }
        void AddCapability(TCapability aCapability);
        void RemoveCapability(TCapability aCapability);
        void Union(const TCapabilitySet&  aCapabilities);
        void Intersection(const TCapabilitySet& aCapabilities);
        void Remove(const TCapabilitySet& aCapabilities);
        bool HasCapability(TCapability aCapability) const;
        bool HasCapabilities(const TCapabilitySet& aCapabilities) const;

        /**
        Make this set consist of the capabilities which are disabled on this platform.
        @internalTechnology
        */
        //void SetDisabled();
        /**
        @internalComponent
        */
        bool NotEmpty() const;
    private:
        TUint32 iCaps[KCapabilitySetMaxSize / sizeof(TUint32)];
    };

struct SCapabilitySet {
    const static short ENCapW=2;

    inline void AddCapability(TCapability aCap1) {
        ((TCapabilitySet*)this)->AddCapability(aCap1);
        }
    inline void Remove(const SCapabilitySet& aCaps) {
        ((TCapabilitySet*)this)->Remove(*((TCapabilitySet*)&aCaps));
        }
    inline bool NotEmpty() const {
        return ((TCapabilitySet*)this)->NotEmpty();
        }

    inline const TUint32& operator[] (TInt aIndex) const {
        return iCaps[aIndex];
        }
    inline TUint32& operator[] (TInt aIndex) {
        return iCaps[aIndex];
        }

    TUint32 iCaps[ENCapW];
    };

const SCapabilitySet AllSupportedCapabilities = {
        {
        ECapability_Limit<32  ? (TUint32)((1u<<(ECapability_Limit&31))-1u) : 0xffffffffu
        ,
        ECapability_Limit>=32 ? (TUint32)((1u<<(ECapability_Limit&31))-1u) : 0u
        }
    };

enum TProcessPriority {
    EPriorityLow=150,
    EPriorityBackground=250,
    EPriorityForeground=350,
    EPriorityHigh=450,
    EPriorityWindowServer=650,
    EPriorityFileServer=750,
    EPriorityRealTimeServer=850,
    EPrioritySupervisor=950
    };

class E32EpocExpSymInfoHdr {
    public:
        TInt iSize;      // size of this Table
        TInt16 iFlags;
        TInt16 iSymCount;     // number of symbols
        TInt iSymbolTblOffset;   // start of the symbol table - offset from byte 0 of this header
        TInt iStringTableSz;    // size of the string table
        TInt iStringTableOffset;   // start of the string table having names of the symbols - offset from byte 0 of this header
        TInt iDllCount;     // Number of dependent DLLs
        TInt iDepDllZeroOrdTableOffset; // offset of the DLL dependency table - offset from byte 0 of this header.
    };

class TVersion {
    public:
        TVersion(TInt aMajor,TInt aMinor,TInt aBuild);
        TVersion(): TVersion(0,0,0) {}
        //IMPORT_C TVersionName Name() const;
    public:
        TInt8 iMajor; //The major version number.
        TInt8 iMinor; //The minor version number.
        TInt16 iBuild; //The build number.
    };


struct E32RelocSection {
    TInt iSize;                 // size of this relocation section
    TInt iNumberOfRelocs;       // number of relocations in this section
    };

typedef struct TExceptionDescriptor {
    TUint32 iExIdxBase;
    TUint32 iExIdxLimit;
    TUint32 iROSegmentBase;
    TUint32 iROSegmentLimit;
    } TExceptionDescriptor;

class E32ImageHeader {
    public:
        /**
        Return total size of file after decompression, or -1 if file not compressed.
        */
        inline TInt UncompressedFileSize() const;

        /**
        Return #iProcessPriority.
        */
        inline TProcessPriority ProcessPriority() const {
            return (TProcessPriority)iProcessPriority;
            }
        /** Return #iModuleVersion.*/
        inline TUint32 ModuleVersion() const {
            return iModuleVersion;
            }

        /**
        Return #iCpuIdentifier.
        */
        inline TCpu CpuIdentifier() const {
            return (TCpu)iCpuIdentifier;
            }

        /**
        Extract ABI type from aFlags.
        */
        inline TUint ABIFromFlags(TUint aFlags) {
            return aFlags & KImageABIMask;
            }

        /**
        Extract entrypoint format from aFlags.
        */
        inline TUint EptFromFlags(TUint aFlags) {
            return aFlags & KImageEptMask;
            }

        TUint HeaderFormat() const {
            return HdrFmtFromFlags(iFlags);
            }

        inline TUint ImpFmtFromFlags(TUint aFlags) {
            return aFlags & KImageImpFmtMask;
            }

        inline TInt TotalSize() const {
            return iCodeOffset;
            }

        inline TUint32 CompressionType() const {
            return iCompressionType;
            }

        inline TUint ImportFormat() {
            return ImpFmtFromFlags(iFlags);
            }

        inline static TUint HdrFmtFromFlags(TUint aFlags) {
            if (aFlags&KImageHdrFmtMask)
                return aFlags & KImageHdrFmtMask;
            if (aFlags&KImageOldJFlag)
                return KImageHdrFmt_J;
            return KImageHdrFmt_Original;
            }

        inline TInt ValidateHeader(TInt aFileSize, TUint32& aUncompressedSize) const;

        TUint32 iUid1;
        TUint32 iUid2;
        TUint32 iUid3;
        TUint32 iUidChecksum;
        TUint iSignature;           // 'EPOC'
        TUint32 iHeaderCrc;         // CRC-32 of entire header
        TUint32 iModuleVersion;     // Version number for this executable (used in link resolution)
        TUint32 iCompressionType;   // Type of compression used (UID or 0 for none)
        TVersion iToolsVersion;     // Version of PETRAN/ELFTRAN which generated this file
        TUint32 iTimeLo;
        TUint32 iTimeHi;
        TUint iFlags;               // 0 = exe, 1 = dll, 2 = fixed address exe
        TInt iCodeSize;             // size of code, import address table, constant data and export dir
        TInt iDataSize;             // size of initialised data
        TInt iHeapSizeMin;
        TInt iHeapSizeMax;
        TInt iStackSize;
        TInt iBssSize;
        TUint iEntryPoint;          // offset into code of entry point
        TUint iCodeBase;            // where the code is linked for
        TUint iDataBase;            // where the data is linked for
        TInt iDllRefTableCount;     // filling this in enables E32ROM to leave space for it
        TUint iExportDirOffset;     // offset into the file of the export address table
        TInt iExportDirCount;
        TInt iTextSize;             // size of just the text section, also doubles as the offset for the iat w.r.t. the code section
        TUint iCodeOffset;          // file offset to code section, also doubles as header size
        TUint iDataOffset;          // file offset to data section
        TUint iImportOffset;        // file offset to import section
        TUint iCodeRelocOffset;     // relocations for code and const
        TUint iDataRelocOffset;     // relocations for data
        TUint16 iProcessPriority;   // executables priority
        TUint16 iCpuIdentifier;     // 0x1000 = X86, 0x2000 = ARM
    };

struct SSecurityInfo {
    TUint32 iSecureId;
    TUint32 iVendorId;
    SCapabilitySet iCaps;   // Capabilities re. platform security
    };

/**
Extends E32ImageHeader.
*/
class E32ImageHeaderComp : public E32ImageHeader
	{
public:
	TUint32 iUncompressedSize;	///< Uncompressed size of file data after the header, or zero if file not compressed.
};

class E32ImageHeaderV : public E32ImageHeaderComp {
    public:
        TInt ValidateWholeImage(void* aBufferStart, TUint aBufferSize) const;
        TInt ValidateHeader(TInt aFileSize, TUint32& aUncompressedSize) const;
        TInt ValidateExportDescription() const;
        TInt ValidateRelocations(void* aBufferStart, TUint aBufferSize, TUint aRelocationInfoOffset,
                                 TUint aRelocatedSectionSize, E32RelocSection*& aRelocationSection) const;
        TInt ValidateImports(void* aBufferStart, TUint aBufferSize, TUint& aBiggestImportCount) const;

        SSecurityInfo iS;

        // Use iSpare1 as offset to Exception Descriptor
        TUint32 iExceptionDescriptor;   // Offset in bytes from start of code section to Exception Descriptor, bit 0 set if valid
        TUint32 iSpare2;
        TUint16 iExportDescSize;    // size of bitmap section
        TUint8  iExportDescType;    // type of description of holes in export table
        TUint8  iExportDesc[1];     // description of holes in export table - extend
    };


TInt E32ImageHeader::ValidateHeader(TInt aFileSize, TUint32& aUncompressedSize) const {
    // check file is big enough for any header...
    if(TUint(aFileSize)<sizeof(*this)) return KErrCorrupt;
    TUint hdrfmt = HeaderFormat();
    if(hdrfmt==KImageHdrFmt_V)
        return ((E32ImageHeaderV*)this)->ValidateHeader(aFileSize,aUncompressedSize);

    return KErrNotSupported; // header format unrecognised
    }

inline TInt E32ImageHeader::UncompressedFileSize() const {
    if(iCompressionType==0)
        return -1;          // not compressed
    else
        return ((E32ImageHeaderV*)this)->iUncompressedSize + TotalSize();
    }

/**
Header for the Import Section in an image, as referenced by E32ImageHeader::iImportOffset.
Immediately following this structure are an array of E32ImportBlock structures.
The number of these is given by E32ImageHeader::iDllRefTableCount.
*/
struct E32ImportSection {
    TInt iSize;     ///< Size of this section excluding 'this' structure
//  E32ImportBlock iImportBlock[iDllRefTableCount];
    };

/**
A block of imports from a single executable.
These structures are conatined in a images Import Section (E32ImportSection).
*/
class E32ImportBlock {
    public:
        inline const E32ImportBlock* NextBlock(TUint aImpFmt) const;
        inline uint32_t Size(TUint aImpFmt) const;
        inline const TUint* Imports() const;    // import list if present
    public:
        TUint32 iOffsetOfDllName;           ///< Offset from start of import section for a NUL terminated executable (DLL or EXE) name.
        TInt    iNumberOfImports;           ///< Number of imports from this executable.
//  TUint   iImport[iNumberOfImports];  ///< For ELF-derived executes: list of code section offsets. For PE, list of imported ordinals. Omitted in PE2 import format
    };

/**
Return size of this import block.
@param aImpFmt Import format as obtained from image header.
*/
inline uint32_t E32ImportBlock::Size(TUint aImpFmt) const {
	uint32_t r = sizeof(E32ImportBlock);
    if(aImpFmt!=KImageImpFmt_PE2)
        r += iNumberOfImports * sizeof(TUint);
    return r;
    }

/**
Return pointer to import block which immediately follows this one.
@param aImpFmt Import format as obtained from image header.
*/
inline const E32ImportBlock* E32ImportBlock::NextBlock(TUint aImpFmt) const {
    const E32ImportBlock* next = this + 1;
    if(aImpFmt!=KImageImpFmt_PE2)
        next = (const E32ImportBlock*)( (TUint8*)next + iNumberOfImports * sizeof(TUint) );
    return next;
    }

/**
Return address of first import in this block.
For import format KImageImpFmt_ELF, imports are list of code section offsets.
For import format KImageImpFmt_PE, imports are a list of imported ordinals.
For import format KImageImpFmt_PE2, the import list is not present and should not be accessed.
*/
inline const TUint* E32ImportBlock::Imports() const {
    return (const TUint*)(this + 1);
    }

#ifndef RETURN_FAILURE
#define RETURN_FAILURE(_r) return (fprintf(stderr, "line %d\n", __LINE__),_r)
#endif // RETURN_FAILURE
#ifndef E32IMAGEHEADER_TRACE
#define E32IMAGEHEADER_TRACE(_t) printf _t
#endif // E32IMAGEHEADER_TRACE

/**
Valdate that the export description is valid.
*/
inline TInt E32ImageHeaderV::ValidateExportDescription() const {
    TUint headerSize = iCodeOffset;

    // check export description...
    TUint edSize = iExportDescSize + sizeof(iExportDescSize) + sizeof(iExportDescType);
    edSize = (edSize+3)&~3;
    TUint edEnd = offsetof(E32ImageHeaderV,iExportDescSize)+edSize;
    if(edEnd!=headerSize)
        RETURN_FAILURE(KErrCorrupt);

    // size of bitmap of exports...
    TUint bitmapSize = (iExportDirCount+7) >> 3;

    // check export description bitmap...
    switch(iExportDescType) {
        case KImageHdr_ExpD_NoHoles:
            // no bitmap to check...
            E32IMAGEHEADER_TRACE(("ValidateExportDescription NoHoles"));
            return KErrNone;

        case KImageHdr_ExpD_FullBitmap:
            // full bitmap present...
            E32IMAGEHEADER_TRACE(("ValidateExportDescription FullBitmap"));
            if(bitmapSize!=iExportDescSize)
                RETURN_FAILURE(KErrCorrupt);
            return KErrNone;

        case KImageHdr_ExpD_SparseBitmap8: {
            // sparse bitmap present...
            E32IMAGEHEADER_TRACE(("ValidateExportDescription SparseBitmap8"));

            // get size of meta-bitmap...
            TUint metaBitmapSize = (bitmapSize+7) >> 3;
            if(metaBitmapSize>iExportDescSize)
                RETURN_FAILURE(KErrCorrupt); // doesn't fit

            TUint totalSize = metaBitmapSize;

            // scan meta-bitmap counting extra bytes which should be present...
            const TUint8* metaBitmap = iExportDesc;
            const TUint8* metaBitmapEnd = metaBitmap + metaBitmapSize;
            while(metaBitmap<metaBitmapEnd) {
                TUint bits = *metaBitmap++;
                do {
                    if(bits&1)
                        ++totalSize; // another byte is present in bitmap
                    }
                while(bits>>=1);
                }

            if(totalSize!=iExportDescSize)
                RETURN_FAILURE(KErrCorrupt);
            }
        return KErrNone;

        default:
            E32IMAGEHEADER_TRACE(("ValidateExportDescription ?"));
            RETURN_FAILURE(KErrNotSupported);
        }
    }






#if 0
/**
@internalTechnology

A sorted list of all the code segments in ROM that contain an Exception Descriptor.

*/
typedef struct TRomExceptionSearchTable {
    /**
    The number of entries in the following table.
    */
    TInt32 iNumEntries;

    /**
    Address of the code segment of each TRomImageHeader that has an Exception Descriptor.
    */
    TLinAddr iEntries[1];
    } TRomExceptionSearchTable;

typedef struct TExceptionDescriptor {
    TLinAddr iExIdxBase;
    TLinAddr iExIdxLimit;
    TLinAddr iROSegmentBase;
    TLinAddr iROSegmentLimit;
    } TExceptionDescriptor;
#endif // 0
#ifdef __MINGW32__
#endif // __MINGW32__

#endif // __portable__
