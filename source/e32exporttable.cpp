// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2018 Strizhniou Fiodar
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
//

#include <string.h>
#include "e32exporttable.h"
#include <iostream>
#include <stdint.h>

#include "pl_elfimage.h"
#include "pl_elfexports.h"
#include "pl_elflocalrelocation.h"
#include "pl_symbol.h"

/**
Destructor for E32ExportTable class.
@internalComponent
@released
*/
E32ExportTable::~E32ExportTable()
{
	delete [] iTable;
}

/**
This function updates the e32 export table.
@param aElfImage - Elf executable
@param aExportList - export symbol list
@internalComponent
@released
*/
void E32ExportTable::CreateExportTable(ElfImage * aElfImage, ElfExports::Exports & aExports)
{
	iElfImage = aElfImage;
	// ELFExports::Exports aExports = aElfImage->GetExportsInOrdinalOrder();
	// The export table has a header containing the number of entries
	// before the entries themselves. So add 1 to number of exports
	iNumExports = aExports.size();
	size_t aSize = iNumExports + 1;
	iSize = aSize * sizeof(MemAddr);

	if(sizeof(MemAddr) == 8) // on windows sizeof(MemAddr) == 4, on MinGW - 8!
        iSize/=2; // it generate same output as SDK tool

	iTable = new uint32_t[aSize];
	// Set up header
	iTable[0] = aExports.size();
	Elf32_Phdr * aROHdr = aElfImage->iCodeSegmentHdr;
	// The export table starts after the header. NB this is a virtual address in the RO
	// segment of the E32Image. It is outside the ELF RO segment.
	Elf32_Addr * aPlace =  ELF_ENTRY_PTR(Elf32_Addr, (intptr_t)aROHdr->p_vaddr, aROHdr->p_filesz) + 1;
	iExportTableAddress = (uintptr_t)aPlace;
	// Tell the E32Image constructor that it must allocate the export table
	// i.e. copy it from iTable.
	iAllocateP = true;
	bool aDelSym;
	ElfExports::Exports::iterator first = aExports.begin();
	for (uint32_t i = 1; i < aSize; i++, first++)
	{
		Elf32_Sym * sym;

		uint32_t ptr;
		if ((*first)->Absent())
		{
			ptr = aElfImage->iEntryPoint;
			sym = new Elf32_Sym;
			memset(sym, 0, sizeof(Elf32_Sym));
			sym->st_value = ptr;
			aDelSym = true;
		}
		else
		{
			ptr = (*first)->iElfSym->st_value;
			sym = (*first)->iElfSym;
			aDelSym = false;
		}
		// set up the pointer
		iTable[i] = ptr;
		ElfLocalRelocation * aRel = new ElfLocalRelocation(iElfImage,
                iExportTableAddress, 0, i, R_ARM_ABS32, nullptr, ESegmentRO, sym, aDelSym);
	    aPlace++; /// TODO (Administrator#1#06/02/17): why that val never used??!!
		aRel->Add();
	}
}

/**
This function returns the number of exports.
@internalComponent
@released
*/
size_t E32ExportTable::GetNumExports()
{
	return iNumExports;
}

/**
This function tells the e32 image allocator if space is required to
be allocated for export table.
@internalComponent
@released
*/
bool E32ExportTable::AllocateP(){
	return iAllocateP;
}

/**
This function returns the e32 export table size.
@internalComponent
@released
*/
size_t E32ExportTable::GetExportTableSize(){
	return iSize;
}

/**
This function returns e32 export table.
@internalComponent
@released
*/
uint32_t * E32ExportTable::GetExportTable() {
	 return iTable;
}

