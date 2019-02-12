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
// Contributors: Strizhniou Fiodar - fix build and runtime errors, refactoring.
//
// Description:
// Implementation of the Header file for Class ParameterManager of the elf2e32 tool
// @internalComponent
// @released
//
//

#if !defined(SYMBIAN_PARAMETERMANAGER_H_)
#define SYMBIAN_PARAMETERMANAGER_H_

#include <portable.h>
#include <vector>
#include <map>
#include <string>

struct Arguments
{
    char *defInFile = nullptr;
    char *defOutFile = nullptr;
    char *E32InFile = nullptr;
    char *E32OutFile = nullptr; // --output
    char *dsoOutFile = nullptr;
    char *elfFile = nullptr; // --elfinput
    const char *fileDumpOpt = nullptr; // --dump
    char *linkAsOpt = nullptr;
};

enum ETargetType
{
	ETargetTypeNotSet = - 2,
	EInvalidTargetType = - 1,
	/** Target type is static Library */
	ELib,
	/** Target type is DLL */
	EDll,
	EExe,
	EPolyDll,
	EExexp,
	EStdExe
};

typedef uint32_t UINT;

class ParameterManager
{
private:
    ParameterManager(){}
    ParameterManager(const ParameterManager& other) = delete;
    ParameterManager& operator=(const ParameterManager&) = delete;

public:
    static ParameterManager *GetInstance(int argc, char** argv, E32ImageHeader* aHdr);
    static ParameterManager *Static();
	virtual ~ParameterManager();

	void CheckOptions();

	struct OptionDesc
	{
		const char * iName;
		const void * iParser;
		const char * iDoc;
	};

	struct PriorityValueDesc
	{
		const char * iName;
		TProcessPriority iPriority;
	};

	struct CompressionMethodDesc
	{
		const char *iMethodName;
		UINT		iMethodUid;
	};

	struct SysDefs
	{
		int iSysDefOrdinalNum;
		std::string iSysDefSymbolName;
	};

	typedef SysDefs Sys;

	typedef std::less<std::string> OptionCompare;
	typedef std::map<std::string, const OptionDesc *, OptionCompare> OptionMap;
	typedef std::vector<std::string> LibSearchPaths;

	typedef void (*ParserFn)(ParameterManager *, const char *, char *, const OptionDesc *);


	#define DECLARE_PARAM_PARSER(name) \
		static void name(ParameterManager* aPM, char* aOption, char* aValue, void* aDesc)

	#define DEFINE_PARAM_PARSER(name) \
		void name(ParameterManager * aPM, char * aOption, char* aValue, void * aDesc)

	#define INITIALISE_PARAM_PARSER \
		aOption = aOption; \
		aValue = aValue; \
		aDesc = aDesc;

	DECLARE_PARAM_PARSER(ParseDefInput);
	DECLARE_PARAM_PARSER(ParseDSOOutput);
	DECLARE_PARAM_PARSER(ParseTargetTypeName);
	DECLARE_PARAM_PARSER(ParseLinkAs);
	DECLARE_PARAM_PARSER(ParseDefOutput);
	DECLARE_PARAM_PARSER(ParseElfInput);
	DECLARE_PARAM_PARSER(ParseFileDump);
	DECLARE_PARAM_PARSER(ParseE32ImageInput);
	DECLARE_PARAM_PARSER(ParseOutput);
	DECLARE_PARAM_PARSER(ParseLogFile);
	DECLARE_PARAM_PARSER(ParseMessageFile);
	DECLARE_PARAM_PARSER(ParseDumpMessageFile);

	DECLARE_PARAM_PARSER(ParamHelp);
	DECLARE_PARAM_PARSER(ParseUID1);
	DECLARE_PARAM_PARSER(ParseUID2);
	DECLARE_PARAM_PARSER(ParseUID3);


	DECLARE_PARAM_PARSER(ParseCapability);
	DECLARE_PARAM_PARSER(ParseSecureId);
	DECLARE_PARAM_PARSER(ParseVendorId);
	DECLARE_PARAM_PARSER(ParseFixedAddress);
	DECLARE_PARAM_PARSER(ParseUncompressed);
	DECLARE_PARAM_PARSER(ParseCompressionMethod);
	DECLARE_PARAM_PARSER(ParseHeap);
	DECLARE_PARAM_PARSER(ParseStackCommitted);
	DECLARE_PARAM_PARSER(ParseUnfrozen);
	DECLARE_PARAM_PARSER(ParseIgnoreNonCallable);
	DECLARE_PARAM_PARSER(ParseLibPaths);
	DECLARE_PARAM_PARSER(ParseSysDefs);
	DECLARE_PARAM_PARSER(ParseAllowDllData);
	DECLARE_PARAM_PARSER(ParsePriority);
	DECLARE_PARAM_PARSER(ParseVersion);
	DECLARE_PARAM_PARSER(ParseCallEntryPoint);
	DECLARE_PARAM_PARSER(ParseFPU);

