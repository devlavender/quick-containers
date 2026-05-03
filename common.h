// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * common.h - header for common definitions and utilities
 *
 * THIS FILE IS PART OF:
 * quick-container - a simple container implementation aiming at being a
 * 'chroot with namespaces'
 * 
 * COPYRIGHT NOTICE:
 * 
 * Copyright (C) 2025, Agatha Isabelle Moreira Guedes <code@agatha.dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * license or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <https://www.gnu.org/licenses/agpl-3.0.txt>.
 * 
 */

#ifndef __COMMON_H
#define __COMMON_H

#define GET_IDX(ptr, idx) (ptr + idx)

#define RET_IF(cond, retval) do { \
        if (cond) { \
                return retval; \
        } \
} while (0)

#define RET_IF_NULL(ptr, retval) RET_IF((ptr) == NULL, retval)
#define CHECK_NULL(ptr) RET_IF_NULL(ptr, NULL)

#define DECL_BIT(bit) (1 << (bit - 1))

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 202311L
#define DECL_FLAG(name, bit) constexpr uint8_t name = DECL_BIT(bit)
#define DECL_TCONST(name, value, type) constexpr type name = value
#define DECL_CONST(name, code) DECL_CONST_TYPE(name, int, code)
#else
#define DECL_FLAG(name, bit) enum { name = DECL_BIT(bit) }
#define DECL_TCONST(name, value, type) enum { name = value }
#define DECL_CONST(name, code) DECL_CONST_TYPE(name, int, code)
#endif
#else
#define DECL_TCONST(name, value, type) enum { name = value }
#define DECL_CONST(name, code) DECL_CONST_TYPE(name, int, code)
#endif


#endif//__COMMON_H