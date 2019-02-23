// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2018 Strizhniou Fiodar.
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
// Implementation of the Class ElfProducer for the elf2e32 tool
// @internalComponent
// @released
//
//

#include <stdio.h>
#include <cstring>
#include "pl_symbol.h"
#include "errorhandler.h"
#include "pl_elfproducer.h"

void InfoPrint(const char* hdr, uint32_t& pos, const uint32_t offset);

/**
 * Following array is indexed on the SECTION_INDEX enum
 */
const char* SECTION_NAME[] = {
	"",
	"ER_RO",
	".dynamic",
	".hash",
	".version_d",
	".version",
	".strtab",
	".dynsym",
	".shstrtab"
};

/**
Constructor for class ElfProducer
@param aElfInput - name of input elf file
@internalComponent
@released
*/
ElfProducer::ElfProducer(string aElfInput):
     ElfImage(aElfInput) {}

/**
Destructor for class ElfProducer to release allocated memory
@internalComponent
@released
*/
ElfProducer::~ElfProducer(){
    DELETE_PTR(iElfHeader);
	DELETE_PTR_ARRAY(iSections);
	DELETE_PTR_ARRAY(iElfDynSym);
	DELETE_PTR_ARRAY(iVersionTbl);
	DELETE_PTR_ARRAY(iVersionDef);
	DELETE_PTR_ARRAY(iDSODaux);

	DELETE_PTR_ARRAY(iProgHeader);
	DELETE_PTR_ARRAY(iCodeSectionData);
	DELETE_PTR_ARRAY(iHashTbl);
	DELETE_PTR_ARRAY(iDSOBuckets);
	DELETE_PTR_ARRAY(iDSOChains);
}

/**
This function sets the export Symbol list
@internalComponent
@released
@param aSymbolList The export Symbol list.
*/
void ElfProducer::SetSymbolList(Symbols& s)
{
	iSymbols = s;
	iNSymbols = iSymbols.size() + 1;

	if (iSymbols.empty())
        return;

    const char *absentSymbol = "_._.absent_export_";
    int length = strlen(absentSymbol);

    // Ordinal Number can be upto 0xffff which is 6 digits
    length +=7;
    for(auto x: iSymbols)
    {
        /* If a symbol is marked as Absent in the DEF file, replace the
         * symbol name with "_._.absent_export_<Ordinal Number>"
         */
        if(x->Absent())
        {
            char *symName = new char[length]{'0'};
            sprintf(symName, "_._.absent_export_%d", x->OrdNum());
            x->SetSymbolName(symName);
            delete[] symName;
        }
    }
}

/**
This function takes the file names and generates the proxy dso library.
@internalComponent
@released
@param aDsoFullName The full path and proxy-dso file name
@param aDsoFileName The proxy-dso file name
@param aLinkAs The DLL that defines the export Symbols
*/
void ElfProducer::WriteElfFile(char* aDsoFullName, char* aDsoFileName , char* aLinkAs){

	//This includes the full path followed by the file name
	iDsoFullName = aDsoFullName;

	iDSOName = aDsoFileName;
	iLinkAs = aLinkAs;

	InitElfContents();

	WriteElfContents();
}