	DECLARE_PARAM_PARSER(ParsePaged);
	DECLARE_PARAM_PARSER(ParseUnpaged);
	DECLARE_PARAM_PARSER(ParseDefaultPaged);

	DECLARE_PARAM_PARSER(ParseCodePaging);
	DECLARE_PARAM_PARSER(ParseDataPaging);

	DECLARE_PARAM_PARSER(ParseExcludeUnwantedExports);
	DECLARE_PARAM_PARSER(ParseIsCustomDllTarget);
	DECLARE_PARAM_PARSER(ParseSymNamedLookup);
	DECLARE_PARAM_PARSER(ParseDebuggable);
	DECLARE_PARAM_PARSER(ParseSmpSafe);

	/**
    This function parses the command line options and sets the appropriate values based on the
    input options.
    @internalComponent
    @released
    */
	void ParameterAnalyser();

	void SetTargetTypeName(ETargetType  aSetTargetTypeName);
	void SetLinkDLLName(char * aSetLinkDLLName);
	void SetUID1(UINT aSetUINT1);
	void SetUID2(UINT aSetUINT2);
	void SetUID3(UINT aSetUINT3);

	void SetCompressionMethod(UINT aCompressionMethod);
	void SetSecureId(UINT aSetSecureID);
	void SetVendorId(UINT aSetVendorID);
	void SetHeapReservedSize(UINT aSetHeapReservedSize);
	void SetStackCommittedSize(UINT aSetStackCommittedSize);
	void SetUnfrozen(bool aVal);
	void SetIgnoreNonCallable(bool aVal);
	void SetCapability(unsigned int newVal);
	void SetCapability(SCapabilitySet & newVal);
	void SetSysDefs(unsigned int aSysDefOrdinal, std::string aSysDefSymbol, int aCount);
	void SetLogFile(char * aSetLogFile);
	void SetMessageFile(char *aMessageFile);
	void SetDumpMessageFile(char *aDumpMessageFile);
	void SetPriority(TProcessPriority anewVal);
	void SetVersion(UINT aSetVersion);
	void SetCallEntryPoint(bool aCallEntryPoint);
	void SetFPU(UINT aVal);

	void SetCodePaged(bool);
	void SetCodeUnpaged(bool);
	void SetCodeDefaultPaged(bool);

	void SetDataPaged(bool);
	void SetDataUnpaged(bool);
	void SetDataDefaultPaged(bool);

	void SetExcludeUnwantedExports(bool aVal);
	void SetCustomDllTarget(bool aVal);
	void SetSymNamedLookup(bool aVal);
	void SetDebuggable(bool aVal);
	void SetSmpSafe(bool aVal);

	int NumOptions();
	int NumShortOptions();
	void InitParamParser();
	void ParseCommandLine();
	char * Path(char * aArg);

	/**
    This function extracts the target type that is passed as input through the --targettype option.
    @internalComponent
    @released
    @return the name of the input target type if provided as input through --targettype or 0.
    */
	ETargetType TargetTypeName();

	ETargetType ValidateTargetType(const char * aTargetType);

	/**
    This function extracts the path (where the intermediate libraries should be put)
    that is passed as input through the --libpath option.
    @internalComponent
    @released
    @return the path if provided as input through --libpath or 0.
    */
	LibSearchPaths& LibPath();

	/**
    This function extracts the DEF file name that is passed as input through the --definput option.
    @internalComponent
    @released
    @return the name of the input DEF file if provided as input through --definput or 0.
    */
	char * DefInput();

	/**
    This function return the Elf file name that is passed as input through the --elfinput option.
    @internalComponent
    @released
    @return the name of the input Elf file if provided as input through --elfinput or 0.
    */
	const std::string& ElfInput();

    /**
    This function extracts the E32 image name that is passed as input through the --e32dump option.
    @internalComponent
    @released
    @return the name of the input E32 image if provided as input through --e32dump or 0.
    */
	char * E32Input();

	bool SysDefOption();
	bool LogFileOption();
	bool MessageFileOption();
	bool HasDllData();
	TProcessPriority Priority();
	bool PriorityOption();
	bool CallEntryPoint();

	/**
    This function extracts the output DEF file name that is passed as input through the --defoutput option.
    @internalComponent
    @released
    @return the name of the output DEF file if provided as input through --defoutput or 0.
    */
	char * DefOutput();

