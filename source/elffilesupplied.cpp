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
//

#include <algorithm>
#include <iostream>
#include <cstring>

#include "deffile.h"
#include "pl_symbol.h"
#include "pl_elfimage.h"
#include "errorhandler.h"
#include "e32imagefile.h"
#include "pl_elfexports.h"
#include "pl_elfproducer.h"
#include "elffilesupplied.h"
#include "staticlibsymbols.h"
#include "parametermanager.h"

using std::cout;
bool UnWantedSymbol(const char * aSymbol);
Symbols GetExports(ParameterManager *param);

/**
Constructor for class ElfFileSupplied
@param aManager - Instance of class ParameterManager
@internalComponent
@released
*/
ElfFileSupplied::ElfFileSupplied(ParameterManager* aManager) :
    iManager(aManager)
{
	iElfProducer = new ElfProducer(aManager->ElfInput());
	iReader = new ElfImage(aManager->ElfInput());
}

/**
Destructor for class ElfFileSupplied to release allocated memory
@internalComponent
@released
*/
ElfFileSupplied::~ElfFileSupplied()
{
	iSymbols.clear();
	delete iElfProducer;
	delete iReader;
	delete [] iExportBitMap;
}

/**
Execute Function for the Elf File Supplied use case
@return 0 on success
@internalComponent
@released
*/
void ElfFileSupplied::Execute()
{
    ReadElfFile();
    ProcessExports();
	BuildAll();
}

/**
Function to read ELF File
@internalComponent
@released
*/
void ElfFileSupplied::ReadElfFile()
{
    if(iManager->ElfInput().empty())
        return;
	iReader->ProcessElfFile();
}

/**
Function to process exports
@internalComponent
@released
*/
void ElfFileSupplied::ProcessExports()
{
    Symbols def = GetExports(iManager);
    try
    {
        ValidateDefExports(def);    }
    catch(SymbolMissingFromElfError& e)
	{
		/* Only DEF file would be generated if symbols found in
		 * DEF file are missing from the ELF file.
		 */
		WriteDefFile();
		throw;
	}
	CreateExports();
}

/**
Function to write DEF File
@internalComponent
@released
*/
void ElfFileSupplied::WriteDefFile()
{
	char * aDEFFileName = iManager->DefOutput();
	if(!aDEFFileName) return;

	DefFile deffile;
	deffile.WriteDefFile(aDEFFileName, iSymbols);
}

/**
Function to create exports
@internalComponent
@released
*/
void ElfFileSupplied::CreateExports()
{
	if (iReader->iExports || iManager->SymNamedLookup())
	{
		iExportTable.CreateExportTable(iReader);
		CreateExportBitMap();
	}
}