/**
This function initializes the Elf members
@internalComponent
@released
*/
void ElfProducer::InitElfContents() {

	iElfHeader		= new Elf32_Ehdr();
	iSections		= new Elf32_Shdr[MAX_SECTIONS+1]();

	iElfDynSym		= new Elf32_Sym[iNSymbols]();
	iVersionTbl		= new Elf32_Half[iNSymbols]();
	iVersionDef		= new Elf32_Verdef[2];
	iDSODaux		= new Elf32_Verdaux[2];

	iProgHeader		 = new Elf32_Phdr[2];
	iCodeSectionData = new PLUINT32[iNSymbols]();

	iHashTbl = new Elf32_HashTable();

	//premeditated
	iHashTbl->nBuckets = (iNSymbols /3) + (iNSymbols % 0x3);

	iHashTbl->nChains = iNSymbols;

	iDSOBuckets = new Elf32_Sword[iHashTbl->nBuckets]();
	iDSOChains = new Elf32_Sword[iHashTbl->nChains]();

	CreateElfHeader();

	PLUINT32	aIdx = 1;

	iDSOSymNameStrTbl.insert(iDSOSymNameStrTbl.end(), 0);

	for(auto aSym: iSymbols)
    {
		string aSymName(aSym->SymbolName());
		//set symbol info..
		iElfDynSym[aIdx].st_name = iDSOSymNameStrTbl.size();

		iDSOSymNameStrTbl.insert(iDSOSymNameStrTbl.end(), aSymName.begin(), aSymName.end() );
		iDSOSymNameStrTbl.insert(iDSOSymNameStrTbl.end(), 0);

		SetSymbolFields( aSym, &iElfDynSym[aIdx], aIdx);

		//set version table info...
		iVersionTbl[aIdx] = DEFAULT_VERSION;
		AddToHashTable(aSym->SymbolName(), aIdx);
        ++aIdx;
	}

	CreateVersionTable();

	//Fill section headers...
	CreateSections();

	//Copy dyn entries..
	CreateDynamicEntries();

	//create code section data - this has the ordinal numbers...
	CreateProgHeader();
}

/**
This function creates the version definition table
@internalComponent
@released
*/
void ElfProducer::CreateVersionTable()
{
	//Fill verdef table...
	iVersionDef[0].vd_ndx = 1;
	iVersionDef[0].vd_cnt = 1;
	iVersionDef[0].vd_flags = 1;
	iVersionDef[0].vd_hash = elf_hash((const PLUCHAR*) iDSOName.c_str());
	iVersionDef[0].vd_version = 1;

	iVersionDef[0].vd_aux = sizeof(Elf32_Verdef);
	iVersionDef[0].vd_next = sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux);

	iDSONameOffset = iDSOSymNameStrTbl.size();

	iDSOSymNameStrTbl.insert(iDSOSymNameStrTbl.end(),iDSOName.begin(), iDSOName.end());
	iDSOSymNameStrTbl.insert(iDSOSymNameStrTbl.end(), 0);

	iDSODaux[0].vda_name = iDSONameOffset;
	iDSODaux[0].vda_next = 0;

	iVersionDef[1].vd_ndx = DEFAULT_VERSION;
	iVersionDef[1].vd_cnt = 1;
	iVersionDef[1].vd_flags = 0;
	iVersionDef[1].vd_hash = elf_hash((const PLUCHAR*)iLinkAs.c_str());
	iVersionDef[1].vd_version = 1;

	iVersionDef[1].vd_aux = sizeof(Elf32_Verdef);
	iVersionDef[1].vd_next = 0;

	iLinkAsOffset = iDSOSymNameStrTbl.size();
	iDSOSymNameStrTbl.insert(iDSOSymNameStrTbl.end(),iLinkAs.begin(), iLinkAs.end());
	iDSOSymNameStrTbl.insert(iDSOSymNameStrTbl.end(), 0);

	iDSODaux[1].vda_name = iLinkAsOffset;
	iDSODaux[1].vda_next = 0;

}

/**
This function sets the Elf Symbol fields
@internalComponent
@released
@param aSym The Symbol representation
@param aElfSym The Elf Symbol
@param aCodeIndex The index at which this Symbol refers to in the code section where, we have the ordinal number
*/
void ElfProducer::SetSymbolFields(Symbol *aSym, Elf32_Sym* aElfSym, PLUINT32 aCodeIndex) {

	aElfSym->st_other = STV_DEFAULT;

	aElfSym->st_info = (PLUCHAR) (ELF32_ST_INFO(STB_GLOBAL, aSym->CodeDataType()));
	aElfSym->st_value	= (aCodeIndex - 1)*sizeof(Elf32_Word);

	if(aSym->CodeDataType() == SymbolTypeCode){
		aElfSym->st_size	= sizeof(Elf32_Word);
	}else{
		aElfSym->st_size	= aSym->SymbolSize();
	}
	aElfSym->st_shndx = CODE_SECTION;
}

