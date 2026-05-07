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

#define _GNU_SOURCE

#include <stddef.h>

#define GET_IDX(ptr, idx) (ptr + idx)

#define WITHIN_RANGE_BEFORE(ptr, bptr, offset) (ptr < bptr+offset)
#define WITHIN_RANGE_AFTER(ptr, bptr) (ptr >= bptr)
#define WITHIN_RANGE(ptr, bptr, offset) \
        (WITHIN_RANGE_BEFORE(ptr, bptr, offset) && \
        WITHIN_RANGE_AFTER(ptr, bptr))

#define RET_IF(cond, retval) do { \
        if (cond) { \
                return retval; \
        } \
} while (0)

#define __STR(s) #s
#define STR(s) __STR(s)

#ifdef __DEBUG_USE_COLORS
// Yellow for the prefix brackets, green for the 'DEBUG' word (label),
// violet for the file, blue for the line, cyan for the function name
#define DEBUG_COLOR_PREFIX_LABEL "\033[1;32m" //green
#define DEBUG_COLOR_PREFIX_BRACKETS "\033[1;33m"
#define DEBUG_COLOR_PREFIX_TEXT "\033[1;32m"
#define DEBUG_COLOR_PREFIX_FILE "\033[1;34m"
#define DEBUG_COLOR_PREFIX_LINE "\033[1;31m"
#define DEBUG_COLOR_PREFIX_FUNC "\033[1;35m"
#define DEBUG_COLOR_RESET "\033[0m"

// What color is good for the actual text of a debug message?
// A light color that stands out from the prefix colors and also from regular
// output, so NO WHITE. I like light magenta, try it.
#define DEBUG_COLOR_MSG "\033[1;36m"
// But heyy, it's repeated, suggest another one: light cyan? 
// let me see the color code: \033[1;36m, it's already used for the function name. 
//Maybe light blue? \033[1;34m, but it's used for the line number. 
//Maybe light green? \033[1;32m, but it's used for the label. 
//Maybe light yellow? \033[1;33m, but it's used for the brackets. 
//Maybe light red? , that could work, let's try it.

#define DEBUG_PREFIX DEBUG_COLOR_PREFIX_BRACKETS "[" \
        DEBUG_COLOR_RESET DEBUG_COLOR_PREFIX_LABEL "DEBUG " \
        DEBUG_COLOR_RESET DEBUG_COLOR_PREFIX_FILE "%s" \
        DEBUG_COLOR_RESET DEBUG_COLOR_PREFIX_BRACKETS ":" \
        DEBUG_COLOR_RESET DEBUG_COLOR_PREFIX_LINE "%d" \
        DEBUG_COLOR_RESET DEBUG_COLOR_PREFIX_BRACKETS "]" \
        DEBUG_COLOR_RESET " " DEBUG_COLOR_PREFIX_FUNC "%s()" \
        DEBUG_COLOR_RESET ": " DEBUG_COLOR_MSG

#define DEBUG_SUFFIX DEBUG_COLOR_RESET "\n"

// A different color I think, maybe NOT CYAN AT ALL, something not used before
// Yellow's been used just a little for the brackets
#define DEBUG_MACRO_NAME_COLOR "\033[1;33m"

#define DEBUG_MACRO_PREFIX DEBUG_COLOR_RESET "MACRO: " \
        DEBUG_MACRO_NAME_COLOR "%s" DEBUG_COLOR_RESET ": " \
        DEBUG_COLOR_MSG
#define DEBUG_MACRO_TEST_PREFIX DEBUG_COLOR_RESET DEBUG_MACRO_NAME_COLOR
#define DEBUG_MACRO_TEST_SUFFIX DEBUG_COLOR_RESET DEBUG_COLOR_MSG

#else//__DEBUG_USE_COLORS
#define DEBUG_PREFIX "[DEBUG %s:%d] %s(): "
#define DEBUG_SUFFIX "\n"
#define DEBUG_MACRO_PREFIX "MACRO: %s"
#define DEBUG_MACRO_TEST_PREFIX ""
#define DEBUG_MACRO_TEST_SUFFIX ""
#endif//__DEBUG_USE_COLORS

#ifdef __DEBUG
#define IF_DEBUG_GETENV(var, code) do { \
        if (getenv(var) != NULL) { \
                code; \
        } \
} while (0)
#define DEBUG(fmt, ...) \
        IF_DEBUG_GETENV("DEBUG", fprintf(stderr, DEBUG_PREFIX fmt DEBUG_SUFFIX, __FILE__, \
                __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__));
#define IF_DEBUG(code) code
#else//__DEBUG
#define DEBUG(fmt, ...)
#define IF_DEBUG(code)
#endif//__DEBUG

#ifdef __DEBUG_MACROS
#define DEBUGMACRO(macro, fmt, ...) IF_DEBUG_GETENV("DEBUG_MACROS", \
        DEBUG(DEBUG_MACRO_PREFIX fmt, \
                macro __VA_OPT__(,) __VA_ARGS__))
#define DEBUGMACRO_REG(fmt, ...) IF_DEBUG_GETENV("DEBUG_MACROS", \
        DEBUG(fmt __VA_OPT__(,) __VA_ARGS__))
#define DEBUGMACRO_CHECK_RANGE(ptr, bptr, offset) IF_DEBUG_GETENV("DEBUG_MACROS", \
        DEBUGMACRO_REG(DEBUG_MACRO_TEST_PREFIX "CHECKING RANGE: " \
                DEBUG_MACRO_TEST_SUFFIX "ptr=%p, bptr=%p, offset=%zu" \
                ", ptr-bptr=%zu result=%d\n[DEBUG] TEST DETAILS:\v%s" \
                "\n\n\n", ptr, bptr, offset, ptr-bptr, \
                WITHIN_RANGE(ptr, bptr, offset), \
                STR(WITHIN_RANGE(ptr, bptr, offset))))
#define DEBUGMACRO_CHECK_COND(cond) IF_DEBUG_GETENV("DEBUG_MACROS", \
        DEBUGMACRO_REG(DEBUG_MACRO_TEST_PREFIX "TESTING CONDITION: " \
                DEBUG_MACRO_TEST_SUFFIX "result=%d\n[DEBUG] " \
                "TEST DETAILS:\v%s" \
                "\n\n\n", (cond), STR(cond)))
#define IF_DEBUGMACRO(code) code
#else//__DEBUG_MACROS
#define DEBUGMACRO(fmt, ...)
#define DEBUGMACRO_REG(fmt, ...)
#define DEBUGMACRO_CHECK_RANGE(ptr, bptr, offset)
#define DEBUGMACRO_CHECK_COND(cond)
#define IF_DEBUGMACRO(code)
#endif//__DEBUG_MACROS

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