/**
Function to validate exports
@param aDefExports - List of export symbols from Def file and/or sysdef.
@internalComponent
@released
*/
void ElfFileSupplied::ValidateDefExports(Symbols &aDefExports)
{
	/**
	 * Symbols from DEF file (DEF_Symbols) => Valid_DEF + Absent
	 * Symbols from ELF file (ELF_Symbols) => Existing  + NEW
	 * 1. if the set {Valid_DEF - ELF_Symbols} is non-empty then, symbols are missing
	 *			from Elf and it is an error if the mode is UNFROZEN
	 * 2. if the intersection set {Absent,ELF_Symbols} is non-empty, absent symbols
	 *			are exported from Elf..warn for this
	 * 3. if the set {ELF_Symbols - Valid_DEF} is non-empty, these are the NEW symbols
	 * 4. if there are symbols marked absent in DEF file but exported in ELF,
	 *		add them into the new list retaining the ordinal number as of the
	 *		absent symbol(using PtrELFExportNameCompareUpdateOrdinal).
	 **/

    if (aDefExports.empty())
		return;

    if(iManager->ElfInput().empty())
    {
        iSymbols = aDefExports;
        return;
    }

	PLUINT32 aMaxOrdinal = 0;
	int len = strlen("_ZTI");

	typedef Symbols::iterator Iterator;


	//Symbols aDefExports, defValidExports, defAbsentExports, elfExports;
	Symbols defValidExports, defAbsentExports, elfExports;

    for(auto x: aDefExports)
    {
        if( x->Absent() ){
            defAbsentExports.push_back(x);
        }
        else {
            defValidExports.push_back(x);
        }

        if( aMaxOrdinal < x->OrdNum() ){
            aMaxOrdinal = x->OrdNum();
        }
    }

	iSymbols = defValidExports;

	if (iReader->iExports)
		elfExports = iReader->GetElfSymbols();

	// REVISIT - return back if elfexports and defexports is NULL

	defValidExports.sort(ElfExports::PtrELFExportNameCompare());
	elfExports.sort(ElfExports::PtrELFExportNameCompare());

	defAbsentExports.sort(ElfExports::PtrELFExportNameCompare());

	//Check for Case 1... {Valid_DEF - ELF_Symbols}
	{
		Symbols aResult(defValidExports.size());
		Iterator aResultPos = aResult.begin();

		Iterator aMissingListEnd = set_difference(defValidExports.begin(), defValidExports.end(),
                elfExports.begin(), elfExports.end(), aResultPos,
                ElfExports::PtrELFExportNameCompareUpdateAttributes());

		std::list<string> aMissingSymNameList;
		while (aResultPos != aMissingListEnd) {
			// {Valid_DEF - ELF_Symbols} is non empty
			(*aResultPos)->SetSymbolStatus(Missing); // Set the symbol Status as Missing
			aMissingSymNameList.push_back((*aResultPos)->SymbolName());
			++aResultPos;
			//throw error
		}
		if( !aMissingSymNameList.empty() ) {
			if (!iManager->Unfrozen())
				throw SymbolMissingFromElfError(SYMBOLMISSINGFROMELFERROR, aMissingSymNameList, iManager->ElfInput().c_str());
			else
				cout << "Elf2e32: Warning: " << aMissingSymNameList.size() << " Frozen Export(s) missing from the ELF file" << "\n";
		}
	}

	//Check for Case 2... intersection set {Absent,ELF_Symbols}
	{
		if(!defAbsentExports.empty())
		{
			Symbols aResult(defAbsentExports.size());
			Iterator aResultPos = aResult.begin();

			Iterator aAbsentListEnd = set_intersection(defAbsentExports.begin(), defAbsentExports.end(), \
				elfExports.begin(), elfExports.end(), aResultPos, ElfExports::PtrELFExportNameCompareUpdateAttributes());

			while( aResultPos != aAbsentListEnd )
			{
				// intersection set {Absent,ELF_Symbols} is non-empty

				iSymbols.insert(iSymbols.end(), *aResultPos);
				cout << "Elf2e32: Warning: Symbol " << (*aResultPos)->SymbolName() << " absent in the DEF file, but present in the ELF file" << "\n";
				++aResultPos;
			}
		}
	}

	//Do 3
	{
		Symbols aResult(elfExports.size());
		Iterator aResultPos = aResult.begin();

		Iterator aNewListEnd = set_difference(elfExports.begin(), elfExports.end(), \
			defValidExports.begin(), defValidExports.end(), aResultPos, ElfExports::PtrELFExportNameCompare());

		bool aIgnoreNonCallable = iManager->IgnoreNonCallable();
		bool aIsCustomDll = iManager->IsCustomDllTarget();
		bool aExcludeUnwantedExports = iManager->ExcludeUnwantedExports();

		while( aResultPos != aNewListEnd )
		{

			if( !(*aResultPos)->Absent() )
			{
				/* For a custom dll and for option "--excludeunwantedexports", the new exports should be filtered,
				 * so that only the exports from the frozen DEF file are considered.
				 */
				if ((aIsCustomDll || aExcludeUnwantedExports) && UnWantedSymbol((*aResultPos)->SymbolName()))
				{
					iReader->iExports->ExportsFilteredP(true);
					iReader->iExports->iFilteredExports.push_back((Symbol *)(*aResultPos));
					++aResultPos;
					continue;
				}
				if (aIgnoreNonCallable)
				{
					// Ignore the non callable exports
					if ((!strncmp("_ZTI", (*aResultPos)->SymbolName(), len)) ||
					    (!strncmp("_ZTV", (*aResultPos)->SymbolName(), len)))
					{
						iReader->iExports->ExportsFilteredP(true);
						iReader->iExports->iFilteredExports.push_back((Symbol *)(*aResultPos));
						++aResultPos;
						continue;
					}
				}
				(*aResultPos)->SetOrdinal( ++aMaxOrdinal );
				(*aResultPos)->SetSymbolStatus(New); // Set the symbol Status as NEW
				iSymbols.push_back(*aResultPos);
				if(WarnForNewExports())
					cout << "Elf2e32: Warning: New Symbol " << (*aResultPos)->SymbolName() << " found, export(s) not yet Frozen" << "\n";
			}
			++aResultPos;
		}
	}

	//Do 4
	{
		if(!defAbsentExports.empty())
		{
			Symbols aResult(defAbsentExports.size());
			Iterator aResultPos = aResult.begin();

			Iterator aAbsentListEnd = set_difference(defAbsentExports.begin(), defAbsentExports.end(), \
				elfExports.begin(), elfExports.end(), aResultPos, ElfExports::PtrELFExportNameCompareUpdateAttributes());

			while( aResultPos != aAbsentListEnd  ) {
				//aElfExports.insert(aElfExports.end(), *aResultPos);
				Symbol *sym = new Symbol( *(*aResultPos), SymbolTypeCode, true);
				iReader->iExports->Add(iReader->iSOName, sym);
				//aElfExports.insert(aElfExports.end(), (Symbol*)aSymbol);
				iSymbols.push_back(sym);
				++aResultPos;
			}
			elfExports.sort(ElfExports::PtrELFExportOrdinalCompare());
			iSymbols.sort(ElfExports::PtrELFExportOrdinalCompare());
		}
	}

	if(iReader->iExports && iReader->iExports->ExportsFilteredP() ) {
		iReader->iExports->FilterExports();
	}

	elfExports.clear();
	defAbsentExports.clear();
	defValidExports.clear();
}

