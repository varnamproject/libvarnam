/*
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#ifndef VARNAM_API_H_INCLUDED_093510
#define VARNAM_API_H_INCLUDED_093510

#include <stdlib.h>
#include "vtypes.h"
#include "util.h"
#include "varray.h"

extern strbuf *varnam_suggestions_dir;
extern strbuf *varnam_symbols_dir;

/**
 * Sets the symbols directory where varnam will look for symbol files while initializing using varnam_init_from_id
 * This is useful if you keep the symbol files in a non-standard location
 *
 **/
VARNAM_EXPORT void
varnam_set_symbols_dir (const char *dir);

/**
 * Sets the suggestions directory where all the learning data will be stored. This value will be used
 * when initializing using varnam_init_from_id
 *
 **/
VARNAM_EXPORT void
varnam_set_suggestions_dir (const char *dir);

/**
 * Initializes the varnam library.
 *
 * scheme_file    - Full path to the varnam scheme file
 * handle         - If successfull, a valid instance of varnam will
 point here. This needs to passed to all the functions in
 varnam library
 * msg            - If any error happens, this will contain the error message.
 User has to free this
 *
 * RETURN
 *
 * VARNAM_SUCCESS       - Successfully initialized
 * VARNAM_ARGS_ERROR    - When mandatory arguements are NULL
 * VARNAM_MEMORY_ERROR  - No sufficient memory to initialize
 * VARNAM_STORAGE_ERROR - Errors related to underlying file
 *
 **/
VARNAM_EXPORT extern int
varnam_init(const char *scheme_file, varnam **handle, char **msg);


VARNAM_EXPORT extern varray*
varnam_get_all_handles();

/**
 * Gets the scheme file currently used
 **/
VARNAM_EXPORT extern const char*
varnam_get_scheme_file (varnam *handle);

/**
 * Gets the suggestions file
 **/
VARNAM_EXPORT extern const char*
varnam_get_suggestions_file (varnam *handle);

/**
 * Initializes the varnam library from the scheme identifier
 *
 * This searches for the symbols file in the following locations
 *
 * /usr/local/share/varnam/vst, /usr/share/varnam/vst, schemes/
 *
 * Suggestions file will be searched in the following locations
 *
 * XDG_DATA_HOME/varnam/suggestions, HOME/.local/share/varnam/suggestions
 *
 * schemeIdentifier       - Unique identifier for the scheme file
 * handle                 - Varnam handle to be initialized.
 * errorMessage           - Set when any error happens
 *
 * RETURN
 *
 * VARNAM_SUCCESS       - Successfully initialized
 * VARNAM_ARGS_ERROR    - When mandatory arguements are NULL
 * VARNAM_MEMORY_ERROR  - No sufficient memory to initialize
 * VARNAM_STORAGE_ERROR - Errors related to underlying file
 * */
VARNAM_EXPORT extern int
varnam_init_from_id(const char *schemeIdentifier, varnam **handle, char **errorMessage);

VARNAM_EXPORT extern const char*
varnam_version();

VARNAM_EXPORT extern int
varnam_register_renderer(
    varnam *handle,
    const char *scheme_id,
    int (*tl)(varnam *handle, vtoken *previous, vtoken *current,  strbuf *output),
    int (*rtl)(varnam *handle, vtoken *previous, vtoken *current,  strbuf *output)
);

