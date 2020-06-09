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
// ElfFileSupplied class for elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef __ELFFILESUPPLIED_H_
#define __ELFFILESUPPLIED_H_

#include <list>
#include "pl_sym_type.h"
#include "e32exporttable.h"

class Symbol;
class ElfProducer;
class E32ImageFile;
class E32ImageHeaderV;
class ParameterManager;

typedef std::list<Symbol*> Symbols;

/**
This class responsible for target generation.

@internalComponent
@released
*/
class ElfFileSupplied
{

public:
	explicit ElfFileSupplied(ParameterManager* parameterManager);
	~ElfFileSupplied();
	void Execute();

	ElfFileSupplied& operator=(const ElfFileSupplied &) = delete;
	ElfFileSupplied(const ElfFileSupplied &) = delete;

	void ReadElfFile();
	void ProcessExports();
	void BuildAll();
	void ValidateDefExports(Symbols& defExports);
	void CreateExports();
	void WriteDefFile();
	void WriteDSOFile();
	void WriteE32();
	bool ImageIsDll();
	bool WarnForNewExports();
	SymbolType SymbolTypeF(char * name);
	E32ImageHeaderV * AllocateE32ImageHeader();

	void CreateExportBitMap();
	size_t GetNumExports();
	PLUINT32* GetExportTable();

	PLUINT16 GetExportDescSize();
	PLUINT8 GetExportDescType();

private:
	Symbols iSymbols;

	ParameterManager * iManager = nullptr;

	E32ExportTable iExportTable;
	int iNumAbsentExports = -1;
	PLUINT8 * iExportBitMap = nullptr;

	E32ImageFile * iE32ImageFile = nullptr;
	ElfImage * iReader = nullptr;
	ElfProducer * iElfProducer = nullptr;

	PLUINT16 iExportDescSize = 0;
	PLUINT8 iExportDescType = 0;
};


#endif