/**
Function to generate output which is E32 Image file, DEF File and DSO file, if
exports are available.
@internalComponent
@released
*/
void ElfFileSupplied::BuildAll()
{
    WriteDefFile();
    WriteDSOFile();
	WriteE32();
}

/**
Function to write DSO file.
@internalComponent
@released
*/
void ElfFileSupplied::WriteDSOFile()
{
	char * aDSOName = iManager->DSOOutput();
	if(!aDSOName)
	    return;

	char * aDSOFileName = iManager->FileName(aDSOName);
	char * aLinkAs = iManager->LinkAsDLLName();

	/** This member is responsible for generating the proxy DSO file. */

	iElfProducer->SetSymbolList(iSymbols);
	iElfProducer->WriteElfFile(aDSOName, aDSOFileName, aLinkAs);
}

/**
Function to write E32 Image file.
@internalComponent
@released
*/
void ElfFileSupplied::WriteE32()
{
	const char * e32 = iManager->E32ImageOutput();

    if(!e32)
	    return;

    if(iManager->ElfInput().empty())
        throw Elf2e32Error(NOREQUIREDOPTIONERROR, "--elfinput");

	iE32ImageFile = new E32ImageFile(iReader, this, iManager, &iExportTable);

	try
	{
		iE32ImageFile->GenerateE32Image();

		if (iE32ImageFile->WriteImage(e32))
		{
			//GetELF2E32()->AddFileCleanup(e32);
			delete iE32ImageFile;
		}
	}
	catch (...)
	{
		delete iE32ImageFile;
		throw;
	}
}

/**
Function to check image is Dll or not.
@return True if image is Dll, otherwise false.
@internalComponent
@released
*/
bool ElfFileSupplied::ImageIsDll()
{
	return (iReader->LookupStaticSymbol("_E32Dll") != NULL);
}

