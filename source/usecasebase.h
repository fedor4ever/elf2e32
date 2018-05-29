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
// Implementation of the Header file for Class UseCaseBase of the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef USECASEBASE_H
#define USECASEBASE_H

#include "pl_symbol.h"
#include "e32imagefile.h"
#include "parametermanager.h"

/**
This class is the base class for the use cases. The appropriate usecases like LibraryTarget,
DLLTarget, EXETarget are derived from this class.

Other classes like ParameterManager are dependant on this class.

@internalComponent
@released
*/
class UseCaseBase {

    public:
        explicit UseCaseBase(ParameterManager* aParameterManager);
        virtual ~UseCaseBase();
        char * DefInput();
        char * DSOOutput();
        char * LinkAsDLLName();
        char * FileName(char *aFileName);

//const char * DefOutput();
        char *DefOutput();

        char *LogFile();
        bool LogFileOption();

        const std::string& InputElfFileName();

        const char * OutputDSOFileName();

        const char * OutputE32FileName();

        bool AllowDllData();

        uint32_t HeapCommittedSize();

        uint32_t HeapReservedSize();

        uint32_t StackCommittedSize();

        uint32_t GetUid1();

        uint32_t GetUid2();

        uint32_t GetUid3();

        uint32_t GetSecureId();
        bool GetSecureIdOption();

        uint32_t GetVendorId();

        uint32_t GetVersion();

        bool GetCallEntryPoints();

        SCapabilitySet GetCapability();
        bool Unfrozen();

        TProcessPriority GetPriority();

        bool GetFixedAddress();

        bool GetCompress();
        uint32_t GetCompressionMethod();

        uint32_t GetFPU();

        ParameterManager::LibSearchPaths &  GetLibSearchPaths();

        bool GetIgnoreNonCallable();

        bool IsCodePaged();
        bool IsCodeUnpaged();
        bool IsCodeDefaultPaged();

        bool IsDataPaged();
        bool IsDataUnpaged();
        bool IsDataDefaultPaged();

        bool ExcludeUnwantedExports();
        bool IsCustomDllTarget();
        bool GetNamedSymLookup();

        bool IsDebuggable();

        bool IsSmpSafe();

        SymbolType SymbolTypeF(char * aName);
        /**
        This function creates the appropriate target.

        @internalComponent
        @released

        @return EXIT_SUCCESS if the generation of the target is successful, else EXIT_FAILURE
        */
        virtual int Execute() = 0;

        ParameterManager *iParameterManager;
};


#endif // USECASEBASE_H
