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
// Implementation of the Class E32Info for the elf2e32 tool
// @internalComponent
// @released
//
//

#include <cstdio>
#include <cstring>

#include "e32info.h"
#include "deffile.h"
#include "pl_symbol.h"
#include "e32parser.h"
#include "errorhandler.h"
#include "e32validator.h"
#include "parametermanager.h"

#define REFERENCE_CAPABILITY_NAMES
//#define INCLUDE_CAPABILITY_NAMES
#include "e32capability.h"

void DumpRelocs(char *relocs);
void GenerateAsmFile(ParameterManager *param);

E32Info::E32Info(ParameterManager *param): iParam(param),
    iFlags(param->FileDumpOptions()) {}

E32Info::~E32Info()
{
    delete iE32;
}

void E32Info::HeaderInfo()
{
    if(!iHdr1)
        printf("%s\n", "File not read!");

    uint32_t flags = iHdr1->iFlags;
    bool isARM = false;
    uint32_t hdrfmt = HdrFmtFromFlags(flags);

    printf("V%d.%02d(%03d)", iHdr1->iVersion.iMajor,
           iHdr1->iVersion.iMinor, iHdr1->iVersion.iBuild);
    printf("\tTime Stamp: %08x,%08x\n", iHdr1->iTimeHi, iHdr1->iTimeLo);

    char sig[5] = {};
    memcpy(sig, (const char*)&iHdr1->iSignature, 4);
    printf(sig);

    if(flags & KImageDll)
        printf(" Dll for ");
    else
        printf(" Exe for ");
    CPUIdentifier(iHdr1->iCpuIdentifier, isARM);

    printf("Flags:\t%08x\n", flags);

    if (!(flags & KImageDll))
    {
        printf("Priority ");
        ImagePriority((TProcessPriority)iHdr1->iProcessPriority);
        if (flags & KImageFixedAddressExe)
            printf("Fixed process\n");
    }

    if (flags & KImageNoCallEntryPoint)
        printf("Entry points are not called\n");

    printf("Image header is format %u\n", hdrfmt>>24);
    uint32_t compression = iHdr1->iCompressionType;

    switch (compression)
    {
    case KFormatNotCompressed:
        printf("Image is not compressed\n");
        break;
    case KUidCompressionDeflate:
        printf("Image is compressed using the DEFLATE algorithm\n");
        break;
    case KUidCompressionBytePair:
        printf("Image is compressed using the BYTEPAIR algorithm\n");
        break;
    default:
        printf("Image compression type UNKNOWN (%08x)\n", compression);
    }

    if (compression)
        printf("Uncompressed size %08x\n", iE32->UncompressedFileSize());


    uint32_t FPU = flags & KImageHWFloatMask;
    if (FPU == KImageHWFloat_None)
        printf("Image FPU support : Soft VFP\n");
    else if (FPU == KImageHWFloat_VFPv2)
        printf("Image FPU support : VFPv2\n");
    else
        printf("Image FPU support : Unknown\n");


    if (flags & KImageCodeUnpaged)
			printf("Code Paging : Unpaged\n");
    else if (flags & KImageCodePaged)
        printf("Code Paging : Paged\n");
    else
        printf("Code Paging : Default\n");


    if (flags & KImageDataUnpaged)
        printf("Data Paging : Unpaged\n");
    else if (flags & KImageDataPaged)
        printf("Data Paging : Paged\n");
    else
        printf("Data Paging : Default\n");

    if (iHdr1->iFlags & KImageDebuggable)
        printf("Debuggable : True\n");
    else
        printf("Debuggable : False\n");

    if (iHdr1->iFlags & KImageSMPSafe)
        printf("SMP Safe : True\n");
    else
        printf("SMP Safe : False\n");

    SecurityInfo();

    if (hdrfmt >= E32HdrFmt::KImageHdrFmt_V)
    {
        E32ImageHeaderV* v = iE32->GetE32HdrV();
        uint32_t xd = v->iExceptionDescriptor;
        if ((xd & 1) && (xd != 0xffffffffu))
        {
            printf("Exception Descriptor Offset:  %08x\n", xd);
            TExceptionDescriptor *ed = iE32->GetExceptionDescriptor();
            printf("Exception Index Table Base: %08x\n", ed->iExIdxBase);
            printf("Exception Index Table Limit: %08x\n", ed->iExIdxLimit);
            printf("RO Segment Base: %08x\n", ed->iROSegmentBase);
            printf("RO Segment Limit: %08x\n", ed->iROSegmentLimit);
        }
        else
            printf("No Exception Descriptor\n");

        printf("Export Description: Size=%03x, Type=%02x\n", v->iExportDescSize, v->iExportDescType);

        if (v->iExportDescType != KImageHdr_ExpD_NoHoles)
        {
            int32_t nb = v->iExportDescSize;
            for (int32_t i=0, j = 0; i<nb; ++i)
            {
                if (++j == 8)
                {
                    j = 0;
                    printf("\n");
                }
                printf(" %02x", v->iExportDesc[i]);
            }
            printf("\n");
        }

        int32_t r = iE32->GetExportDescription();

        if (r == KErrNone)
            printf("Export description consistent\n");
        else if (r == KErrNotSupported)
            printf("Export description type not recognised\n");
        else
            printf("!! Export description inconsistent !!\n");
    }

    uint32_t mv = iHdr1->iModuleVersion;
    printf("Module Version: %u.%u\n", mv>>16, mv&0xffff);

    uint32_t impfmt = ImpFmtFromFlags(flags);
    if (impfmt == KImageImpFmt_PE)
    {
        printf("Imports are PE-style\n");
    }
    else if (impfmt == KImageImpFmt_ELF)
    {
        printf("Imports are ELF-style\n");
    }
    else if (impfmt == KImageImpFmt_PE2)
    {
        printf("Imports are PE-style without redundant ordinal lists\n");
    }

    /// TODO (Administrator#1#09/11/18): Print right message for pre-9 binaries
    printf("ARM EABI\n");
    printf("Built against EKA2\n");
    // end todo

    printf("Uids:\t\t%08x %08x %08x (%08x)\n", iHdr1->iUid1, iHdr1->iUid2, iHdr1->iUid3, iHdr1->iUidChecksum);

    if (hdrfmt >= KImageHdrFmt_V)
        printf("Header CRC:\t%08x\n", iHdr1->iHeaderCrc);

    printf("File Size:\t%08x\n", iE32->GetFileSize());
    printf("Code Size:\t%08x\n", iHdr1->iCodeSize);
    printf("Data Size:\t%08x\n", iHdr1->iDataSize);
    printf("Compression:\t%08x\n", iHdr1->iCompressionType);
    printf("Min Heap Size:\t%08x\n", iHdr1->iHeapSizeMin);
    printf("Max Heap Size:\t%08x\n", iHdr1->iHeapSizeMax);
    printf("Stack Size:\t%08x\n", iHdr1->iStackSize);
    printf("Code link addr:\t%08x\n", iHdr1->iCodeBase);
    printf("Data link addr:\t%08x\n", iHdr1->iDataBase);
    printf("Code reloc offset:\t%08x\n", iHdr1->iCodeRelocOffset);
    printf("Data reloc offset:\t%08x\n", iHdr1->iDataRelocOffset);
    printf("Dll ref table count: %d\n", iHdr1->iDllRefTableCount);

    if (iHdr1->iCodeSize || iHdr1->iDataSize || iHdr1->iBssSize || iHdr1->iImportOffset)
        printf("        Offset  Size  Relocs #Relocs\n");

    printf("Code    %06x %06x", iHdr1->iCodeOffset, iHdr1->iCodeSize);

    if (iHdr1->iCodeRelocOffset)
    {
        E32RelocSection *r=(E32RelocSection *)(iE32->GetBufferedImage() +
                                               iHdr1->iCodeRelocOffset);
        printf(" %06x %06x", iHdr1->iCodeRelocOffset, r->iNumberOfRelocs);
    }
    else
        printf("              ");

    printf("        +%06x (entry pnt)", iHdr1->iEntryPoint);
    printf("\n");

    printf("Data    %06x %06x", iHdr1->iDataOffset, iHdr1->iDataSize);

    if (iHdr1->iDataRelocOffset)
    {
        E32RelocSection *r=(E32RelocSection *)(iE32->GetBufferedImage() +
                                               iHdr1->iDataRelocOffset);
        printf(" %06x %06x", iHdr1->iDataRelocOffset, r->iNumberOfRelocs);
    }
    printf("\n");

    printf("Bss            %06x\n", iHdr1->iBssSize);

    if (iHdr1->iExportDirOffset)
        printf("Export  %06x %06x                      (%u entries)\n",
               iHdr1->iExportDirOffset, iHdr1->iExportDirCount*4, iHdr1->iExportDirCount);

    if (iHdr1->iImportOffset)
        printf("Import  %06x\n", iHdr1->iImportOffset);
}

