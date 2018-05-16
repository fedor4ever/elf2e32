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
// Implementation of the Class Elf2E32 for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "parametermanager.h"
#include "elf2e32.h"
#include "errorhandler.h"
#include "librarytarget.h"
#include "exetarget.h"
#include "polydll_fb_target.h"
#include "polydll_rebuild_target.h"
#include "filedump.h"

#include <iostream>

using std::cout;
using std::endl;

/** Static Pointer to the ParameterManager */
static ParameterManager * iInstance = nullptr;

/**
Constructor for Elf2E32

@internalComponent
@released

@param aArgc
 The number of command line arguments passed into the program
@param aArgv
 The listing of all the arguments
*/
Elf2E32::Elf2E32(int aArgc, char **aArgv)
{
	iInstance = ParameterManager::GetInstance(aArgc, aArgv);
}


/**
This function is to select the appropriate use case based on the input values.
1. If the input is only DEF file, then the usecase is the Create Library Target.
   For Library Creation, alongwith the DEF file input, the DSO file option and
   the link as option SHOULD be passed. Otherwise, appropriate error message will
   be generated.
@internalComponent
@released

@return A pointer to the newly created UseCaseBase object

*/
UseCaseBase * Elf2E32::SelectUseCase()
{
	char * deffilein = iInstance->DefInput();
	char * elfin = iInstance->ElfInput();
	char * e32in = iInstance->E32Input();

    if (iInstance->DumpMessageFile())
        return nullptr;
    if (e32in){
        return new FileDump(iInstance);
    }

	ETargetType iTargetType = iInstance->TargetTypeName();
	if (iTargetType == EInvalidTargetType || iTargetType == ETargetTypeNotSet)
	{
	    Message::GetInstance()->ReportMessage(WARNING, NOREQUIREDOPTIONERROR,"--targettype");
		if (elfin)
		{
			if (deffilein)
				return new ExportTypeRebuildTarget(iInstance);
			else
				return new ElfFileSupplied(iInstance);
		}
		else if (deffilein)
		{
			iTargetType = ELib;
			if(iInstance->DefOutput()) iTargetType = EDll;
		}
		else
			throw Elf2e32Error(INVALIDINVOCATIONERROR); //REVISIT
	}

	switch (iTargetType)
	{
	case EDll:
		if (deffilein)
			return new ExportTypeRebuildTarget(iInstance);
		else if (!deffilein)
			return new DLLTarget(iInstance);
	case ELib:
        return new LibraryTarget(iInstance);
	case EStdExe: // fallthru
	case EExe:
		return new ExeTarget(iInstance);
	case EPolyDll:
		if (!deffilein)
			return new POLYDLLFBTarget(iInstance);
		else if (deffilein)
			return new POLYDLLRebuildTarget(iInstance);
	case EExexp:
		if (!deffilein)
			return new DLLTarget(iInstance);
		else if (deffilein)
			return new ExportTypeRebuildTarget(iInstance);
	default:
		throw Elf2e32Error(INVALIDINVOCATIONERROR);
	}

	return nullptr;
}

/**
This function:
 1. Calls the ParameterAnalyser() which parses the command line options and extracts the inputs.
 2. Calls the CheckOptions() to find and fix wrong input options
 3. Calls the SelectUseCase() to select the appropriate use case based on the input values.
 4. Calls the Execute() of the selected use case.
@internalComponent
@released

@return EXIT_SUCCESS if the generation of the target is successful, else EXIT_FAILURE

*/
int Elf2E32::Execute()
{
	int result = EXIT_SUCCESS;
	UseCaseBase * usecase = nullptr;

	try
	{
		iInstance->ParameterAnalyser();
		iInstance->CheckOptions();

		char * dumpMessageFile = iInstance->DumpMessageFile();
	 	if(dumpMessageFile)
		{
			//create message file
            Message::GetInstance()->CreateMessageFile(dumpMessageFile);
            return result;
		}

		usecase = SelectUseCase();
		if (usecase)
		{
			result = usecase->Execute();
		}else
		{
			result = EXIT_FAILURE;
		}
	}
	catch(ErrorHandler& error)
	{
		result = EXIT_FAILURE;
		error.Report();
	}
	catch(...) // If there are any other unhandled exception,they are handled here.
	{
		result = EXIT_FAILURE;
		Message::GetInstance()->ReportMessage(ERROR, POSTLINKERERROR);
	}
	delete usecase;
	return result;
}