/**
This function adds an entry into the hash table based on the symbol name.
@internalComponent
@released
@param aSymName The Symbol name
@param aIndex The Symbol index in the Symbol table
*/
void ElfProducer::AddToHashTable(const char* aSymName, PLUINT32 aIndex)
{
	PLULONG	hsh = elf_hash((PLUCHAR*)aSymName);
	PLUINT32  aBIdx = hsh % iHashTbl->nBuckets;

	if(iDSOBuckets[aBIdx] == 0)
	{
		iDSOBuckets[aBIdx] = aIndex;
	}
	else
	{
		PLUINT32 aCIdx = iDSOBuckets[aBIdx];

		while(iDSOChains[aCIdx] != 0){

			/* If the entry is already added into the hash table*/
			if((PLUINT32)iDSOChains[aCIdx] == aIndex) {
				return;
			}
			aCIdx = iDSOChains[aCIdx];
		}
		iDSOChains[aCIdx] = aIndex;
	}
}

/**
This function creates the different sections of the Elf file
@internalComponent
@released
*/
void ElfProducer::CreateSections() {
	PLUINT32	aIdx = 0;

	//clear the first section...
	memset(&iSections[0], 0, sizeof(Elf32_Shdr));
	iDSOSectionNames.insert(iDSOSectionNames.begin(), 0);

	// Set the ELF file offset.
	// This indicates the start of sections.
	iElfFileOffset = sizeof(Elf32_Ehdr);

	iElfFileOffset += (sizeof(Elf32_Shdr) * iElfHeader->e_shnum);

	//Check if the string table is 4-byte aligned..
	AlignString(iDSOSymNameStrTbl);

	for(aIdx = 1; aIdx <= MAX_SECTIONS; aIdx++) {
		switch(aIdx)
		{
			case SYMBOL_SECTION:
				SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_DYNSYM, \
								   sizeof(Elf32_Sym), (iNSymbols * sizeof(Elf32_Sym)),\
								   STRING_SECTION, CODE_SECTION, 4, 0, 0);
				break;
			case STRING_SECTION:
				SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_STRTAB, \
								   1, iDSOSymNameStrTbl.size(), 0, \
								   0, 0, 0,0);
				break;
			case VERSION_SECTION:
				SetSectionFields(aIdx, SECTION_NAME[aIdx], 0x6fffffff, \
								   sizeof(Elf32_Half), (iNSymbols * sizeof(Elf32_Half)), SYMBOL_SECTION, \
								   0, 2, 0, 0);
				break;
			case VER_DEF_SECTION:
				SetSectionFields(aIdx, SECTION_NAME[aIdx],0x6ffffffd, \
								   sizeof(Elf32_Verdaux), (2*(sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux))),\
								   STRING_SECTION, DYNAMIC_SECTION, 4, 0, 0);
				break;
			case HASH_TBL_SECTION:
				{
					PLUINT32 aSize = sizeof(Elf32_HashTable) + \
							(sizeof(Elf32_Sword) * (iHashTbl->nBuckets + iHashTbl->nChains));

				SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_HASH, \
								   0, aSize, SYMBOL_SECTION, 0, 4, 0, 0);
				}
				break;
			case DYNAMIC_SECTION:
				SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_DYNAMIC, \
								   sizeof(Elf32_Dyn), ((MAX_DYN_ENTS + 1) *sizeof(Elf32_Dyn)),\
								   STRING_SECTION, 0, 4, 0,0);
				break;
			case CODE_SECTION:
				SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_PROGBITS, \
								   0, (iNSymbols *sizeof(PLUINT32)),\
								   0, 0, 4, (SHF_ALLOC | SHF_EXECINSTR),0);
				break;
			case SH_STR_SECTION:
				{
				PLUINT32 aSize = iDSOSectionNames.size() + strlen(SECTION_NAME[aIdx]);
				SetSectionFields(aIdx, SECTION_NAME[aIdx], SHT_STRTAB, \
								   1, aSize, 0, \
								   0, 0, 0, 0);
				//Check if the string table is 4-byte aligned..
				AlignString(iDSOSectionNames);
				iSections[aIdx].sh_size = iDSOSectionNames.size();

				}
				break;
			default:
				break;
		}
		iSections[aIdx].sh_offset = iElfFileOffset;
		iElfFileOffset += iSections[aIdx].sh_size;

		if(iElfFileOffset %4) {
			iElfFileOffset += (4 - (iElfFileOffset %4));
		}
	}
}

