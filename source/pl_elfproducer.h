// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017 Strizhniou Fiodar.
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

#if !defined(_PL_ELFPRODUCER_H_)
#define _PL_ELFPRODUCER_H_

#include "pl_elfimage.h"
#include <string>

//enum for section index
typedef enum _SECTION_INDEX_ {
	DUMMY_SECTION=0,
	CODE_SECTION,
	DYNAMIC_SECTION,
	HASH_TBL_SECTION,
	VER_DEF_SECTION,
	VERSION_SECTION,
	STRING_SECTION,
	SYMBOL_SECTION,
	SH_STR_SECTION,
	MAX_SECTIONS=SH_STR_SECTION
}SECTION_INDEX;

//enum for DYN entries
typedef enum _DYN_ENTRIES_ {
	DSO_DT_DSONAME=0,
	DSO_DT_SYMTAB,
	DSO_DT_SYMENT,
	DSO_DT_STRTAB,
	DSO_DT_STRSZ,
	DSO_DT_VERSYM,
	DSO_DT_VERDEF,
	DSO_DT_VERDEFNUM,
	DSO_DT_HASH,
	DSO_DT_NULL,
	MAX_DYN_ENTS=DSO_DT_NULL,
}DYN_ENTRIES;

using std::string;

#define DEFAULT_VERSION	2
/**
This class provides elf contents for the dso file creation. It generates the
elf header, sections, dynamic array and the relocatrion entries.
@internalComponent
@released
*/
class ElfProducer : public ElfImage
{

	typedef std::list<Symbol*> SymbolList;
public:
	explicit ElfProducer(std::string aElfInput);
	~ElfProducer();

	void SetSymbolList(SymbolList& aSymbolList);
	void WriteElfFile(char* aDsoFullName,char* aDsoFileName, char* aLinkAs);

private:
	/**
	 * This list is passed from the UseCaseHandler via the DSOHandler to this class.
	 * This is used to create the export info when only a def file is available and
	 * the DSO needs to be generated for that.
	 */

	/** The proxy DSO file name*/
	string			iDSOName;

	/** The offset of the file name within the string table of the ELF file*/
	PLUINT32		iDSONameOffset=0;

	/** The DLL name that defines the export Symbols*/
	string			iLinkAs;

	/** The offset of the DLL name within the string table within the ELF file*/
	PLUINT32		iLinkAsOffset=0;

	/** The export Symbol list*/
	SymbolList		*iSymbolsList=nullptr;

	/** The proxy DSO full file name*/
	string			iDsoFullName;

	/*DSO content Fields*/

	/** The Elf version definition auxiliary section*/
	Elf32_Verdaux	*iDSODaux=nullptr;

	/** The Buckets for the hash table*/
	Elf32_Sword		*iDSOBuckets=nullptr;

	/** The chains pointed to by the buckets belonging to the hash table*/
	Elf32_Sword		*iDSOChains=nullptr;

	/** The Elf Dynamic section table*/
	Elf32_Dyn		iDSODynTbl[MAX_DYN_ENTS+1];

	/** The code section*/
	PLUINT32		*iCodeSectionData=nullptr;

	/** The Elf file offset maintained for writing the different sections of the Elf file*/
	PLUINT32		iElfFileOffset=0;

	/** The Elf string table*/
	string			iDSOSymNameStrTbl;

	/** The Elf Section-header string table*/
	string			iDSOSectionNames;

	void InitElfContents();
	void Cleanup();
	void SetSymolFields(Symbol *aSym, Elf32_Sym* aElfSym, PLUINT32 aIndex);
	void AddToHashTable(const char* aSymName, PLUINT32 aIndex);
	void CreateVersionTable();
	void CreateElfHeader();
	void CreateSections();
	void SetSectionFields(PLUINT32 aSectionIndex, char* aSectionName,
						PLUINT32 aType, PLUINT32 aEntSz, PLUINT32 aSectionSize,
						PLUINT32 aLink, PLUINT32 aInfo, PLUINT32 aAddrAlign,
						PLUINT32 aFlags, PLUINT32 aAddr);
	void CreateDynamicEntries();
	void CreateProgHeader();

	void WriteElfContents();
	void AlignString(string& aStr);
};

#endif // !defined(_PL_ELFPRODUCER_H_)
