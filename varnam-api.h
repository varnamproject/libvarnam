
/* varnam.h
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA */


#ifndef VARNAM_API_H_INCLUDED_093510
#define VARNAM_API_H_INCLUDED_093510

#include <stdlib.h>
#include "varnam-types.h"
#include "varnam-util.h"

VARNAM_EXPORT extern int 
varnam_init(const char *symbols_file, varnam **handle, char **msg);

/* VARNAM_EXPORT extern int varnam_create_token(varnam *handle,  */
/*                                              const char *pattern,  */
/*                                              const char *value1, */
/*                                              const char *value2, */
/*                                              int token_type, */
/*                                              int match_type, */
/*                                              int buffered); */

VARNAM_EXPORT extern int 
varnam_transliterate(varnam *handle, const char *input, char **result);

VARNAM_EXPORT extern int 
varnam_reverse_transliterate(varnam *handle, const char *input, char **result);

VARNAM_EXPORT extern const char* 
varnam_scheme_identifier(varnam *handle);

VARNAM_EXPORT extern const char* 
varnam_scheme_display_name(varnam *handle);

VARNAM_EXPORT extern const char* 
varnam_scheme_author(varnam *handle);

VARNAM_EXPORT extern const char* 
varnam_last_error(varnam *handle);

/**
 * Enable logging.
 *
 * handle   - A valid varnam handle instance
 * log_type - Either VARNAM_LOG_DEFAULT or VARNAM_LOG_DEBUG
 * callback - Actual function that does the logging
 *
 * USAGE 
 *
 * varnam_enable_logging (handle, VARNAM_LOG_DEFAULT, func) - Enables default logging
 * varnam_enable_logging (handle, VARNAM_LOG_DBUG, func)    - Enables debug level logging
 * varnam_enable_logging (handle, VARNAM_LOG_DEFAULT, NULL) - Disables logging
 *
 * Logging is hierarchical. So enabling debug level logging will generate all the log messages
 * varnam is writing. Default logging level will not produce debug level messages
 **/
VARNAM_EXPORT extern int varnam_enable_logging(
    varnam *handle,
    int log_type,
    void (*callback)(const char*)
);

VARNAM_EXPORT extern int 
varnam_destroy(varnam *handle);

#endif
