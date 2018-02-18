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
// Implementation of the Class ElfExecutable for the elf2e32 tool
// @internalComponent
// @released
//
//

#if !defined(_PL_ELFEXECUTABLE_H_)
#define _PL_ELFEXECUTABLE_H_

#include "pl_common.h"
#include <list>
#include "elfdefs.h"
#include "pl_elfimports.h"
#include "pl_elfrelocations.h"
#include "pl_elfexports.h"
#include "parametermanager.h"

using std::list;

class Symbol;
class ElfRelocations;
class ElfExports;
class ParameterListInterface;
class ElfLocalRelocation;
//static PLUINT32 globalcntr = 0;
/**
This class is for ELF object carrying the elf header, sections, segments.
@internalComponent
@released
*/
class ElfExecutable
{
public:
	explicit ElfExecutable(ParameterManager* aParameterManager);
	virtual ~ElfExecutable();

	PLUINT32 ProcessElfFile(Elf32_Ehdr *aElfHdr);

	PLUINT32 ProcessSymbols();
	ElfImports::ImportLibs GetImports();
	ElfExports* GetExports();
	Symbol* AddToExports(char* aDll, Symbol* aSymbol);
	void AddToImports(ElfImportRelocation* aReloc);
	PLUINT32 AddToRelocations(ElfRelocation* aReloc);
	void AddToLocalRelocations(ElfRelocation* aReloc);
	void ProcessVerInfo();

	Elf32_Sym* FindSymbol(char* aSymName);

	PLUINT32 GetSymbolOrdinal( char* aSymName);
	PLUINT32 GetSymbolOrdinal( Elf32_Sym* );
	char* GetSymbolName( PLUINT32 aSymIdx);
	Elf32_Word GetRelocationOffset(ElfRelocation * aReloc);
	Elf32_Word *GetRelocationPlace(ElfRelocation * aReloc);

	Elf32_Word GetROBase();
	Elf32_Word GetRWBase();
	uint32_t GetROSize();
	uint32_t GetRWSize();
	uint32_t GetBssSize();
	MemAddr GetRawROSegment();
	MemAddr GetRawRWSegment();
	Elf32_Word EntryPointOffset();
	ESegmentType SegmentType(Elf32_Addr aAddr);
	Elf32_Phdr* GetSegmentAtAddr(Elf32_Addr aAddr);
	ElfRelocations::Relocations & GetCodeRelocations();
	ElfRelocations::Relocations & GetDataRelocations();
	bool ExeceptionsPresentP();
	ElfExports::ExportList& GetExportsInOrdinalOrder();
	Elf32_Sym* LookupStaticSymbol(char * aName);
public:
	/**
	 * The elf header pointer which points to the base of the file records
	 */
	Elf32_Ehdr	*iElfHeader = nullptr;
	Elf32_Addr	iEntryPoint = 0;
	/**
	 * The dynamic program header of the elf file
	 */
	Elf32_Phdr	*iProgHeader = nullptr;

	PLUINT32	iSONameOffset = 0;
	char		*iSOName = nullptr;
	/**
	 * This member points to the base of the section header table.
	 */
	Elf32_Shdr		*iSections = nullptr;
	Elf32_Verdef	*iVersionDef = nullptr;
	PLUINT32		iVerDefCount = 0;
	Elf32_Verneed	*iVersionNeed = nullptr;
	PLUINT32		iVerNeedCount = 0;
	Elf32_Half		*iVersionTbl = nullptr;
	PLUINT32		iRelSize = 0;
	PLUINT32		iRelEntSize = 0;
	PLUINT32		iNRelocs = 0;
	Elf32_Rel		*iRel = nullptr;
	PLUINT32		iRelaSize = 0;
	PLUINT32		iRelaEntSize = 0;
	Elf32_Rela		*iRela = nullptr;
	char			*iStringTable = nullptr;
	char			*iSectionHdrStrTbl = nullptr;

	list<char*>		iNeeded;
	VersionInfo		*iVerInfo = nullptr;
	/**
	 * The dynamic symbol array.
	 */
	Elf32_Sym		*iElfDynSym = nullptr;//The ELF symbol

	/**
	 * The static symbol table.
	 */
	Elf32_Sym *iSymTab = nullptr;
	char *iStrTab = nullptr;
	Elf32_Sym *iLim = nullptr;