	/**
    This function extracts the DSO file name that is passed as input through the --dso option.
    @internalComponent
    @released
    @return the name of the output DSO file if provided as input through --dso or 0.
    */
	char * DSOOutput();

	/**
    This function extracts the E32 image output that is passed as input through the --output option.
    @internalComponent
    @released
    @return the name of the output E32 image output if provided as input through --output or 0.
    */
	char * E32ImageOutput();

    /**
    This function extracts the name of the DLL (that the DSO is to be linked with)
    that is passed as input through the --linkas option.
    @internalComponent
    @released
    @return the name of the DLL name to be linked with if provided as input through --linkas or 0.
    */
	char * LinkAsDLLName();

	/**
    This function extracts the filename from the absolute path that is given as input.
    @internalComponent
    @released
    @param aFileName
    The filename alongwith the absolute path.
    @return the filename (without the absolute path) for valid input else the filename itself.
    */
	char * FileName(char * aArg);

	char * LogFile();
	char * MessageFile();
	char * DumpMessageFile();
	const char * FileDumpOptions();

	int SysDefCount();
	Sys SysDefSymbols(int count);

	UINT Version();
	bool FixedAddress();

	UINT CompressionMethod();
	uint32_t HeapCommittedSize();
	uint32_t HeapReservedSize();
	uint32_t StackCommittedSize();
	bool Unfrozen();
	bool IgnoreNonCallable();
	SCapabilitySet Capability();
	void ParseCapability1(std::string CapabilityList, SCapabilitySet& aCapabilities, bool invert);
	void ParseCapabilitiesArg(SCapabilitySet& aCapabilities, const char *aText);
	uint32_t FPU();

	bool IsCodePaged();
	bool IsCodeUnpaged();
	bool IsCodeDefaultPaged();

	bool IsDataPaged();
	bool IsDataUnpaged();
	bool IsDataDefaultPaged();

	bool ExcludeUnwantedExports();
	bool IsCustomDllTarget();
	bool SymNamedLookup();
	bool IsDebuggable();
	bool IsSmpSafe();

	E32ImageHeader *GetE32Header();
	SSecurityInfo *GetSSecurityInfo();

private:
    E32ImageHeader *iE32Header = nullptr;
    Arguments iOptionArgs;
    SSecurityInfo iSecInfo;

	/** The number of command line arguments passed into the program */
	int iArgc;

	/** The listing of all the arguments */
	std::vector<char *> iArgv;

	/** REVISIT */
	char * iImageLocation = nullptr;

	/** REVISIT */
	char * iImageName = nullptr;

	bool iFixedAddress = false;

	bool iUnfrozen = false;
	bool iIgnoreNonCallable = false;

	/** The list of command line options (with normal prefix '--') that will be accepted by the program */
	static const OptionDesc iOptions[];

	/** The list of command line options (with short prefix '-') that will be accepted by the program */
	static const OptionDesc iShortOptions[];

	/** The map between the command line option (with normal prefix '--') and the corresponding function */
	OptionMap iOptionMap;

	/** The map between the command line option (with short prefix '-') and the corresponding function */
	OptionMap iShortOptionMap;

	/** Target Type that is passed as input to the --targettype option */
	ETargetType iTargetTypeName = ETargetTypeNotSet;

	/** File name of the input DSO file passed as input to the --elfinput option */
	std::string iElfInput;

	/** Path name of the intermediate libraries passed as input to the --libpath option */
	char * iLibPath = nullptr;

	bool iSysDefOption = false;
	char * iLogFileName = nullptr;
	char * iMessageFileName = nullptr;
	char * iDumpMessageFileName = nullptr;

	bool iDllData = false;

	LibSearchPaths iLibPathList;
	SCapabilitySet iCapability;
	//struct SysDefs iSysDefSymbols[10];
	Sys iSysDefSymbols[10];
	int iSysDefCount = 0;
	bool iPriorityOption = false;
	TProcessPriority iPriorityVal = (TProcessPriority)0;
	UINT iVersion = 0x000a0000u;
	bool iVersionOption = false;
	bool iCallEntryPoint = true;
	UINT iFPU = 0;

	bool iCodePaged			= false;
	bool iCodeUnpaged		= false;
	bool iCodeDefaultPaged	= false;

	bool iDataPaged			= false;
	bool iDataUnpaged		= false;
	bool iDataDefaultPaged	= false;

	bool iExcludeUnwantedExports = false;
	bool iCustomDllTarget = false;
	bool iSymNamedLookup = false;
	bool iDebuggable = false;
	bool iSmpSafe = false;
	bool iSSTDDll = false;
};


#endif // !defined(SYMBIAN_PARAMETERMANAGER_H_)