/**
This function sets the section header fields.
@internalComponent
@released
@return Error status
@param aSectionIndex The index of the section
@param aSectionName The name of the section
@param aType The type of the section
@param aEntSz The size of each entry of the section
@param aSectionSize The size of the section
@param aLink The section this section is linked to
@param aInfo Extra information. Depends on the section type of this section.
@param aAddrAlign The address alignment of this section.
@param aFlags Section flags
@param aAddr The address of this section in memory(Here it remains 0)
*/
void ElfProducer::SetSectionFields(PLUINT32 aSectionIndex, const char* aSectionName, PLUINT32 aType, \
								   PLUINT32 aEntSz, PLUINT32 aSectionSize, PLUINT32 aLink, \
								   PLUINT32 aInfo, PLUINT32 aAddrAlign, PLUINT32 aFlags, \
								   PLUINT32 aAddr)
{
	string aSecName = aSectionName;

	iSections[aSectionIndex].sh_name			= iDSOSectionNames.size();
	iDSOSectionNames.insert(iDSOSectionNames.end(), aSecName.begin(), aSecName.end());
	iDSOSectionNames.insert(iDSOSectionNames.end(), 0);

	iSections[aSectionIndex].sh_type			= aType;
	iSections[aSectionIndex].sh_entsize		= aEntSz;
	iSections[aSectionIndex].sh_size			= aSectionSize;
	iSections[aSectionIndex].sh_link			= aLink;
	iSections[aSectionIndex].sh_flags		= aFlags;
	iSections[aSectionIndex].sh_addralign	= aAddrAlign;
	iSections[aSectionIndex].sh_info			= aInfo;
	iSections[aSectionIndex].sh_addr			= aAddr;
}

/**
This function creates the dynamic sections.
@internalComponent
@released
*/
void ElfProducer::CreateDynamicEntries()
{
	for(PLUINT32 aIdx = 0; aIdx <= MAX_DYN_ENTS; aIdx++ ) {
		switch(aIdx) {
			case DSO_DT_DSONAME:
				iDSODynTbl[aIdx].d_tag = DT_SONAME;
				iDSODynTbl[aIdx].d_val = iDSONameOffset;
				break;
			case DSO_DT_SYMTAB:
				iDSODynTbl[aIdx].d_tag = DT_SYMTAB;
				iDSODynTbl[aIdx].d_val = iSections[SYMBOL_SECTION].sh_offset;
				break;
			case DSO_DT_SYMENT:
				iDSODynTbl[aIdx].d_tag = DT_SYMENT;
				iDSODynTbl[aIdx].d_val = iSections[SYMBOL_SECTION].sh_entsize;
				break;
			case DSO_DT_STRTAB:
				iDSODynTbl[aIdx].d_tag = DT_STRTAB;
				iDSODynTbl[aIdx].d_val = iSections[STRING_SECTION].sh_offset;
				break;
			case DSO_DT_STRSZ:
				iDSODynTbl[aIdx].d_tag = DT_STRSZ;
				iDSODynTbl[aIdx].d_val = iDSOSymNameStrTbl.size();
				break;
			case DSO_DT_VERSYM:
				iDSODynTbl[aIdx].d_tag = DT_VERSYM;
				iDSODynTbl[aIdx].d_val = iSections[VERSION_SECTION].sh_offset;
				break;
			case DSO_DT_VERDEF:
				iDSODynTbl[aIdx].d_tag = DT_VERDEF;
				iDSODynTbl[aIdx].d_val = iSections[VER_DEF_SECTION].sh_offset;
				break;
			case DSO_DT_VERDEFNUM:
				iDSODynTbl[aIdx].d_tag = DT_VERDEFNUM;
				iDSODynTbl[aIdx].d_val = 2;
				break;
			case DSO_DT_HASH:
				iDSODynTbl[aIdx].d_tag = DT_HASH;
				iDSODynTbl[aIdx].d_val = iSections[HASH_TBL_SECTION].sh_offset;
				break;
			case DSO_DT_NULL:
				iDSODynTbl[aIdx].d_tag = DT_NULL;
				iDSODynTbl[aIdx].d_val = 0;
				break;
			default:
				break;
		}
	}
}

