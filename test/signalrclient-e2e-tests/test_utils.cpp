// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "stdafx.h"
#include <string>
#include "cpprest\details\basic_types.h"

utility::string_t get_url(int argc, _TCHAR* argv[])
{
    utility::string_t url(U("http://localhost:8081/"));

    for (int i = 0; i < argc; ++i)
    {
        utility::string_t str = argv[i];

        if (str.find(U("url=")) != std::string::npos)
        {
            size_t pos = str.find(U("url="));
            url = str.substr(pos + 4);
            break;
        }
    }

    return url;
}
