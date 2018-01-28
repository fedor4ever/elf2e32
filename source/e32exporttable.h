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
// Class for E32 Export Table implementation of the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef __E32EXPORTTABLE__
#define __E32EXPORTTABLE__

#include <elfdefs.h>

#include "pl_elfexports.h"

class ElfExecutable;

/**
class for E32 Export Table
@internalComponent
@released
*/
class E32ExportTable {
 public:
  ~E32ExportTable();
  void CreateExportTable(ElfExecutable * aElfExecutable, ElfExports::ExportList & aExportList);
  size_t GetNumExports();
  bool AllocateP();
  size_t GetExportTableSize();
  uint32_t * GetExportTable();

 public:
  ElfExecutable * iElfExecutable=nullptr;
  size_t iSize=0;
  uint32_t * iTable=nullptr;
  // NB. This a virtual address (within the RO segment).
  uintptr_t iExportTableAddress=0;
  // True if the postlinker must allocate the export table in the E32Image.
  // This should only be false for custom built ELF executables.
  bool iAllocateP=true;
  size_t iNumExports=0;
};


#endif
