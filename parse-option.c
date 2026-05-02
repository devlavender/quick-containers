// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * parse-option.c - implementation for parsing command-line options
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

#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>

#include "parse-option.h"
#include "common.h"

/**
 * is_longopt(opt)
 * @fn bool is_longopt(const char *opt, bool partial_parsing)
 * @brief Helper function to check if currently parsed option is long
 * @param opt The option string to check
 * @param partial_parsing If true assume the first hyphen to be already
 *        parsed.
 * @return true if the option is a long option, false otherwise
 * @details A long opt starts with "--" and is followed by alphanumeric
 *          characters, hyphens or underlines. If partial_parsing is
 *          true, It's expected that the first hyphen was the previously
 *          parsed character, so we check the current.
 * @note This function does not check if it's valid, just if it matches
 *       the long option format.
*/
static inline bool is_longopt(const char *opt, bool partial_parsing)
{
        return (!partial_parsing && *GET_IDX(opt, 0) == '-' && 
                *GET_IDX(opt, 1) == '-' && isalpha(*GET_IDX(opt, 2))) ||
               (partial_parsing && *GET_IDX(opt, 0) == '-' &&
               isalpha(*GET_IDX(opt, 1)));
}

/**
 * is_shortopt(opt)
 * @fn bool is_shortopt(const char *opt)
 * @brief Helper function to check if currently parsed option is a short
 *        option.
 * @param opt The option string to check
 * @return true if the option is a short option, false otherwise
 * @details A short opt starts with a single hyphen and is followed by a
 *          single alphanumeric character. This function does not check if
 *          it's valid, just if it matches the short option format.
 * @note This function does not check if it's valid, just if it matches
 *       the short option format.
 */
static inline bool is_shortopt(const char *opt)
{
        return *GET_IDX(opt, 0) == '-' && isalpha(*GET_IDX(opt, 1)) &&
               *GET_IDX(opt, 2) == '\0';
}

/**
 * find_flag(arg, flag_list)
 * @fn struct opt_flag *find_flag(const char *arg,
 *                                struct opt_flag **flag_list)
 * @brief Helper function to find the opt_flag definition matching the
 *        provided argument string.
 * @param arg The argument string to match
 * @param flag_list The list of opt_flag definitions to search through
 * @return The matching opt_flag definition if found, NULL otherwise
 * @details This function iterates through the provided flag_list and checks
 *          if any of the opt_flag definitions match the provided argument
 */
static struct opt_flag *find_flag(const char *arg,
                                  struct opt_flag **flag_list)
{
        struct opt_flag *ret = NULL;
        size_t i = 0;

        CHECK_NULL(arg);
        CHECK_NULL(flag_list);
        CHECK_NULL(*flag_list); //See if it's not empty

        while ((ret = *GET_IDX(flag_list, i++)) != NULL) {
                if (is_shortopt(arg) &&
                    ret->flag_opts & OPT_FLAG_HAS_SHORT) {
                        if (ret->short_name == *GET_IDX(arg, 1)) return ret;
                        continue;
                }
                if (is_longopt(arg, false) &&
                    ret->flag_opts & OPT_FLAG_HAS_LONG) {
                        if (!strncmp(arg, ret->long_name, OPT_ARGLONG_MAX)) {
                                return ret;
                        }
                        continue;
                }
        }

        return NULL;
}

int opt_parse(const char *const *arglist, struct opt_parser *parser)
{
        struct opt_flag *curopt = NULL;
        char **arg_ptr = (char **)arglist;
        int ret = 0;
        size_t i = 0;
        size_t inc = 0;

        RET_IF_NULL(arglist, OPT_RET_ERR_INVALID);
        RET_IF_NULL(parser, OPT_RET_ERR_INVALID);
        RET_IF_NULL(parser->arguments, OPT_RET_ERR_INVALID);

        while (arg_ptr != NULL && *arg_ptr != NULL) {
                inc = 1;
                curopt = find_flag(*arg_ptr, parser->flags);
                if (curopt == NULL) {
                        if (is_longopt(*arg_ptr, false) ||
                            is_shortopt(*arg_ptr)) {
                                return OPT_RET_ERR_SEMANTIC;
                        }
                        opt_arg_add(parser->arguments, *arg_ptr);
                }

                if (curopt->arg_parser == NULL) {
                        return OPT_RET_ERR_NOPARSER;
                }

                if (curopt->flag_opts & OPT_FLAG_HAS_ARG) {
                        ret = curopt->arg_parser(curopt, GET_IDX(arg_ptr, 1));
                        inc++;
                        goto parse_opt_pos_ret;
                }

                ret = curopt->arg_parser(curopt, NULL);
parse_opt_pos_ret:
                if (ret != OPT_RET_SUCCESS) {
                        return ret;
                }

                i += inc;
                parser->opt_index += inc;
                arg_ptr = GET_IDX(arg_ptr, inc);
        }

        return OPT_RET_SUCCESS;
}

struct opt_parser *opt_init()
{
        struct opt_parser *ret = NULL;

        ret = malloc(sizeof(struct opt_parser));
        if (ret == NULL) {
                return ret;
        }

        bzero(ret, sizeof(struct opt_parser));
        ret->arguments = malloc(sizeof(struct opt_arg_list));
        if (ret->arguments == NULL) {
                opt_free(ret);
                return NULL;
        }

        bzero(ret->arguments, sizeof(struct opt_arg_list));
        return ret;
}

void opt_free(struct opt_parameters *opt)
{
        struct opt_argument *next = NULL;

        if (opt == NULL) {
                return;
        }

        if (opt->arguments != NULL) {
                struct opt_argument *current = opt->arguments->first;
                while (current != NULL) {
                        next = current->next;
                        free(current);
                        current = next;
                        opt->arguments->count--;
                }
                free(opt->arguments);
        }

        free(opt);
}

int opt_arg_add(struct opt_arg_list *arg_list, const char *arg)
{
        struct opt_argument *new_arg = NULL;

        RET_IF_NULL(arg_list, OPT_RET_ERR_INVALID);
        RET_IF_NULL(arg, OPT_RET_ERR_INVALID);

        new_arg = malloc(sizeof(struct opt_argument));
        if (new_arg == NULL) {
                return OPT_RET_ERR_NOMEM;
        }

        new_arg->arg_str = arg;
        new_arg->next = NULL;

        if (arg_list->count == 0) {
                arg_list->first = new_arg;
                arg_list->last = new_arg;
                return OPT_RET_SUCCESS;
        }

        assert(arg_list->first != NULL && arg_list->last != NULL);

        arg_list->last->next = new_arg;
        arg_list->count++;

        return OPT_RET_SUCCESS:
}