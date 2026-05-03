// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * parse-option.h - header for parsing command-line options
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

#ifndef __PARSE_OPTION_H
#define __PARSE_OPTION_H

#include <stdint.h>
#include <stddef.h>

#include "common.h"

DECL_CONST(OPT_ARGLONG_MAX, 256);

DECL_FLAG(OPT_FLAG_HAS_SHORT, 1);
DECL_FLAG(OPT_FLAG_HAS_LONG, 2);
DECL_FLAG(OPT_FLAG_HAS_ARG, 3);

DECL_CONST(OPT_SHORT_ONLY, OPT_FLAG_HAS_SHORT);
DECL_CONST(OPT_LONG_ONLY, OPT_FLAG_HAS_LONG);
DECL_CONST(OPT_SHORT_LONG, OPT_FLAG_HAS_SHORT | OPT_FLAG_HAS_LONG);
DECL_CONST(OPT_SHORT_ARG, OPT_FLAG_HAS_SHORT | OPT_FLAG_HAS_ARG);
DECL_CONST(OPT_LONG_ARG, OPT_FLAG_HAS_LONG | OPT_FLAG_HAS_ARG);
DECL_CONST(OPT_SHORT_LONG_ARG,
        OPT_FLAG_HAS_SHORT | OPT_FLAG_HAS_LONG | OPT_FLAG_HAS_ARG);

struct flag;

/* Function pointer prototype for processing argument:
 *  int fn(struct flag *context, const char *arg)
 */
typedef int (*opt_flag_arg_parser)(struct flag *, const char *);

struct opt_flag
{
        uint8_t flag_opts;
        char short_name;
        const char *long_name;
        const char *help_text;
        opt_flag_arg_parser arg_parser;
        void *data;
        uint8_t flag_id;
        // TODO: Optimize member order
};

#define OPT_FLAG(id, short, long, flags, parser, data, help) \
        { .flag_id = id, .short_name = short, .long_name = long, \
          .flag_opts = flags, .arg_parser = parser, .data = data, \
          .help_text = help }

#define OPT_FLAG_LONGONLY(id, long, parser, data, help) \
        OPT_FLAG(id, 0, long, OPT_LONG_ONLY, parser, data, help)
#define OPT_FLAG_SHORTONLY(id, short, parser, data, help) \
        OPT_FLAG(id, short, NULL, OPT_SHORT_ONLY, parser, data, help)
#define OPT_FLAG_SHORTLONG(id, short, long, parser, data, help) \
        OPT_FLAG(id, short, long, OPT_SHORT_LONG, parser, data, help)
#define OPT_FLAG_SHORTARG(id, short, parser, data, help) \
        OPT_FLAG(id, short, NULL, OPT_SHORT_ARG, parser, data, help)
#define OPT_FLAG_LONGARG(id, long, parser, data, help) \
        OPT_FLAG(id, 0, long, OPT_LONG_ARG, parser, data, help)
#define OPT_FLAG_SHORTLONGARG(id, short, long, parser, data, help) \
        OPT_FLAG(id, short, long, OPT_SHORT_LONG_ARG, parser, data, help)

#define OPT_FLAG_LONGONLY_NODATA(id, long, parser, help) \
        OPT_FLAG_LONGONLY(id, long, parser, NULL, help)
#define OPT_FLAG_SHORTONLY_NODATA(id, short, parser, help) \
        OPT_FLAG_SHORTONLY(id, short, parser, NULL, help)
#define OPT_FLAG_SHORTLONG_NODATA(id, short, long, parser, help) \
        OPT_FLAG_SHORTLONG(id, short, long, parser, NULL, help)
#define OPT_FLAG_SHORTARG_NODATA(id, short, parser, help) \
        OPT_FLAG_SHORTARG(id, short, parser, NULL, help)
#define OPT_FLAG_LONGARG_NODATA(id, long, parser, help) \
        OPT_FLAG_LONGARG(id, long, parser, NULL, help)
#define OPT_FLAG_SHORTLONGARG_NODATA(id, short, long, parser, help) \
        OPT_FLAG_SHORTLONGARG(id, short, long, parser, NULL, help)

struct opt_argument
{
        const char *arg_str;
        struct opt_argument *next;
};

struct opt_arg_list
{
        size_t count;
        struct opt_argument *first;
        struct opt_argument *last;
};

struct opt_parser
{
        struct opt_flag **flags;
        struct opt_arg_list *arguments;
        size_t opt_index;
};

DECL_CONST(OPT_RET_SUCCESS, 0);
DECL_CONST(OPT_RET_ERR_INVALID, -1);
DECL_CONST(OPT_RET_ERR_FAILED, -2);
DECL_CONST(OPT_RET_ERR_NOMEM, -3);
DECL_CONST(OPT_RET_ERR_SEMANTIC, -4);
DECL_CONST(OPT_RET_ERR_NOPARSER, -5);

/**
 * opt_parse(arglist, flags)
 * @fn int opt_parse(const char *const *arglist,
 *                   struct opt_parser *parser)
 * @brief Parses command-line options based on the provided flag
 *        definitions
 * @param arglist The list of command-line arguments (argv)
 * @param flags The array of opt_flag definitions to match against
 * @return OPT_RET_SUCCESS on success or error code on failure
 * @description This function iterates through the provided arglist and
 *         matches each argument against the provided opt_flag
 *         definitions. When non-flags (args) are found other than
 *         after options, or when the end of options marker ("--") is
 *         found, the remainder arguments are stored into the parser's
 *         arguments list. If an error occurs during parsing, the
 *         function returns the error and keeps the opt_index at its
 *         current position to inform the caller where the error happens.
 * @notes The argument list is dynamically allocated and is only freed
 *        when opt_free() is called on the parser.
 */
int opt_parse(const char *const *arglist, struct opt_parser *parser);

/**
 * opt_parser *opt_init()
 * @fn struct opt_parser *opt_init()
 * @brief Initializes an opt_parser structure for storing parsed options
 * @return A pointer to the initialized opt_parser structure or NULL
 *         on failure.
 */
struct opt_parser *opt_init();

/**
 * opt_free(opt)
 * @fn void opt_free(struct opt_parser *opt)
 * @brief Frees the memory allocated for an opt_parser structure and
 *        its arguments list.
 * @param opt The opt_parser structure to free
 */
void opt_free(struct opt_parser *opt);

/**
 * opt_arg_add(opt_arg_list, arg)
 * @fn int opt_arg_add(struct opt_arg_list *arg_list, const char *arg)
 * @brief Adds an argument string to the opt_arg_list
 * @param arg_list The opt_arg_list to add the argument to
 * @param arg The argument string to add
 * @return OPT_RET_SUCCESS on success or error code on failure
 */
int opt_arg_add(struct opt_arg_list *arg_list, const char *arg);

#endif//__PARSE_OPTION_H