/**
 * Configure varnam library.
 *
 * handle - Valid varnam instance
 * type   - One of VARNAM_CONFIG_XXX
 * args   - Depends on the specified VARNAM_CONFIG_XXX
 *
 * NOTES
 *
 * This function won't persist configuration options permanantly. It will reset
 * back to the default configuration when varnam_init() is called next time.
 *
 * Following configuration options are available.
 *
 * VARNAM_CONFIG_USE_DEAD_CONSONANTS
 *   This will make varnam_create_token() to infer dead consonants and persist that rather
 *   than storing consonants. This option is set by default when varnam initializes.
 *   Eg : varnam_config(handle, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 0) - Turns this option off
 *        varnam_config(handle, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 1) - Turns this option on
 *
 * VARNAM_CONFIG_USE_INDIC_DIGITS
 *   This option controls how numbers are rendered in varnam_transliterate().
 *   by default, this option is set to false.
 *   Eg : varnam_config(handle, VARNAM_CONFIG_USE_INDIC_DIGITS, 0) - Turns this option off
 *        varnam_config(handle, VARNAM_CONFIG_USE_INDIC_DIGITS, 1) - Turns this option on
 *
 * VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN
 *   This will make varnam_create_token() to ignore duplicate tokens. Varnam will issue a warning
 *   log message when duplicates are detected.
 *
 * VARNAM_CONFIG_ENABLE_SUGGESTIONS
 *   If this option is set, varnam_transliterate() will return possible suggestions from the words
 *   known to varnam. This configuration option takes a file which contains all know words
 *   Eg: varnam_config(handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, "/home/user/.words") - Use words from the file
 *       varnam_config(handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, NULL) - Turn off suggestions
 *
 * RETURN
 *
 * VARNAM_SUCCESS         - Successfull operation
 * VARNAM_ARGS_ERROR      - Invalid handle
 * VARNAM_INVALID_CONFIG  - Invalid configuration option
 * VARNAM_ERROR           - All other errors
 **/
VARNAM_EXPORT extern int
varnam_config(varnam *handle, int type, ...);

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
    const char *value3,
    const char *tag,
    int token_type,
    int match_type,
    int priority,
    int accept_condition,
    int buffered
    );

VARNAM_EXPORT extern int
varnam_transliterate(varnam *handle, const char *input, varray **output);

VARNAM_EXPORT extern int
varnam_reverse_transliterate(varnam *handle, const char *input, char **result);

/**
 * Varnam will learn the supplied word. It will also learn all possible ways to write
 * the supplied word.
 *
 * handle            - Valid varnam instance
 * word              - word to learn
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - Upon successful execution
 * VARNAM_ARGS_ERROR - Handle or word is NULL
 * VARNAM_ERROR      - Any other errors
 **/
VARNAM_EXPORT extern int
varnam_learn(varnam *handle, const char *word);

/**
 * Train varnam to associate pattern to the word
 *
 * handle            - Valid varnam instance
 * pattern           - Pattern
 * word              - Word to learn
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - Upon successful execution
 * VARNAM_ARGS_ERROR - Handle or word is NULL
 * VARNAM_ERROR      - Any other errors
 **/
VARNAM_EXPORT extern int
varnam_train(varnam *handle, const char *pattern, const char *word);

/**
 * Delete the supplied word from varnam's known words list
 *
 * handle            - Valid varnam instance
 * word              - Word to be deleted
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - After successful deletion
 * VARNAM_ARGS_ERROR - Handle or word is NULL
 * VARNAM_ERROR      - Any other error
 **/
VARNAM_EXPORT extern int
varnam_delete_word(varnam *handle, const char *word);

/**
 * Varnam will learn words from the supplied file.
 * Each word can optionaly take a confidence. Word and confidence should be separated with a space
 *
 * handle            - Valid varnam instance
 * filepath          - file to read from
 * status            - Instance of vlearn_status struct. Varnam will write status of learning to this.
 * callback          - Function which will be invoked for each word
 * object            - Any user data which will be passed in while calling callback
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - Upon successful execution
 * VARNAM_ARGS_ERROR - Handle or word is NULL
 * VARNAM_ERROR      - Any other errors
 **/
VARNAM_EXPORT extern int
varnam_learn_from_file(varnam *handle,
                       const char *filepath,
                       vlearn_status *status,
                       void (*callback)(varnam *handle, const char *word, int status, void *object),
                       void *object);

/**
 * Learnings file will be compacted/ Mostly this will reduce the file size
 *
 * handle - Valid varnam instance
 *
 * RETURN
 *
 * VARNAM_SUCCESS - Upon successful execution
 * VARNAM_ERROR   - Any errors occured during compact
 *
 * */
VARNAM_EXPORT extern int
varnam_compact_learnings_file(varnam *handle);

