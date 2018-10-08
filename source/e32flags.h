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
// Class for first-time initialization of E32ImageHeader::iFlags.
//      For details see E32Flags::Run().
// @internalComponent
// @released
//
//

#ifndef E32FLAGS_H
#define E32FLAGS_H

#include "parametermanager.h"


class E32Flags
{
    public:
        E32Flags(ParameterManager *args);
        ~E32Flags();
        uint32_t Run();

    private:
        void SetSymbolLookup();
        void SetFPU();
        void SetDebuggable();
        void SetSmpSafe();
        void SetCallEntryPoints();
        void SetPaged();

    private:
        ParameterManager *iArgs = nullptr;
        uint32_t iFlags = 0;
};

#endif // E32FLAGS_H
