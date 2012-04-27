
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

/**
 * Creates a token
 *
 * handle     - Valid varnam instance
 * pattern    - Lookup pattern
 * value1     - Primary replacement
 * value2     - Alternative replacement (Optional)
 * token_type - One among VARNAM_TOKEN_XXX
 * match_type - Either VARNAM_MATCH_EXACT or VARNAM_MATCH_POSSIBILITY
 * buffered   - Setting TRUE will enable buffering. If set to TRUE, 
                varnam_flush() has to be called to flush buffers.
 *
 * NOTES
 *
 * Turning on buffering improves performance as it delays disk writes. This is 
 * helpful when creating large number of tokens in a tight loop.
 *
 *
 * RETURN
 *
 * VARNAM_SUCCESS        - On successful execution.
 * VARNAM_ARGS_ERROR     - If any of the required arguments are empty, exceeds length or invalid
 * VARNAM_STORAGE_ERROR  - Any error related to writing to disk.
 * VARNAM_ERROR          - Other errors
 **/
VARNAM_EXPORT extern int varnam_create_token(
    varnam *handle,
    const char *pattern,
    const char *value1,
    const char *value2,
    const char *token_type,
    int match_type,
    int buffered
);

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
 * handle   - A valid varnam instance
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

/**
 * Writes changes in the buffer to the disk. Calling this function when no buffered data is available will be a no-op
 *
 * handle - A valid varnam instance
 *
 * NOTES
 *
 * Usually this is called after completing a buffered operation like, varnam_create_token().
 *
 * RETURN
 *
 * VARNAM_SUCCESS       - On successfull execution
 * VARNAM_ARGS_ERROR    - When handle is invalid
 * VARNAM_STORAGE_ERROR - Errors related to writing to disk
 **/
VARNAM_EXPORT extern int varnam_flush_buffer(
    varnam *handle
);

VARNAM_EXPORT extern int 
varnam_destroy(varnam *handle);

#endif