/**
 * Set scheme details. This will overwrite any scheme details set before
 *
 * handle            - Valid varnam instance
 * language_code     - ISO 639-1 standard two letter language code
 * identifier        - Unique identifier used to identify this scheme
 * display_name      - Friendly name for the scheme
 * author            - Author
 * compiled_date     - Date on which the compilation happened.
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - Upon successful execution
 * VARNAM_ARGS_ERROR - If any arguements are not supplied.
 * VARNAM_ERROR      - Any other error
 *
 **/
VARNAM_EXPORT extern int
varnam_set_scheme_details(
    varnam *handle,
		vscheme_details *scheme_details
    );

/**
 * Gets the metadata for the current scheme
 *
 **/
VARNAM_EXPORT extern int
varnam_get_scheme_details(varnam *handle, vscheme_details **details);

/**
 * Gets the metadata about the current word corpus
 *
 **/
VARNAM_EXPORT extern int
varnam_get_corpus_details(varnam *handle, vcorpus_details **details);

/**
 * Returns error message for the most recent failed call to varnam API functions.
 *
 **/
VARNAM_EXPORT extern const char*
varnam_get_last_error(varnam *handle);

/**
 * Retrieves all tokens matching the supplied token type
 *
 * handle     - Valid varnam instance
 * token_type - One among VARNAM_TOKEN_XXX
 * tokens     - Output will be written here. This will be an array of tokens.
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - On successful execution
 * VARNAM_ARGS_ERROR - Invalid handle or incorrect token type
 * VARNAM_ERROR      - Any other errors
 * */
VARNAM_EXPORT extern int
varnam_get_all_tokens(
    varnam *handle,
    int token_type,
    varray **tokens
    );

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
 * varnam_enable_logging (handle, VARNAM_LOG_DEBUG, func)    - Enables debug level logging
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

VARNAM_EXPORT extern int varnam_get_info(
    varnam *handle,
    bool detailed,
    vinfo **info
    );

/**
 * Exports words and patterns to text file(s). This may produce multiple text files depending on the number of words
 *
 * handle         - A valid varnam handle
 * words_per_file - Number of words to be written to one file
 * out_dir        - Directory path without trailing '/' where files will be written
 * export_type    - VARNAM_EXPORT_FULL or VARNAM_EXPORT_WORDS
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - On successful execution
 * VARNAM_ARGS_ERROR - Invalid arguments
 * VARNAM_ERROR      - Any other error
 *
 * **/
VARNAM_EXPORT extern int varnam_export_words(
    varnam *handle,
    int words_per_file,
    const char *out_dir,
    int export_type,
    void (*callback)(int , int , const char *)
    );

/**
 * Import learned data from the specified file.
 * Mostly the files exported using varnam_export_words will be given to this function.
 *
 * handle    - A valid varnam instance
 * filepath  - Full path to the file
 *
 * RETURN
 *
 * VARNAM_SUCCESS    - Upon successful import
 * VARNAM_ARGS_ERROR - When incorrect arguments are specified
 * VARNAM_ERROR      - Any other errors
 **/
int
varnam_import_learnings_from_file(varnam *handle, const char *filepath);

/**
 * Detects the language for the supplied word. Current implementation works only for devanagari based scripts.
 *
 * handle - A valid varnam instance
 * word   - Word in some language
 *
 * RETURN
 *
 * VARNAM_LANG_CODE_XX      - Language code upon successful execution
 * VARNAM_ERROR             - All other errors
 **/
VARNAM_EXPORT extern int varnam_detect_lang(
    varnam *handle,
    const char *word);

/**
 * Checks the specified word is know to varnam
 *
 * handle - A valid varnam instance
 * word   - Word to check for
 *
 * RETURN
 *
 * A boolean value indicating the whether the word is known to varnam.
 **/
VARNAM_EXPORT extern int varnam_is_known_word(
        varnam *handle,
        const char *word);

/*Creates a stemrule in the varnam symbol table*/
VARNAM_EXPORT extern int 
varnam_create_stemrule(varnam* handle, const char* old_ending, const char* new_ending);

VARNAM_EXPORT extern int
varnam_create_stem_exception(varnam *handle, const char *rule, const char *exception);

VARNAM_EXPORT extern void
varnam_destroy(varnam *handle);

#endif
