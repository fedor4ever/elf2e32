// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017 Strizhniou Fiodar
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
// Class for E32 Image implementation and dump of the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef E32IMAGEFILE_H
#define E32IMAGEFILE_H


#include <vector>
#include <fstream>
#include <iostream>

#include "elfdefs.h"
#include "portable.h"

using std::vector;
using std::string;
using std::ifstream;

class ElfImage;
class Elfparser;
class ElfRelocation;
class ELFExecutable;
class ElfFileSupplied;
class ParameterManager;

/**
Class E32ImageChunkDesc for different sections in the E32 image.
@internalComponent
@released
*/
class E32ImageChunkDesc {
    public:
        E32ImageChunkDesc(char * aData, size_t aSize, size_t aOffset, char * aDoc);
        ~E32ImageChunkDesc();
        void Init(char * aPlace);

    public:
        char * iData;
        size_t iSize;
        size_t iOffset;
        char * iDoc;
    };

typedef vector<E32ImageChunkDesc *> ChunkList;
/**
Class E32ImageChunks for a list of sections in the E32 image.
@internalComponent
@released
*/
class E32ImageChunks {
    public:
        ~E32ImageChunks();

        void AddChunk(char * aData, size_t aSize, size_t aOffset, char * aDoc);
        size_t GetOffset();
        void SetOffset(size_t aOffset);
        ChunkList & GetChunks();
        void SectionsInfo();
        void DisasmChunk(uint16_t index, uint32_t length = 0, uint32_t pos = 0);

    private:
        ChunkList iChunks;

        size_t iOffset=0;

    };

typedef unsigned char uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

/**
Class E32ImageFile for fields of an E32 image.
@internalComponent
@released
*/
class E32ImageFile {
    public:
        E32ImageFile(ElfImage * aElfImage, ElfFileSupplied * aUseCase, ParameterManager * aManager);
        E32ImageFile();
        virtual ~E32ImageFile();

        void GenerateE32Image();

        void ReadInputELFFile(string aName, size_t & aFileSize, Elf32_Ehdr * & aELFFile );

        void ProcessImports();
        string FindDSO(string aName);

        void ProcessCodeRelocations();
        void ProcessDataRelocations();

        uint16 GetE32RelocType(ElfRelocation * aReloc);

        void ConstructImage();

        void InitE32ImageHeader();
        size_t GetE32ImageHeaderSize();
        size_t GetExtendedE32ImageHeaderSize();
        void SetExtendedE32ImageHeaderSize(size_t aSize);

        void ComputeE32ImageLayout();
        size_t GetE32ImageSize();
        size_t GetExportOffset();
        void CreateExportBitMap();
        void AddExportDescription();

        void AllocateE32Image();
        void FinalizeE32Image();
        uint16_t GetCpuIdentifier();
        uint32_t EntryPointOffset();

        bool AllowDllData();

        enum EEntryPointStatus {
            EEntryPointOK,
            EEntryPointCorrupt,
            EEntryPointNotSupported
            };

        EEntryPointStatus ValidateEntryPoint();

        void SetUpExceptions();
        void SetUids();
        void SetSecureId();
        void SetVendorId();
        void SetCallEntryPoints();
        void SetCapability();
        void SetPriority(bool isDllp);
        void SetFixedAddress(bool isDllp);
        void SetVersion();
        void SetCompressionType();
        void SetFPU();
        void SetPaged();
        void SetSymbolLookup();
        void SetDebuggable();
        void SetSmpSafe();

        void UpdateHeaderCrc();

        bool WriteImage(const char * aName);

    public:

        char * iE32Image=nullptr;
        uint8 * iExportBitMap=nullptr;
        ElfImage * iElfImage=nullptr;

        char* iData=nullptr;

        ElfFileSupplied * iUseCase=nullptr;
        ParameterManager * iManager=nullptr;

        E32ImageHeaderV * iHdr=nullptr;
        size_t iHdrSize=sizeof(E32ImageHeaderV);

        E32ImageChunks iChunks;

        uint32 iNumDlls=0;
        uint32 iNumImports=0;

        uint32 * iImportSection=nullptr;
        size_t iImportSectionSize=0;

        char * iCodeRelocs=nullptr;
        size_t iCodeRelocsSize=0;

        char * iDataRelocs=nullptr;
        size_t iDataRelocsSize=0;

        size_t iExportOffset=0;
        bool   iLayoutDone=false;

        int iMissingExports=0;

        // This table carries the byte offsets in the import table entries corresponding
        // to the 0th ordinal entry of static dependencies.
        std::vector<int32_t>    iImportTabLocations;
        std::vector<uint32_t> iSymAddrTab;
        std::vector<uint32_t> iSymNameOffTab;
        string      iSymbolNames;
        uint32_t    iSymNameOffset=0;

    public:
        TInt ReadHeader(ifstream& is);
        TInt Open(const char* aFileName);
        void Adjust(int32_t aSize, bool aAllowShrink=true);

        TUint TextOffset();
        TUint DataOffset();
        TUint BssOffset();

        void Dump(const char *aFileName,TInt aDumpFlags);
        void DumpHeader(TInt aDumpFlags);
        void DumpData(TInt aDumpFlags);
        void DumpSymbolInfo(E32EpocExpSymInfoHdr *aSymInfoHdr);
        void E32ImageExportBitMap();
        TInt CheckExportDescription();
        void ProcessSymbolInfo();
        char* CreateSymbolInfo(size_t aBaseOffset);
        void SetSymInfo(E32EpocExpSymInfoHdr& aSymInfo);
    public:
        inline TUint OffsetUnadjust(TUint a) const {
            return a ? a-iOrigHdrOffsetAdj : 0;
            }
        inline TUint OrigCodeOffset() const {
            return OffsetUnadjust(iOrigHdr->iCodeOffset);
            }
        inline TUint OrigDataOffset() const {
            return OffsetUnadjust(iOrigHdr->iDataOffset);
            }
        inline TUint OrigCodeRelocOffset() const {
            return OffsetUnadjust(iOrigHdr->iCodeRelocOffset);
            }
        inline TUint OrigDataRelocOffset() const {
            return OffsetUnadjust(iOrigHdr->iDataRelocOffset);
            }
        inline TUint OrigImportOffset() const {
            return OffsetUnadjust(iOrigHdr->iImportOffset);
            }
        inline TUint OrigExportDirOffset() const {
            return OffsetUnadjust(iOrigHdr->iExportDirOffset);
            }
        inline void OffsetAdjust(TUint& a) {
            if (a) a+=iOrigHdrOffsetAdj;
            }

    public:
        TInt iSize=0;
        E32ImageHeader* iOrigHdr=nullptr;
        TInt iError=0;
        TUint iOrigHdrOffsetAdj=0;
        TInt iFileSize=0;
    private:
        Elfparser *aParser=nullptr;
    };

ifstream &operator>>(ifstream &is, E32ImageFile &aImage);
void InflateUnCompress(unsigned char* source, int sourcesize, unsigned char* dest, int destsize);


#endif // E32IMAGEFILE_H


