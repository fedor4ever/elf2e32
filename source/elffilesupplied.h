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
#include "usecasebase.h"
#include "e32exporttable.h"

class Symbol;
class ElfReader;
class ElfProducer;
class ParameterManager;

typedef std::list<Symbol*> Symbols;

/**
This class is derived from the base class UseCaseBase and handles case for elf file supplied.

@internalComponent
@released
*/
class ElfFileSupplied : public UseCaseBase
{

public:
	explicit ElfFileSupplied(ParameterManager* aParameterManager);
	virtual ~ElfFileSupplied();
	int Execute();

	void ReadElfFile();
	virtual void ProcessExports();
	virtual void BuildAll();
	void ValidateDefExports(Symbols* aDefExports);
	void CreateExports();
	void WriteDefFile();
	void WriteDSOFile();
	void WriteE32();
	virtual bool ImageIsDll();
	virtual bool WarnForNewExports();
	SymbolType SymbolTypeF(char * aName);
	E32ImageHeaderV * AllocateE32ImageHeader();

	virtual void CreateExportTable();
	void CreateExportBitMap();
	virtual size_t GetNumExports();
	virtual size_t GetExportOffset();
	virtual bool AllocExpTable();
	virtual char * GetExportTable();
	virtual size_t GetExportTableSize();

	PLUINT16 GetExportDescSize();
	PLUINT8 GetExportDescType();
	size_t GetExportTableAddress();
	//Method to check whether a symbol name is unwanted
	bool UnWantedSymbol(const char * aSymbol);

protected:
	Symbols iSymbols;

	E32ExportTable iExportTable;
	int iNumAbsentExports;
	PLUINT8 * iExportBitMap;

	E32ImageFile * iE32ImageFile;
	ElfReader *iReader;
	ElfProducer* iElfProducer;

	PLUINT16 iExportDescSize;
	PLUINT8 iExportDescType;
};


#endif

