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
// Class for creation E32 image in the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef E32PRODUCER_H
#define E32PRODUCER_H

struct E32ImageHeader;
class ParameterManager;

class E32Producer
{
    public:
        E32Producer(ParameterManager *args);
        ~E32Producer();

        void Run();
    private:
        void ReCompress();
        void MakeE32();
        void SaveE32(const char* s, size_t size);
    private:
        E32ImageHeader *iE32Hdr = nullptr;
        ParameterManager *iMan = nullptr;
};

#endif // E32PRODUCER_H
