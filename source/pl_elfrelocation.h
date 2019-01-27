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
// Implementation of the Class ElfRelocation for the elf2e32 tool
// @internalComponent
// @released
//
//

#if !defined(_PL_ELFRELOCATION_H_)
#define _PL_ELFRELOCATION_H_

#include "elfdefs.h"
#include "pl_common.h"

class DllSymbol;
class ElfImage;

/**
This class is for Elf relocation.

@internalComponent
@released
*/
class ElfRelocation
{

public:
	ElfRelocation(ElfImage *aElfImage, PLMemAddr32 aAddr, PLUINT32 aAddend,
			PLUINT32 aIndex, PLUCHAR aRelType, Elf32_Rel* aRel);
	virtual ~ElfRelocation(){}

	PLMemAddr32 iAddr;
	PLUINT32	iAddend;
	PLUINT32	iSymNdx;
	PLUCHAR		iRelType;
	Elf32_Rel	*iRel = nullptr;
	Elf32_Sym	*iSymbol = nullptr;
	ElfImage 	*iElfImage = nullptr;
	Elf32_Phdr	*iSegment = nullptr;
	ESegmentType iSegmentType = ESegmentUndefined;

	VersionInfo *iVerRecord = nullptr;
};

#endif // !defined(_PL_ELFRELOCATION_H_)
