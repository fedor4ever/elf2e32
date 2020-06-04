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
// Implementation of the Class ElfImage for the elf2e32 tool
// @internalComponent
// @released
//
//

#include <string.h>
#include <fstream>

#include "pl_elfimage.h"
#include "errorhandler.h"

using std::min;
using std::list;
using std::fstream;


void ElfImage::Read(){

    fstream fs(iElfInput.c_str(), fstream::binary | fstream::in);
    if(!fs)
		throw Elf2e32Error(FILEOPENERROR, iElfInput);

    fs.seekg(0, fs.end);
    size_t elfSize = fs.tellg();
    fs.seekg(0, fs.beg);

    iMemBlock = new char[elfSize]();
    fs.read(iMemBlock, elfSize);
    fs.close();
}


/**
Funtion for getting elf symbol list
@param aList - list of symbols found in elf files
@return - 0 for no exports in elf files, otherwise number of symbols found
@internalComponent
@released
*/
Symbols ElfImage::GetElfSymbols(){

	if(!iExports)
		return Symbols();

	//Get the exported symbols
	vector<Symbol*> tmp = iExports->GetExports(true);
	return Symbols(tmp.begin(), tmp.end());

}


/**
Funtion for processing elf file
@internalComponent
@released
*/
PLUINT32 ElfImage::ProcessElfFile()
{
    Read();
	Elf32_Ehdr *aElfHdr = ELF_ENTRY_PTR(Elf32_Ehdr, iMemBlock, 0);
    ProcessElfFile(aElfHdr);

    /* The following is a workaround for the ARM linker problem.
     * Linker Problem: ARM linker generates Long ARM to Thumb veneers for which
     * relocation entries are not generated.
     * The linker problem is resolved in ARM Linker version RVCT 2.2 Build 616.
     * Hence the workaround is applicable only for executables generated
     * by ARM linker 2.2 and if build number is below 616.
     */
    char const aARMCompiler[] = "ARM Linker, RVCT";
    int length = strlen(aARMCompiler);
    char * aCommentSection = ElfImage::FindCommentSection();
    /* The .comment section in an elf file contains the compiler version information and
     * it is used to apply the fore mentioned workaround.
     * Some build tool chains generating elf file output without the .comment section,
     * just to save the disk space. In this case the variable aCommentSection gets the NULL value.
     * Solution: This workaround is only applicable for RVCT compiler. So if the .comment section
     * is not available in the elf file, then this workaround is no need to be applied.
     */
    if(aCommentSection && (!strncmp(aCommentSection, aARMCompiler, length)) )
    {
        int WorkAroundBuildNo = 616;
        int BuildNo = 0;
        char* RVCTVersion = aCommentSection+length;

        /* RVCTVersion contains the following string
         * "<MajorVersion>.<MinorVersion> [Build <BuildNumber>]"
         * Example: "2.2 [Build 616]"
         */
        string Version(RVCTVersion);
        size_t pos = Version.find_last_of(' ');
        size_t size = Version.size();
        if (pos < size)
        {
            size_t index = pos + 1;
            if (index < size)
            {
                BuildNo = atoi(strtok(RVCTVersion+index, "]"));
            }
        }

        /* Workaround is applicable only when the version is 2.2 and if the
         * build number is below 616.
         */
        size_t minorVersionPos = Version.find_first_of('.');
        char RVCTMinorVersion='0';
        if (minorVersionPos < size)
        {
            size_t index = minorVersionPos + 1;
            if (index < size)
            {
                RVCTMinorVersion = *(RVCTVersion + index);
            }
        }

        if ((*RVCTVersion == '2') && (RVCTMinorVersion == '2') &&
            (BuildNo < WorkAroundBuildNo))
        {
            /* The static symbol table should be processed to identify the veneer symbols.
             * Relocation entries should be generated for these symbols if the linker
             * is not generating the same.
             */
            ElfImage::FindStaticSymbolTable();
            ElfImage::ProcessVeneers();
        }
    }
	return 0;
}
