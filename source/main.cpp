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
// Implementation of the main function for elf2e32 tool
// @internalComponent
// @released
//
//

//
#include <stdlib.h>

#include "message.h"
#include "filedump.h"
#include "errorhandler.h"
#include "elffilesupplied.h"
#include "parametermanager.h"

static ParameterManager * Instance = nullptr;

/**
This function creates an instance of Elf2E32 and calls Execute() of Elf2E32 to generate
the appropriate target.
@internalComponent
@released

@param aArgc
 The number of command line arguments passed into the program
@param aArgv
 The listing of all the arguments


@return EXIT_SUCCESS if the generation of the target is successful, else EXIT_FAILURE

*/
int main(int argc, char** argv)
{
    int result = EXIT_SUCCESS;

    try
    {
        Instance = ParameterManager::GetInstance(argc, argv);
        Instance->ParameterAnalyser();
		Instance->CheckOptions();

        char * dumpMessageFile = Instance->DumpMessageFile();
	 	if(dumpMessageFile){
			//create message file
            Message::GetInstance()->CreateMessageFile(dumpMessageFile);
            return result;
		}

        if (Instance->E32Input()){
            FileDump *f = new FileDump(Instance);
            result = f->Execute();
            delete f;
			return result;
        }

        ElfFileSupplied *job = new ElfFileSupplied(Instance);
        job->Execute();
       // delete job; // FIXME: app crash if enabled!
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

