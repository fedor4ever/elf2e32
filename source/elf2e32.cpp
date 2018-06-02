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

#include <iostream>

#include "parametermanager.h"
#include "elf2e32.h"
#include "errorhandler.h"
#include "elffilesupplied.h"
#include "filedump.h"

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
This function:
 1. Calls the ParameterAnalyser() which parses the command line options and extracts the inputs.
 2. Calls the CheckOptions() to find and fix wrong input options
 3. Calls the Execute() of the selected use case.
@internalComponent
@released

@return EXIT_SUCCESS if the generation of the target is successful, else EXIT_FAILURE

*/
int Elf2E32::Execute()
{
	int result = EXIT_SUCCESS;

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

        if (iInstance->E32Input()){
            FileDump *f = new FileDump(iInstance);
            result = f->Execute();
            delete f;
			return result;
        }

        ElfFileSupplied *job = new ElfFileSupplied(iInstance);
        result = job->Execute();
        delete job;
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
	return result;
}
