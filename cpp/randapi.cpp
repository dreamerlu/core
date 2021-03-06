/*
 * Copyright (c) 2012-2020 MIRACL UK Ltd.
 *
 * This file is part of MIRACL Core
 * (see https://github.com/miracl/core).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "randapi.h"

using namespace core;

/* Initialise a Cryptographically Strong Random Number Generator from
   an octet of raw random data */

void core::CREATE_CSPRNG(csprng *RNG, octet *RAW)
{
    RAND_seed(RNG, RAW->len, RAW->val);
}

void core::KILL_CSPRNG(csprng *RNG)
{
    RAND_clean(RNG);
}

