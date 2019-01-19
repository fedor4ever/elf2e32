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
#include "e32producer.h"
#include "errorhandler.h"
#include "elffilesupplied.h"
#include "parametermanager.h"

#define E32COMMON_H_INCLUDED
#include "e32info.h"

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
    auto hdr = new E32ImageHeader();

    try
    {
        Instance = ParameterManager::GetInstance(argc, argv, hdr);
        Instance->ParameterAnalyser();
		Instance->CheckOptions();

        char * dumpMessageFile = Instance->DumpMessageFile();
	 	if(dumpMessageFile){
			//create message file
            Message::GetInstance()->CreateMessageFile(dumpMessageFile);
            return result;
		}


        if(Instance->E32Input() && Instance->E32ImageOutput()){
            auto f = new E32Producer(Instance);
            f->Run();
            delete f;
			return result;
        }
        if (Instance->FileDumpOptions()){
            auto f = new E32Info(Instance);
            f->Run();
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
	delete hdr;
	return result;
}