void E32Info::ImagePriority(TProcessPriority priority) const
{
    if (priority==EPrioritySupervisor)
		printf("Supervisor");

    else if (priority>EPriorityRealTimeServer)
		printf("RealTime+%d", priority-EPriorityRealTimeServer);
    else if (priority==EPriorityRealTimeServer)
		printf("RealTime");

    else if (priority>EPriorityFileServer)
		printf("FileServer+%d", priority-EPriorityFileServer);
	else if (priority==EPriorityFileServer)
		printf("FileServer");

	else if (priority>EPriorityWindowServer)
		printf("WindowServer+%d", priority-EPriorityWindowServer);
	else if (priority==EPriorityWindowServer)
		printf("WindowServer");

    else if (priority>EPriorityHigh)
		printf("High+%d", priority-EPriorityHigh);
	else if (priority==EPriorityHigh)
		printf("High");

	else if (priority>EPriorityForeground)
		printf("Foreground+%d", priority-EPriorityForeground);
	else if (priority==EPriorityForeground)
		printf("Foreground");

	else if (priority>EPriorityBackground)
		printf("Background+%d", priority-EPriorityBackground);
	else if (priority==EPriorityBackground)
		printf("Background");

    else if (priority>EPriorityLow)
		printf("Low+%d", priority-EPriorityLow);
	else if (priority==EPriorityLow)
		printf("Low");

	else
		printf("Illegal (%d)", priority);

    printf("\n");
}