/**
Function to allocate E32 Image header.
@return pointer to E32ImageHeaderV
@internalComponent
@released
*/
E32ImageHeaderV * ElfFileSupplied::AllocateE32ImageHeader()
{
	if (iNumAbsentExports == 0)
	{
		return new E32ImageHeaderV;
	}

	int nexp = iExportTable.GetNumExports();

	size_t memsz = (nexp + 7) >> 3;	// size of complete bitmap
	size_t mbs = (memsz + 7) >> 3;	// size of meta-bitmap
	size_t nbytes = 0;
	PLUINT32 i;
	for (i=0; i<memsz; ++i)
		if (iExportBitMap[i] != 0xff)
			++nbytes;				// number of groups of 8
	PLUINT8 edt = KImageHdr_ExpD_FullBitmap;
	PLUINT32 extra_space = memsz - 1;
	if (mbs + nbytes < memsz)
	{
		edt = KImageHdr_ExpD_SparseBitmap8;
		extra_space = mbs + nbytes - 1;
	}
	extra_space = (extra_space + sizeof(PLUINT32) - 1) &~ (sizeof(PLUINT32) - 1);
	size_t aExtendedHeaderSize = sizeof(E32ImageHeaderV) + extra_space;
	iE32ImageFile->SetExtendedE32ImageHeaderSize(aExtendedHeaderSize);
	E32ImageHeaderV * aHdr = (E32ImageHeaderV *)new PLUINT8[aExtendedHeaderSize];

	iExportDescType = edt;
	if (edt == KImageHdr_ExpD_FullBitmap)
	{
		iExportDescSize = (PLUINT16)memsz;
		memcpy(aHdr->iExportDesc, iExportBitMap, memsz);
	}
	else
	{
		iExportDescSize = (PLUINT16) (mbs + nbytes);
		memset(aHdr->iExportDesc, 0, extra_space + 1);
		PLUINT8* mptr = aHdr->iExportDesc;
		PLUINT8* gptr = mptr + mbs;
		for (i=0; i<memsz; ++i)
		{
			if (iExportBitMap[i] != 0xff)
			{
				mptr[i>>3] |= (1u << (i&7));
				*gptr++ = iExportBitMap[i];
			}
		}
	}
	return aHdr;
}

/**
Function to create export bitmap
@internalComponent
@released
*/
void ElfFileSupplied::CreateExportBitMap()
{
	int nexp = iExportTable.GetNumExports();
	size_t memsz = (nexp + 7) >> 3;
	iExportBitMap = new PLUINT8[memsz];
	memset(iExportBitMap, 0xff, memsz);
	// skip header
	PLUINT32* exports = iExportTable.GetExportTable() + 1;
	PLUINT32 absentVal = iReader->EntryPointOffset() + iReader->GetROBase();
	iNumAbsentExports = 0;
	for (int i=0; i<nexp; ++i)
	{
		if (exports[i] == absentVal)
		{
			iExportBitMap[i>>3] &= ~(1u << (i & 7));
			++iNumAbsentExports;
		}
	}
}

/**
Function to get symbol type based on the encoded names.
@return CODE or DATA
@internalComponent
@released
*/
SymbolType ElfFileSupplied::SymbolTypeF(char * aName)
{
	size_t prefixLength = strlen("_ZTV");
	bool classImpedimenta = !strncmp(aName, "_ZTV", prefixLength) ||
					   !strncmp(aName, "_ZTI", prefixLength) ||
					   !strncmp(aName, "_ZTS", prefixLength) ;

	return classImpedimenta? SymbolTypeData : SymbolTypeCode;
}

/**
Function to get export description size
@return export description size
@internalComponent
@released
*/
PLUINT16 ElfFileSupplied::GetExportDescSize()
{
	return iExportDescSize;
}

/**
Function to get export description type
@return export description type
@internalComponent
@released
*/
PLUINT8 ElfFileSupplied::GetExportDescType()
{
	return iExportDescType;
}

/**
Function to indicate if the new exports are to be reported as warnings.
@return export description type
@internalComponent
@released
*/
bool ElfFileSupplied::WarnForNewExports()
{
	return EPolyDll != iManager->TargetTypeName();
}

/**
Function to provide a predicate which checks whether a symbol name is unwanted:
@return true if new symbols are present in the static library list else return false
@param aSymbol symbols to be checked if part of static lib
@internalComponent
@released
*/
bool UnWantedSymbol(const char * aSymbol)
{
	constexpr size_t symbollistsize = sizeof(Unwantedruntimesymbols) / sizeof(Unwantedruntimesymbols[0]);
	for (size_t i = 0; i<symbollistsize; i++)
	{
		if (strstr(Unwantedruntimesymbols[i], aSymbol))
			return true;
	}
	return false;
}

