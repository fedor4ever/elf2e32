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
// Declaration of Class ExportTypeTarget of the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef EXPORT_TYPE_TARGET_H
#define EXPORT_TYPE_TARGET_H

#include "elffilesupplied.h"

/**
class ExportTypeTarget is dervied from ElfSupplied and handles export type target

@internalComponent
@released
*/
class DLLTarget : public ElfFileSupplied
{

public:
	explicit DLLTarget(ParameterManager* aParameterManager);
	~DLLTarget();

	bool ImageIsDll();
	void BuildAll();

protected:
	Symbols *iDefExports;

};


#endif // EXPORT_TYPE_TARGET_H

