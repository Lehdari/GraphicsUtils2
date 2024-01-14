//
// Project: GraphicsUtils2
// File: Macros.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#if defined(__GNUC__)
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE inline
#endif