void E32Info::SecurityInfo(bool CapNames)
{
    uint32_t fmt = HdrFmtFromFlags(iHdr1->iFlags);
    if (fmt < KImageHdrFmt_V)
        return;

    //
    // Important. Don't change output format of following security info
    // because this is relied on by used by "Symbian Signed".
    //
    E32ImageHeaderV* v = iE32->GetE32HdrV();
    printf("Secure ID: %08x\n", v->iS.iSecureId);
    printf("Vendor ID: %08x\n", v->iS.iVendorId);
    printf("Capabilities: %08x %08x\n", v->iS.iCaps.iSet[1], v->iS.iCaps.iSet[0]);

    if(!CapNames)
        return;

    for(int32_t i=0; i<ECapability_Limit; i++)
        if(v->iS.iCaps.iSet[i>>5]&(1<<(i&31)))
            printf("\t\t%s\n", CapabilityNames[i]);
    printf("\n");
}

void E32Info::CodeSection()
{
    printf("\nCode (text size=%08x)\n", iHdr1->iTextSize);
    PrintHexData(iE32->GetBufferedImage() + iHdr1->iCodeOffset, iHdr1->iCodeSize);

    char *a = iE32->GetE32RelocSection(iHdr1->iCodeRelocOffset);
    if (iHdr1->iCodeRelocOffset)
        DumpRelocs(a);
}

void E32Info::DataSection()
{
    printf("\nData\n");
    PrintHexData(iE32->GetBufferedImage() + iHdr1->iDataOffset, iHdr1->iDataSize);

    char *a = iE32->GetE32RelocSection(iHdr1->iDataRelocOffset);
    if (iHdr1->iDataRelocOffset)
        DumpRelocs(a);
}

