// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * common.c - implementation for common definitions and utilities
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

#include "common.h"

#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

// TODO: Remove those includes later
#include <stdio.h>

struct wrapped_string *wrapped_string_init(const char *str,
                                           size_t length)
{
        struct wrapped_string *ret = NULL;
        char *c_str = NULL;

        CHECK_NULL(str);

        ret = malloc(sizeof(struct wrapped_string));
        CHECK_NULL(ret);

        bzero(ret, sizeof(struct wrapped_string));

        if (length == 0) {
                c_str = strdup(str);
                ret->length = strlen(str);
        }
        else {
                c_str = strndup(str, length);
                ret->length = length;
        }

        if (c_str == NULL) {
                free(ret);
                return NULL;
        }

        ret->start_string = c_str;
        return ret;
}

void wrapped_string_free(struct wrapped_string *wrapstr)
{
        RET_IF_NULL(wrapstr, );

        if (wrapstr->start_string != NULL) {
                free(wrapstr->start_string);
        }

        free(wrapstr);
}

#define NOTNUL(c) (c != CHR_NUL)

#define UNPRINT_OR_SPACE(c) (!isprint(c) || isspace(c))
#define UNWANTED(c) (UNPRINT_OR_SPACE(c) || iscntrl(c))
#define UNWRAPPABLE(c) (isgraph(c) && !iscntrl(c) && !isblank(c))
#define SKIP_CLASS(ptr, s, len, class_fn) do { \
        while (WITHIN_RANGE(ptr, s, len) && class_fn(*ptr) && NOTNUL(*ptr)) { \
                ptr++; \
        } \
} while (0)
#define SKIP_BLANKS(ptr, s, len) SKIP_CLASS(ptr, s, len, isblank)
#define SKIP_UNWANTED(ptr, s, len) \
        SKIP_CLASS(ptr, s, len, UNWANTED)
#define SKIP_UNPRINT(ptr, s, len) \
        SKIP_CLASS(ptr, s, len, !isprint)

#define REWIND_CLASS(ptr, s, len, class_fn) do { \
        IF_DEBUGMACRO(char *___bkp_ptr = ptr;) \
        DEBUGMACRO("REWIND_CLASS", \
                "rewind: len=%zu, ptr-s=%zu, ptr='%.10s [...]' (%p), " \
                "s='%.30s [...]' (%p)\n", len, ptr - s, ptr, ptr, s, \
                s); \
        DEBUGMACRO_CHECK_RANGE(ptr, s, len); \
        DEBUGMACRO_CHECK_COND(class_fn(*ptr)); \
        DEBUGMACRO_CHECK_COND(ptr > s); \
        while (WITHIN_RANGE(ptr, s, len) && class_fn(*ptr) && ptr > s) { \
                ptr--; \
                DEBUGMACRO("REWIND_CLASS", "rewind: loop: rewinded *ptr=%c\n", *ptr); \
        } \
        DEBUGMACRO("REWIND_CLASS", "rewind done, ptr moved: prev=%p, new=%p, moved=%td\n\n\n", ___bkp_ptr, ptr, ___bkp_ptr - ptr); \
} while (0)
#define REWIND_UNWANTED(ptr, s, len) REWIND_CLASS(ptr, s, len, UNWANTED)
#define REWIND_UNWRAPPABLE(ptr, s, len) REWIND_CLASS(ptr, s, len, UNWRAPPABLE)

int str_wrapper(struct wrapped_string *wrapstr, size_t wrap_length)
{
        size_t maxlen;
        size_t maxitr = STR_WRAPPER_MAX_TOKENS;
        char *str = NULL;
        char *ptr = NULL;
        char *nptr = NULL;

        RET_IF_NULL(wrapstr, STR_WRAPPER_EINVAL);
        RET_IF_NULL(wrapstr->start_string, STR_WRAPPER_EINVAL);
        RET_IF(wrap_length == 0, STR_WRAPPER_EINVAL);

        str = wrapstr->start_string;
        ptr = str;
        nptr = ptr + wrap_length;

        maxlen = str_clean(str, wrapstr->length);
        wrapstr->token_count = 0;

        while (WITHIN_RANGE(nptr, str, maxlen) &&
                wrapstr->token_count < STR_WRAPPER_MAX_TOKENS) {

                REWIND_UNWRAPPABLE(nptr, ptr+1, wrap_length);
                if (nptr <= ptr) {
                        return STR_WRAPPER_EUNFIT;
                }

                ptr = nptr;
                *ptr++ = CHR_NUL;
                wrapstr->tokens[wrapstr->token_count++] = ptr;

                nptr = ptr + wrap_length;
        }

        return STR_WRAPPER_SUCCESS;

}

size_t str_clean(char *str, size_t length)
{
        char *ins = str;
        char *ptr = str;
        size_t new_length = 0;

        RET_IF(str == NULL, 0);
        RET_IF(length == 0, 0);

        // skip leading whitespace and invisible chars
        SKIP_UNWANTED(ptr, str, length);

        // copy everything but repeated spaces and unwanted chars
        while(WITHIN_RANGE(ptr, str, length + (ptr-ins)) && NOTNUL(*ptr)) {

                if (isblank(*ptr)) {
                        *ins++ = ' ';
                        SKIP_UNWANTED(ptr, str, length);
                        continue;
                }
                SKIP_UNPRINT(ptr, str, length);
                assert(ins < ptr);
                *ins = *ptr;
                ins++;
                ptr++;
        }

        // now rewind to remove trailing spaces/unwanted chars (if any)
        REWIND_UNWANTED(ins, str, length);
        new_length = ins - str;
        *ins = CHR_NUL;

        return new_length;

}