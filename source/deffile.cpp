// Copyright (c) 2018 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Contributors:
//
// Description:
// Implementation of the Class DefFile for the elf2e32 tool
// @internalComponent
// @released
//
//

//
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "deffile.h"
#include "pl_symbol.h"
#include "errorhandler.h"

using std::fstream;
using std::string;

void WriteDefString(Symbol *sym, std::fstream &fstr);
void TokensChecker(const std::vector<std::string> &tokens);

/**
Function to Read def file and get the internal representation in structure.
@param defFile - DEF File name
*/
Symbols DefFile::GetSymbols(char *defFile)
{
	ReadDefFile(defFile);
	ParseDefFile();
	return iSymbols;
}

/**
Function to read def file line by line to std::vector.
@param defFile - DEF File name
*/
void DefFile::ReadDefFile(char * aDefFile)
{
    iFileName=aDefFile;

    fstream file(aDefFile, fstream::in /*| fstream::binary | fstream::ate*/);

    if(!file.is_open())
    {
        throw Elf2e32Error(FILEOPENERROR, aDefFile);
    }

    string s;
    while(file.good())
    {
        getline(file, s);
        iDefFile.push_back(s);
    }
}

const std::string trim_chars = " \t\n\v\f\r\0";
void LTrim(std::string& str)
{
    str.erase(0, str.find_first_not_of(trim_chars));
}

void RTrim(std::string& str)
{
    str.erase(str.find_last_not_of(trim_chars) + 1);
}

void Trim(std::string& str)
{
    RTrim(str);
    LTrim(str);
}

/**
Function to Parse Def File which has been read in buffer.
@internalComponent
@released
*/
void DefFile::ParseDefFile()
{
	int PreviousOrdinal=0;
	int LineNum = 0;

	for(auto str: iDefFile)
    {
        if(str.find("NONAME") != string::npos)
        {
            Symbol aSymbol("", SymbolTypeCode);
            Trim(str);
            Tokenizer(str, LineNum);
            int ordinalNo = iSymbol->OrdNum();
            if (ordinalNo != PreviousOrdinal+1)
            {
                throw DEFFileError(ORDINALSEQUENCEERROR, (char*)iFileName.c_str(), LineNum, (char*)aSymbol.SymbolName());
            }

            PreviousOrdinal = ordinalNo;
        }
        LineNum++;
    }
}

void TokensChecker(const std::vector<std::string> &tokens)
{
    if(tokens[1] != "@")
        throw Elf2e32Error(ARGUMENTNAMEERROR, tokens[1]);

    for(auto x: tokens)
    {
        if(x.find_first_of(trim_chars) != string::npos)
            throw Elf2e32Error(ARGUMENTNAMEERROR, x);
    }
}

/** @brief Analyze line from .def file
  *
  * It split every line to tokens and initialize Symbol class with them
  * Example string for tokenize:
  *  "BIGNUM_it @ 2717 NONAME R3UNUSED ABSENT; some comment"
  *  "BIGNUM_it @ 2717 NONAME DATA 28; some comment"
  */
void DefFile::Tokenizer(std::string aLine, size_t aIndex)
{
    iSymbol = new Symbol("", SymbolTypeCode);

//    take comments
    std::size_t pos = aLine.find_first_of(';');
    if(pos < string::npos)
    {
        std::string comment = aLine.substr(pos);
        iSymbol->Comment(comment);
        aLine.erase(pos);
    }

//    check optional arguments
    if(aLine.find(" DATA ") < string::npos)
        iSymbol->CodeDataType(SymbolTypeData);
    if(aLine.find(" R3UNUSED") < string::npos)
        iSymbol->R3Unused(true);
    if(aLine.find(" ABSENT") < string::npos)
        iSymbol->SetAbsent(true);

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(aLine);

    while (std::getline(tokenStream, token, ' '))
    {
        tokens.push_back(token);
    }

    if((tokens.size() > 4) && (iSymbol->CodeDataType() == SymbolTypeData))
    {
        for(auto z: tokens[5]) // size of variable in elf
        {
            if(!isdigit(z))
                throw DEFFileError(UNRECOGNIZEDTOKEN, (char* )iFileName.c_str(),
                    aIndex, (char* )tokens[5].c_str());
        }
        uint32_t lenth = atol( tokens[5].c_str() );
        iSymbol->SetSymbolSize(lenth);
    }

    /**< Take SymbolName and maybe AliasName  */
    if(tokens[0].find('=') == string::npos)
        iSymbol->SetSymbolName((char* )tokens[0].c_str());
    else
    {
        /**< Symbol name may have alias like SymbolName=AliasName */
        if(tokens[0].find('=') != tokens[0].rfind('='))
            throw DEFFileError(UNRECOGNIZEDTOKEN, (char* )iFileName.c_str(),
                    aIndex, (char* )tokens[0].c_str()); /**< Not allowed like SomeName=OtherName=AnotherName */

        std::string sName =tokens[0].substr(0, tokens[0].find('='));
        std::string eName =tokens[0].substr(tokens[0].find('='));
        iSymbol->SetSymbolName( (char* )sName.c_str() );
        iSymbol->ExportName((char* )eName.c_str());
    }

    iSymbol->SetOrdinal( atol( tokens[2].c_str() ) );

    iSymbols.push_back(iSymbol);
}


/**
Function to write DEF file from symbol entry List.
@param fileName - Def file name
@param newSymbols - pointer to Symbols which we get as an input for writing in DEF File
@internalComponent
@released
*/
void DefFile::WriteDefFile(char *fileName, Symbols * newSymbols)
{
    std::fstream fs;
    fs.open(fileName, std::fstream::out | std::fstream::trunc);
    if(!fs.is_open())
        throw Elf2e32Error(FILEOPENERROR,fileName);

    bool newSymbol = false;
    fs << "EXPORTS\n";

    for(auto x: *newSymbols)
    {
        if(x->GetSymbolStatus()==New)
        {
            newSymbol = true;
            continue;
        }

        if(x->GetSymbolStatus()==Missing)
            fs << "; MISSING:";
        WriteDefString(x, fs);
    }

//This is for writing new def entry in DEF File
    if(newSymbol)
    {
        fs << "; NEW:\n";
        for(auto x: *newSymbols)
        {
            if(x->GetSymbolStatus() != New)
                continue;

            WriteDefString(x, fs);
        }
    }
    fs << "\n";
    fs.close();
}

void WriteDefString(Symbol *sym, std::fstream &fstr)
{
    fstr << "\t";
    if((sym->ExportName()) && strcmp(sym->SymbolName(),sym->ExportName())!=0)
        fstr << sym->ExportName();

    fstr << sym->SymbolName();
    fstr << " @ ";
    fstr << sym->OrdNum();
    fstr << " NONAME";

    if(sym->CodeDataType()==SymbolTypeData)
    {
        fstr << " DATA ";
        fstr << sym->SymbolSize();
    }

    if(sym->R3unused())
        fstr << " R3UNUSED";
    if(sym->Absent())
        fstr << " ABSENT";

    if(!sym->Comment().empty())
    {
        fstr << " ; ";
        fstr << sym->Comment();
    }

    fstr << "\n";
}
