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
#include "stdexe_target.h"
#include "filedump.h"

#include <iostream>

using std::cout;
using std::endl;

ParameterManager * Elf2E32::iInstance=nullptr;

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
	iPrmManager = GetInstance(aArgc, aArgv);
}


Elf2E32::~Elf2E32()
{
	delete iInstance;
}

/**
This function creates a single instance of the ParameterManager which is derived from
ParameterListInterface which is the abstract base class.

@internalComponent
@released

@param aArgc
 The number of command line arguments passed into the program
@param aArgv
 The listing of all the arguments

@return A pointer to the newly created ParameterListInterface object which is the
 abstract interface
*/
ParameterManager * Elf2E32::GetInstance(int aArgc, char ** aArgv)
{
	if (!iInstance)
		iInstance = new ParameterManager(aArgc, aArgv);

	return iInstance;
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
	bool definputoption = iPrmManager->DefFileInOption();
	bool elfinputoption = iPrmManager->ElfFileInOption();
	char * deffilein = iPrmManager->DefInput();
	char * elfin = iPrmManager->ElfInput();
	bool filedumpoption = iPrmManager->FileDumpOption();
	int dumpOptions = iPrmManager->DumpOptions();
	char *filedumpsuboptions = iPrmManager->FileDumpSubOptions();

	bool e32inputoption = iPrmManager->E32ImageInOption();
	char * e32in = iPrmManager->E32Input();

	bool dumpMessageFileOption = iPrmManager->DumpMessageFileOption();

	if (definputoption && !deffilein)
		throw ParameterParserError(NOARGUMENTERROR, "--definput");

	if (elfinputoption && !elfin)
		throw ParameterParserError(NOARGUMENTERROR, "--elfinput");

	if(filedumpoption && !dumpOptions)
	{
		//throw for wrong options
		throw InvalidArgumentError(INVALIDARGUMENTERROR,filedumpsuboptions,"--dump");
	}

	if(e32inputoption && !e32in)
	{
        throw ParameterParserError(NOARGUMENTERROR, "--e32input");
	}

	iTargetType = iPrmManager->TargetTypeName();

//	if (iTargetType == ETargetTypeNotSet) // Will get this warning in build
//		cout << "Warning: Target Type is not specified explicitly" << endl;

	if (iTargetType == EInvalidTargetType || iTargetType == ETargetTypeNotSet)
	{
		if (elfin)
		{
			if (deffilein)
				return iUseCase = new ExportTypeRebuildTarget(iPrmManager);
			else
				return iUseCase = new ElfFileSupplied(iPrmManager);
		}
		else if (filedumpoption || e32in)
		{
			return iUseCase = new FileDump(iPrmManager);
		}
		else if (deffilein)
		{
			iTargetType = ELib;
		}
		else if (dumpMessageFileOption)
			return nullptr;
		else
			throw InvalidInvocationError(INVALIDINVOCATIONERROR); //REVISIT
	}

	switch (iTargetType)
	{
	case EDll:
		if (!deffilein)
			iUseCase = new DLLTarget(iPrmManager);
		else if (deffilein)
			iUseCase = new ExportTypeRebuildTarget(iPrmManager);
		return iUseCase;
	case ELib:
        return iUseCase = new LibraryTarget(iPrmManager);
	case EExe:
		return iUseCase = new ExeTarget(iPrmManager);
	case EPolyDll:
		if (!deffilein)
			iUseCase = new POLYDLLFBTarget(iPrmManager);
		else if (deffilein)
			iUseCase = new POLYDLLRebuildTarget(iPrmManager);
		return iUseCase;
	case EExexp:
		if (!deffilein)
			iUseCase = new DLLTarget(iPrmManager);
		else if (deffilein)
			iUseCase = new ExportTypeRebuildTarget(iPrmManager);
		return iUseCase;
	case EStdExe:
		return iUseCase = new StdExeTarget(iPrmManager);
	default:
		throw InvalidInvocationError(INVALIDINVOCATIONERROR);
	}

	return (iUseCase=0x0);
}

/**
This function:
 1. Calls the ParameterAnalyser() which parses the command line options and extracts the inputs.
 2. Calls the CheckOptions() to find and fix wrong input options
 2. Calls the SelectUseCase() to select the appropriate use case based on the input values.
 3. Calls the Execute() of the selected use case.
@internalComponent
@released

@return EXIT_SUCCESS if the generation of the target is successful, else EXIT_FAILURE

*/
int Elf2E32::Execute()
{
	int result = EXIT_SUCCESS;
	UseCaseBase * usecase=nullptr;

	try
	{
		iPrmManager->ParameterAnalyser();
		iPrmManager->CheckOptions();

		bool dumpMessageFileOption = iPrmManager->DumpMessageFileOption();
		char * dumpMessageFile = iPrmManager->DumpMessageFile();

	 	if(dumpMessageFileOption)
		{
			if (dumpMessageFile)
			{
				//create message file
				MessageHandler::GetInstance()->CreateMessageFile(dumpMessageFile);
				//return result;
			}
			else
			//dumpmessage file name is not provided as input
			throw ParameterParserError(NOARGUMENTERROR, "--dumpmessagefile");
		}

		usecase = SelectUseCase();
		if (usecase)
		{
			result = usecase->Execute();
		}
		else if (dumpMessageFileOption)
		{
			return result;
		}
		else
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
		MessageHandler::GetInstance()->ReportMessage(ERROR, POSTLINKERERROR);
	}
	delete usecase;
	return result;
}