	PLUINT32		iNSymbols = 0;
	Elf32_HashTable	*iHashTbl = nullptr;
	Elf32_Phdr		*iDynSegmentHdr = nullptr;
	Elf32_Phdr		*iDataSegmentHdr = nullptr;
	MemAddr			iDataSegment = nullptr;
	uint32_t		iDataSegmentSize = 0;
	PLUINT32		iDataSegmentIdx = 0;
	Elf32_Phdr		*iCodeSegmentHdr = nullptr;
	MemAddr			iCodeSegment = nullptr;
	uint32_t		iCodeSegmentSize = 0;
	PLUINT32		iCodeSegmentIdx = 0;
	ElfImports		iImports;
	ElfExports		*iExports = nullptr;
	ElfRelocations  iElfRelocations;
	ParameterManager *iParameterManager;
	PLUINT32		iPltGotBase = 0;
	PLUINT32		iPltGotLimit = 0;
	PLUINT32		iStrTabSz = 0;
	PLUINT32		iSymEntSz = 0;
	Elf32_Word		*iPltGot = nullptr;
	PLUINT32		iPltRelType = 0;
	Elf32_Rel		*iPltRel = nullptr;
	PLUINT32		iPltRelaSz = 0;
	Elf32_Rela		*iPltRela = nullptr;
	PLUINT32		iPltRelSz = 0;
	PLUINT32		iJmpRelOffset = 0;

	PLUINT32	ValidateElfFile();
	PLUINT32	ProcessDynamicEntries();
	void		ProcessRelocations();
	template <class T> void	ProcessRelocations(T *aElfRel, size_t aSize);

	VersionInfo* GetVersionInfo(PLUINT32 aIndex);
	char*		SymbolDefinedInDll(PLUINT32 aSymbolIndex);
	void 		SetVersionRecord( ElfRelocation* aReloc );

/** This function says if the Symbol is a global symbol
@return It returns true if the Symbol is a global one.
@param aSym The reference to the Symbol whose attribute is being checked
*/
	bool GlobalSymbol(Elf32_Sym* aSym);

/** This function says if the Symbol is Exported
@return It returns true if the Symbol is an exported one.
@param aSym The reference to the Symbol whose attribute is being checked
*/
	bool ExportedSymbol(Elf32_Sym* aSym);

/** This function says if the Symbol is Imported
@return It returns true if the Symbol is an imported one.
@param aSym The reference to the Symbol whose attribute is being checked
*/
	bool ImportedSymbol(Elf32_Sym* aSym);

/** This function says if the Symbol is a Code Symbol
@return It returns true if the Symbol is of Code type.
@param aSym The reference to the Symbol whose attribute is being checked
*/
	bool FunctionSymbol(Elf32_Sym* aSym);

/** This function says if the Symbol is a Data Symbol
@return It returns true if the Symbol is of Data type.
@param aSym The reference to the Symbol whose attribute is being checked
*/
	bool DataSymbol(Elf32_Sym* aSym);

/** This function says if the Symbol is Defined
@return It returns true if the Symbol is defined.
@param aSym The reference to the Symbol whose attribute is being checked
*/
	bool DefinedSymbol(Elf32_Sym* aSym);

/** This function says if the Symbol has a default Visibility
@return It returns true if the Symbol has a default Visibility
@param aSym The reference to the Symbol whose attribute is being checked
*/
	bool VisibleSymbol(Elf32_Sym* aSym);

/** This function finds the segment in which this address belongs
@return the segment type
@param aAddr The address within the executable
*/

	Elf32_Word Addend(Elf32_Rel* aRel);
	Elf32_Word Addend(Elf32_Rela* aRel);

	char* SymbolFromDSO(PLUINT32  aSymbolIndex);
	Elf32_Word* GetFixupLocation(ElfLocalRelocation* aReloc, Elf32_Addr aPlace);
	ESegmentType Segment(Elf32_Sym *aSym);
	Elf32_Phdr* Segment(ESegmentType aType);

/** This function processes the linker generated Veneer symbols and
 *  creates a relocation entry.
 */
	void ProcessVeneers();
/** This function processes the ELF file to find the static symbol table.
*/
	void FindStaticSymbolTable();
/** This function finds the .comment section
@return the pointer to the comment section
*/
	char* FindCommentSection();
/** This function finds the value at the address passed in
@return the value at the address passed in
@param aAddr The address within the executable
*/
	Elf32_Word FindValueAtLoc(Elf32_Addr aOffset);
};


#endif // !defined(_PL_ELFEXECUTABLE_H_)