/**
This function creates the Elf header.
@internalComponent
@released
*/
void ElfProducer::CreateElfHeader()
{
	//create ELF header
	unsigned char c[EI_NIDENT] = {0x7f, 'E', 'L', 'F', \
				ELFCLASS32, ELFDATA2LSB, 1, 0, \
				0, 0, 0, 0, \
				0, 0, 0, 0};       // e_ident

	for (PLUINT32 i=0; i <EI_NIDENT;i++)
		iElfHeader->e_ident[i] = c[i];

	iElfHeader->e_type		= ET_DYN;
	iElfHeader->e_machine	= EM_ARM;
	iElfHeader->e_version	= EV_CURRENT;
	iElfHeader->e_entry		= 0;
	iElfHeader->e_shoff		= sizeof(Elf32_Ehdr);
	iElfHeader->e_flags		= EF_ARM_BPABI_VERSION | EF_ARM_SYMSARESORTED;
	iElfHeader->e_ehsize	= sizeof(Elf32_Ehdr);
	iElfHeader->e_phentsize = sizeof(Elf32_Phdr);
	iElfHeader->e_shentsize = sizeof(Elf32_Shdr);
	iElfHeader->e_shnum		= MAX_SECTIONS + 1;
	iElfHeader->e_shstrndx	= SH_STR_SECTION;
	iElfHeader->e_phnum		= 2;
}

/**
This function creates the program header
@internalComponent
@released
*/
void ElfProducer::CreateProgHeader()
{
	//Update the program header offset..
	iElfHeader->e_phoff		= iElfFileOffset;

	// Update code section data..
	PLUINT32	aPos = 0;
	for(auto x: iSymbols)
    {
		iCodeSectionData[aPos++] = x->OrdNum();
	}

	//Create program header
	iProgHeader[0].p_align	= 4;
	iProgHeader[0].p_offset = iSections[CODE_SECTION].sh_offset;
	iProgHeader[0].p_type	= PT_LOAD;
	iProgHeader[0].p_flags	= (PF_X | PF_ARM_ENTRY);
	iProgHeader[0].p_filesz = iSections[CODE_SECTION].sh_size;
	iProgHeader[0].p_paddr	= 0;
	iProgHeader[0].p_vaddr	= 0;
	iProgHeader[0].p_memsz  = iSections[CODE_SECTION].sh_size;

	iProgHeader[1].p_align	= 4;
	iProgHeader[1].p_offset = iSections[DYNAMIC_SECTION].sh_offset;
	iProgHeader[1].p_type	= PT_DYNAMIC;
	iProgHeader[1].p_flags	= (PF_R );
	iProgHeader[1].p_filesz = iSections[DYNAMIC_SECTION].sh_size;
	iProgHeader[1].p_paddr	= 0;
	iProgHeader[1].p_vaddr	= 0;
	iProgHeader[1].p_memsz  = 0;

}

/**
This function aligns the string table to a 4-byte boundary
@internalComponent
@released
@return Error status
@param aStr - string to be aligned
*/
void ElfProducer::AlignString(string& aStr) {

	if( aStr.size() %4 ){
		PLUCHAR	aPad = (PLUCHAR)(4 - (aStr.size() %4));

		while(aPad--) {
			aStr.insert(aStr.end(), 0);
		}
	}
}

