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
// Implementation of the Class ElfRelocations for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "pl_elfrelocations.h"
#include "pl_elflocalrelocation.h"

/**
Destructor for class ElfRelocations to release allocated memory
@internalComponent
@released
*/
ElfRelocations::~ElfRelocations()
{
	if(iCodeRelocations.size())
	{
		for(auto x: iCodeRelocations) delete x;

		iCodeRelocations.clear();
	}
	if(iDataRelocations.size())
	{
		for(auto x: iCodeRelocations) delete x;

		iDataRelocations.clear();
	}
}


/**
Function for adding Elf local Relocations.
@internalComponent
@released
@return 0 if therelocation is valid.
*/
PLUINT32 ElfRelocations::Add(ElfLocalRelocation* aReloc){
	if(!aReloc) return 1;

	switch (aReloc->iSegmentType)
	{
	case ESegmentRO:
		iCodeSortedP = false;
		iCodeRelocations.push_back(aReloc);
		break;
	case ESegmentRW:
		iDataSortedP = false;
		iDataRelocations.push_back(aReloc);
		break;
	default:
		;
	}

	return 0;
}

/**
Function for getting code relocations. The reloc entries are
sorted on the address they refer to.
@internalComponent
@released
@return list of code relocation
*/
ElfRelocations::RelocationList & ElfRelocations::GetCodeRelocations()
{
	if (!iCodeSortedP)
	{
		iCodeRelocations.sort(Cmp());
		iCodeSortedP = true;
	}
  return iCodeRelocations;
}

/**
Function for getting data relocations. The reloc entries are
sorted on the address they refer to.
@internalComponent
@released
@return list of code relocation
*/
ElfRelocations::RelocationList & ElfRelocations::GetDataRelocations()
{
	if (!iDataSortedP)
	{
		iDataRelocations.sort(Cmp());
		iDataSortedP = true;
	}
  return iDataRelocations;
}

/**
Overloaded operator for comparing location the relocations refer to.
@internalComponent
@released
@return comparison of both relocation
*/
bool ElfRelocations::Cmp::operator()(ElfRelocation * x, ElfRelocation * y)
{
	return x->iAddr < y->iAddr;
}


