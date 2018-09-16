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
// Class for implementation of the Class E32Parser for the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef E32PRODUCER_H
#define E32PRODUCER_H

class E32ImageHeader;
class E32ImageHeaderJ;
class E32ImageHeaderV;
class E32ImportSection;

class E32Parser
{
    public:
        E32Parser(const char* fileName);
        ~E32Parser();

        E32ImageHeader *GetFileLayout();
        E32ImageHeaderJ *GetE32HdrJ() const;
        E32ImageHeaderV *GetE32HdrV() const;

        uint32_t BSSOffset();
        TExceptionDescriptor *GetExceptionDescriptor() const;
        E32ImportSection *GetImportSection() const;
        char *GetImportAddressTable() const;

        E32EpocExpSymInfoHdr *GetEpocExpSymInfoHdr() const;

        char *GetDLLName(uint32_t OffsetOfDllName) const;

        char *GetBufferedImage() const;
        int32_t UncompressedFileSize() const;
        size_t GetFileSize() const;
        int32_t GetExportDescription();
        char *GetE32RelocSection(uint32_t offSet);
    private:
        void ReadFile();
        void ParseExportBitMap();
        void DecompressImage();

    private:
        E32ImageHeader *iHdr = nullptr;
        E32ImageHeaderJ *iHdrJ = nullptr;
        E32ImageHeaderV *iHdrV = nullptr;

        const char *iFileName = nullptr;
        char *iBufferedFile = nullptr;
        size_t iE32Size = 0;

        //used in ParseExportBitMap()
        uint8_t *iExportBitMap = nullptr;
        size_t iMissingExports = 0;
};

uint32_t HdrFmtFromFlags(uint32_t flags);
uint32_t ImpFmtFromFlags(uint32_t flags);
size_t RoundUp(size_t value, size_t rounding = 4);

#endif // E32PRODUCER_H