void E32Info::ExportTable()
{
    printf("\nNumber of exports = %u\n", iHdr1->iExportDirCount);
    uint32_t* exports = (uint32_t*)(iE32->GetBufferedImage() + iHdr1->iExportDirOffset);
    uint32_t absoluteEntryPoint = iHdr1->iEntryPoint + iHdr1->iCodeBase;
    uint32_t impfmt = iHdr1->iFlags & KImageImpFmtMask;
    uint32_t absentVal = (impfmt == KImageImpFmt_ELF) ? absoluteEntryPoint : iHdr1->iEntryPoint;
    for (uint32_t i = 0; i < iHdr1->iExportDirCount; ++i)
    {
        uint32_t exp = exports[i];
        if (exp == absentVal)
            printf("\tOrdinal %5u:\tABSENT\n", i+1);
        else
            printf("\tOrdinal %5u:\t%08x\n", i+1, exp);
    }
}

void E32Info::ImportTableInfo()
{
    if(!iHdr1->iImportOffset)
        return;

    char *iData = iE32->GetBufferedImage() + iHdr1->iCodeOffset;

    const E32ImportSection* section = iE32->GetImportSection();
    uint32_t* iat = (uint32_t*)iE32->GetImportAddressTable();
    printf("\nIdata\tSize=%08x\n", section->iSize);
    printf("Offset of import address table (relative to code section): %08x\n", iHdr1->iTextSize);

    const E32ImportBlock* b = (const E32ImportBlock*)(section + 1);
    for (int32_t d=0; d<iHdr1->iDllRefTableCount; d++)
    {
        char* dllname = iE32->GetDLLName(b->iOffsetOfDllName);
        int32_t n = b->iNumberOfImports;
        printf("%d imports from %s\n", b->iNumberOfImports, dllname);
        const uint32_t* p = b->Imports();
        uint32_t impfmt = ImpFmtFromFlags(iHdr1->iFlags);
        if (impfmt == KImageImpFmt_ELF)
        {
            while (n--)
            {
                uint32_t impd_offset = *p++;
                uint32_t impd = *(uint32_t*)(iData + impd_offset);
                uint32_t ordinal = impd & 0xffff;
                uint32_t offset = impd >> 16;

                if (offset)
                    printf("%10u offset by %u\n", ordinal, offset);
                else
                    printf("%10u\n", ordinal);
            }
        }
        else
        {
            while (n--)
                printf("\t%u\n", *iat++);
        }
        b = b->NextBlock(impfmt);
    }
}