/**
This function writes the Elf file contents.
@internalComponent
@released
*/
void ElfProducer::WriteElfContents()
{
	FILE		*elf = nullptr;
	PLUINT32	index = 0;

	if((elf = fopen(iDsoFullName.c_str(), "wb")) == NULL ) {
		throw Elf2e32Error(FILEOPENERROR, iDsoFullName);
	}

	// The ELF header..
    fwrite(iElfHeader, 1, sizeof(Elf32_Ehdr), elf);
	uint32_t pos = 0, offset = sizeof(Elf32_Ehdr);
    InfoPrint("Elf Header starts", pos, sizeof(Elf32_Ehdr));

	//Section headers
	for(index = 0; index <= MAX_SECTIONS; index++) {
		fwrite(&iSections[index], 1, sizeof(Elf32_Shdr), elf);
	}
	InfoPrint("Section Headers", pos, sizeof(Elf32_Shdr) * index);


	//Each section..

		//code
		for(index = 0; index < iNSymbols; index++) {
			fwrite(&iCodeSectionData[index], 1, sizeof(PLUINT32), elf );
		}
        InfoPrint(" Code sections", pos, sizeof(PLUINT32) * iNSymbols);

		//dyn table
		for(index = 0; index <= MAX_DYN_ENTS; index++) {
			fwrite(&iDSODynTbl[index], 1, sizeof(Elf32_Dyn), elf);
		}
        InfoPrint(" Dyn table", pos, sizeof(Elf32_Dyn) * index);

		//hash table
		fwrite(&iHashTbl->nBuckets, 1, sizeof(Elf32_Word), elf);
		fwrite(&iHashTbl->nChains, 1, sizeof(Elf32_Word), elf);

		for(index=0; index < iHashTbl->nBuckets; index++) {
			fwrite(&iDSOBuckets[index], 1, sizeof(Elf32_Sword), elf);

		}
		for(index=0; index < iHashTbl->nChains; index++) {
			fwrite(&iDSOChains[index], 1, sizeof(Elf32_Sword), elf);

		}
        InfoPrint(" Hash table", pos, sizeof(Elf32_Word) * 2 +
            iHashTbl->nBuckets * sizeof(Elf32_Sword) +
            iHashTbl->nChains * sizeof(Elf32_Sword));

		//version def table
		for(index = 0; index < 2; index++) {
			fwrite(&iVersionDef[index], 1, sizeof(Elf32_Verdef), elf);
			fwrite(&iDSODaux[index], 1, sizeof(Elf32_Verdaux), elf);
		}
        InfoPrint(" Version def table", pos,
                  (sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux)) * 2);

		//version table
		for(index = 0; index < iNSymbols; index++) {
			fwrite(&iVersionTbl[index], 1, sizeof(Elf32_Half), elf );
		}

		if( iSections[VERSION_SECTION].sh_size %4 ) {
			PLUINT32 aNPads = 4 - (iSections[VERSION_SECTION].sh_size %4);
			PLUCHAR aPad = '\0';
			while(aNPads--) {
				fwrite(&aPad, 1, 1, elf);
			}
		}
        InfoPrint(" Version table", pos,
            sizeof(Elf32_Half) * iNSymbols + 4 -
                  (iSections[VERSION_SECTION].sh_size %4));

		//string table
		PLUINT32 aSz = iDSOSymNameStrTbl.size();
		char *aData = new char[aSz];
		memcpy(aData, iDSOSymNameStrTbl.data(), aSz);
		fwrite(aData, 1, aSz, elf);
        InfoPrint(" String table", pos, aSz);

		DELETE_PTR_ARRAY(aData);

		//Sym table
		for(index = 0; index < iNSymbols; index ++) {
			fwrite(&iElfDynSym[index], 1, sizeof(Elf32_Sym), elf);
		}
        InfoPrint(" Sym table", pos, sizeof(Elf32_Sym) * iNSymbols);

		//section header name table
		aSz = iDSOSectionNames.size();
		char *aData1 = new char[ aSz];
		memcpy(aData1, iDSOSectionNames.data(), aSz);
		fwrite(aData1, 1, aSz, elf);
        InfoPrint(" Section header", pos, aSz);
		DELETE_PTR_ARRAY(aData1);

	//program header
	for(index = 0; index < 2; index++) {
		fwrite(&iProgHeader[index], 1, sizeof(Elf32_Phdr), elf );
	}
    InfoPrint("Program header", pos, sizeof(Elf32_Phdr) * 2);
#ifdef EXPLORE_DSO_BUILD
    printf("Filesize: %d\n", pos);
#endif // EXPLORE_DSO_BUILD

	fclose(elf);
}

void InfoPrint(const char* hdr, uint32_t& pos, const uint32_t offset)
{
#ifdef EXPLORE_DSO_BUILD
    printf("%s starts at: %08x and ends at: %08x"
       " with size: %06x\n\n", hdr, pos, pos + offset, offset);
    pos += offset;
#endif // EXPLORE_DSO_BUILD
}

