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
class E32ExportTable;
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
        E32ImageFile(ElfImage * aElfImage, ElfFileSupplied * aUseCase,
                     ParameterManager * aManager, E32ExportTable *aTable);
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
        void SetE32ImgHdrFields();
        uint32_t EntryPointOffset();

        bool AllowDllData();

        enum EEntryPointStatus {
            EEntryPointOK,
            EEntryPointCorrupt,
            EEntryPointNotSupported
            };

        EEntryPointStatus ValidateEntryPoint();

        void SetUpExceptions();
        void SetPriority(bool isDllp);
        void SetFixedAddress(bool isDllp);

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

        bool   iLayoutDone=false;

        int iMissingExports=0;

        // This table carries the byte offsets in the import table entries corresponding
        // to the 0th ordinal entry of static dependencies.
        std::vector<int32_t>  iImportTabLocations;
        std::vector<uint32_t> iSymAddrTab;
        std::vector<uint32_t> iSymNameOffTab;
        string      iSymbolNames;
        uint32_t    iSymNameOffset=0;

    public:
        void ProcessSymbolInfo();
        char* CreateSymbolInfo(size_t aBaseOffset);
        void SetSymInfo(E32EpocExpSymInfoHdr& aSymInfo);

    public:
        TInt iSize=0;

    private:
        E32ExportTable *iTable=nullptr;
    };

#endif // E32IMAGEFILE_H