/// TODO (Administrator#1#09/14/18): Investigate how creates data for symbolic lookup
void E32Info::SymbolInfo()
{
    printf("***************************************************\n");
    printf("***************************************************\n");
    printf("**********         WARNING!!!           ***********\n");
    printf("**********  That function is broken!!!  ***********\n");
    printf("**********         Do not use!!!        ***********\n");
    printf("***************************************************\n");
    printf("***************************************************\n");

    if(!(iHdr1->iFlags & KImageNmdExpData))
        return;
    E32EpocExpSymInfoHdr *symInfoHdr = iE32->GetEpocExpSymInfoHdr();
    if(!symInfoHdr)
		return;

    char *symTblBase = (char*)symInfoHdr;
    uint32_t *aSymAddrTbl = (uint32_t*)(symTblBase + symInfoHdr->iSymbolTblOffset);
    char *aSymNameTbl = (char*)(aSymAddrTbl + symInfoHdr->iSymCount);

    printf("\n");

	printf("\n\n\t\tSymbol Info\n");
	if(symInfoHdr->iSymCount)
	{
        char *strTable = symTblBase + symInfoHdr->iStringTableOffset;
		char *symbolName = nullptr;
		printf("%d Symbols exported\n",symInfoHdr->iSymCount);
		printf("  Addr\t\tName\n");
		printf("----------------------------------------\n");
		size_t nameOffset = 0;
		for(int i = 0; i < symInfoHdr->iSymCount; i++)
		{
			if(symInfoHdr->iFlags & 1)
			{
				uint32_t* offset = ((uint32_t*)aSymNameTbl+i);
				nameOffset = (*offset << 2);
				symbolName = strTable + nameOffset;
			}
			else
			{
				uint16_t* offset = ((uint16_t*)aSymNameTbl+i);
				nameOffset = (*offset << 2);
				symbolName = strTable + nameOffset;
			}
			printf("0x%08x \t%s\t\n",aSymAddrTbl[i], symbolName);
		}
	}
    else
		printf("No Symbol exported\n");
    printf("\n\n");

	if(symInfoHdr->iDllCount)
	{
	    char *iData = iE32->GetBufferedImage();
		// The import table orders the dependencies alphabetically...
		// We need to list out in the link order...
		printf("%d Static dependencies found\n", symInfoHdr->iDllCount);
		uint32_t* aDepTbl = (uint32_t*)((char*)symInfoHdr + symInfoHdr->iDepDllZeroOrdTableOffset);
		uint32_t* aDepOffset =  (uint32_t*)((char*)aDepTbl - iData);

		const E32ImportSection* isection = iE32->GetImportSection();

		/* The import table has offsets to the location (in code section) where the
		 * import is required. For dependencies pointed by 0th ordinal, this offset
		 * must be same as the offset of the dependency table entry (relative to
		 * the code section).
		 */
		for(int aDep = 0; aDep < symInfoHdr->iDllCount; aDep++)
		{
			const E32ImportBlock* b = (const E32ImportBlock*)(isection + 1);
			bool aZerothFound = false;
			for (int32_t d=0; d<iHdr1->iDllRefTableCount; d++)
			{
				char* dllname = iE32->GetDLLName(b->iOffsetOfDllName);
				int32_t n = b->iNumberOfImports;

				const uint32_t* p = b->Imports()+ (n - 1);//start from the end of the import table
				uint32_t impfmt = ImpFmtFromFlags(iHdr1->iFlags);;
				if (impfmt == KImageImpFmt_ELF)
				{
					while (n--)
					{
						uint32_t impd_offset = *p--;
						uint32_t impd = *(uint32_t*)(iData + iHdr1->iCodeOffset + impd_offset);
						uint32_t ordinal = impd & 0xffff;

						if (ordinal == 0 )
						{
							if( impd_offset == (*aDepOffset - iHdr1->iCodeOffset))
							{
								/* The offset in import table is same as the offset of this
								 * dependency entry
								 */
								printf("\t%s\n", dllname);
								aZerothFound = true;
							}
							break;
						}
					}
				}
				if(aZerothFound)
					break;

				b = b->NextBlock(impfmt);
			}
			if(!aZerothFound)
				printf("!!Invalid dependency listed at %d\n",aDep );

			aDepOffset++;
		}
	}
}

/** \brief This function prints in hex
 *
 * \param *pos - pointer to first element in memory
 * \param length - size of memory block to print
 * \return void
 * This function prints bytes in usual order like ordinal hex viewer.
 * That behavior differ from original algorithm (example with 4-byte block):
 * original - b5102802
 * usual    - 022810b5
 */

void E32Info::PrintHexData(void *pos, size_t length)
{
    printf("Block length: %lu\n", length);
    const unsigned LINE_MAX = 32;
    char *p = (char *)pos;
    char str[LINE_MAX + 1] = {"\n"};

    size_t i = 0;
    for(; (i + LINE_MAX) < length; i += LINE_MAX)
    {
        printf("%06x: ", i);
        memcpy(str, p + i, LINE_MAX);
        for(size_t j = 0; j < LINE_MAX; j++)
        {
            printf("%02x", (unsigned char)str[j]);

            if( (j+1)%4 == 0)
                printf(" ");

            if( ((size_t)str[j] < 32) || ((size_t)str[j] > 128))
                str[j] = '.';
        }
        printf("%s\n", str);
    }

    if(i==length)
        return;

    size_t diff = length-i;
    memset(str, ' ', LINE_MAX);
    memcpy(str, (p + i), diff);

    printf("%06x: ", i);
    for(uint32_t j = 0; j < diff; j++)
    {
        printf("%02x", (unsigned char)str[j]);

        if( (i+1)%4 == 0)
            printf(" ");

        if( ((size_t)str[j] < 32) || ((size_t)str[j] > 128))
            str[j] = '.';
    }
    printf("%s\n", str);
}

