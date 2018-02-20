// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017 Strizhniou Fiodar.
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
// Implementation of the Class DSOHandler for the elf2e32 tool
// @internalComponent
// @released
//
//


#include "pl_dso_handler.h"
#include "pl_elfreader.h"
#include "pl_elfproducer.h"

/**
Constructor for class DSOHandler
@param aParameterListInterface - instance of ParameterListInterface
@internalComponent
@released
*/
DSOHandler::DSOHandler(string aElfInput)
{
	iElfProducer = new ElfProducer(aElfInput);
	iElfReader = new ElfReader(aElfInput);
}


/**
Destructor for class DSOHandler to release allocated memory
@internalComponent
@released
*/
DSOHandler::~DSOHandler()
{
	DELETE_PTR(iElfProducer);
	DELETE_PTR(iElfReader);
}


/**
Reads the ELF executable file through its ElfReader member
@param aElfFile The ELF executable file name
@internalComponent
@released
@return Error status
*/
PLUINT32 DSOHandler::ReadElfFile(){
	return iElfReader->Read();
}

/**
Processes the ELF executable file through its ElfReader member
@internalComponent
@released
@return Error status
*/
void DSOHandler::ProcessElfFile(){
	iElfReader->ProcessElfFile();
}

/**
Writes the proxy DSO file through its ElfProducer member
@internalComponent
@released
@return Error Status
@param aDsoFullName The full path and the proxy DSO library file name
@param aDSOName The proxy DSO library file name
@param aDllName The DLL name that defines the exported symbols.
@param aSymbolList The list of exported Symbols that are to be included within the proxy DSO library.
*/
PLUINT32 DSOHandler::WriteElfFile(char* aDsoFullName, char* aDSOName, char* aDllName, SymbolList& aSymbolList){

	iElfProducer->SetSymbolList( aSymbolList );
	iElfProducer->WriteElfFile(aDsoFullName, aDSOName, aDllName);

	return 0;
}


/**
This operation fetches the list of symbols that are exported from the ELF
file. This list is used by UseCaseHandler to finalise the export symbols and
compare them with those found from the DEF file.
@internalComponent
@released
@return Error status
@param aList A reference to the list is passed so as to fetch all the exported Symbols from ELF executable.
*/
int DSOHandler::GetElfExportSymbolList(SymbolList& aList){
	return (iElfReader->GetElfSymbols(aList));
}


/**

@internalComponent
@released
@param
*/
void DSOHandler::GetImageDetails(/*E32ImageInterface aImageInterface*/){

}

/**
Function for returning instance of elf consumer
@internalComponent
@released
@return return the elf consumer instance
*/
ElfImage * DSOHandler::ElfImageP(){
	return iElfReader;
}


