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
// Implementation of the Class ElfImportRelocation for the elf2e32 tool
// @internalComponent
// @released
//
//

#if !defined(_PL_ELFIMPORTRELOCATION_H_)
#define _PL_ELFIMPORTRELOCATION_H_

#include "pl_elfrelocation.h"

/**
This class represents relocation entries corresponding to the import symbols.
@internalComponent
@released
*/
class ElfImportRelocation : public ElfRelocation
{

public:
	ElfImportRelocation(ElfImage *aElfImage, PLMemAddr32 aAddr,
			PLUINT32 aAddend, PLUINT32 aIndex, PLUCHAR aRelType,
			Elf32_Rel* aRel);
	~ElfImportRelocation();
	void Add();

	VersionInfo *iVerRecord = nullptr;
};




#endif // !defined(_PL_ELFIMPORTRELOCATION_H_)