void E32Info::Run()
{
    if(iParam->E32Input())
    {
        iE32File = iParam->E32Input();
        iE32 = new E32Parser(iE32File);
        printf("E32ImageFile \'%s\'\n", iE32File);
        iHdr1 = iE32->GetFileLayout();

        ValidateE32Image(iE32->GetBufferedImage(), iE32->GetFileSize());
    }

    char c;
    while((c = *iFlags++))
    {
        switch(c)
        {
            case 'a':
                GenerateAsmFile(iParam);
                break;
            case 'h':
                HeaderInfo();
                break;
            case 's':
                SecurityInfo(true);
                break;
            case 'c':
                CodeSection();
                break;
            case 'd':
                DataSection();
                break;
            case 'e':
                ExportTable();
                break;
            case 'i':
                ImportTableInfo();
                break;
            case 't':
                SymbolInfo();
                break;
            default:
                throw Elf2e32Error(INVALIDARGUMENTERROR, iFlags, "--dump");
        }
    }
}

void E32Info::CPUIdentifier(uint16_t aCPUType, bool &isARM)
{
    switch (aCPUType)
    {
    case X86Cpu:
        printf("X86 CPU\n");
        break;
    case ArmV4Cpu:
        printf("ARMV4 CPU\n");
        isARM = true;
        break;
    case ArmV5Cpu:
        printf("ARMV5 CPU\n");
        isARM = true;
        break;
    case ArmV6Cpu:
        printf("ARMV6 CPU\n");
        isARM = true;
        break;
    case MCoreCpu:
        printf("M*Core CPU\n");
        break;
    case UnknownCpu: //fallthru
    default:
        printf("Unknown CPU\n");
        break;
    }
}

void DumpRelocs(char *relocs)
{
    int32_t cnt=((E32RelocSection *)relocs)->iNumberOfRelocs;
	printf("%d relocs\n", cnt);
	relocs+=sizeof(E32RelocSection);
	int32_t printed=0;
	while(cnt > 0)
    {
        int32_t page=*(uint32_t *)relocs;
		int32_t size=*(uint32_t *)(relocs+4);
		int32_t pagesize=size;
		size-=8;
		uint16_t *p=(uint16_t *)(relocs+8);
		while (size>0)
		{
			uint32_t a=*p++;
			uint32_t relocType = (a & 0x3000) >> 12;
			if ((relocType == 1) || (relocType == 3)) //only relocation type1 and type3
			{
				printf("%08x(%u) ", page + (a&0x0fff), relocType);
				printed++;
				if (printed>3)
				{
				    printf("\n");
					printed=0;
				}
			}
			size-=2;
			cnt--;
		}
		relocs+=pagesize;
    }
    printf("\n");
}

Symbols SymbolsFromDef(const char *defFile);
void GenerateAsmFile(ParameterManager *param)
{
    char *output = param->E32ImageOutput();

    char *defin = param->DefInput();
    if(!defin)
        throw Elf2e32Error(NOREQUIREDOPTIONERROR, "--definput");

	Symbols aSymList = SymbolsFromDef(defin);

	FILE *fptr = nullptr;
	if(output)
        fptr = freopen(output, "w", stdout);
	if(!fptr)
        printf("Can't store ASM in file! Print to screen.\n");

    for(auto x: aSymList)
    {
        if(x->Absent())
            continue;
        //Set the visibility of the symbols as default."DYNAMIC" option is
        //added to remove STV_HIDDEN visibility warnings generated by every
        //export during kernel build
        printf("\tIMPORT %s [DYNAMIC]\n", x->SymbolName());
    }

    // Create a directive section that instructs the linker to make all listed
    // symbols visible.
    printf("\n AREA |.directive|, READONLY, NOALLOC\n\n");
    printf("\tDCB \"#<SYMEDIT>#\\n\"\n");

    for(auto x: aSymList)
    {
        if(x->Absent())
            continue;
        // Example:
        //  DCB "EXPORT __ARM_ll_mlass\n"
        printf("\tDCB \"EXPORT %s\\n\"\n", x->SymbolName());
    }

    printf("\n END\n");
	if(fptr)
        fclose(fptr);
}
