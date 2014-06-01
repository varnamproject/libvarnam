/*
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "varray.h"
#include "api.h"
#include "vtypes.h"
#include "util.h"
#include "result-codes.h"
#include "symbol-table.h"
#include "words-table.h"
#include "token.h"
#include "vword.h"
#include "renderer/renderers.h"

strbuf *varnam_suggestions_dir = NULL;
strbuf *varnam_symbols_dir = NULL;

void
varnam_set_symbols_dir (const char *dir)
{
    if (varnam_symbols_dir == NULL) {
        varnam_symbols_dir = strbuf_init (20);
    }
    strbuf_clear (varnam_symbols_dir);
    strbuf_add (varnam_symbols_dir, dir);
}

void
varnam_set_suggestions_dir (const char *dir)
{
    if (varnam_suggestions_dir == NULL) {
        varnam_suggestions_dir = strbuf_init (20);
    }
    strbuf_clear (varnam_suggestions_dir);
    strbuf_add (varnam_suggestions_dir, dir);
}

static void
destroy_varnam_internal(struct varnam_internal* vi);

static struct varnam_internal*
initialize_internal()
{
    struct varnam_internal *vi;
    vi = (struct varnam_internal *) xmalloc(sizeof (struct varnam_internal));
    if(vi) {
        vi->virama = NULL;
        vi->renderers = NULL;
        vi->last_error = strbuf_init(100);
        vi->log_level = VARNAM_LOG_DEFAULT;
        vi->log_callback = NULL;
        vi->log_message = strbuf_init(100);
        vi->vst_buffering = 0;
        vi->lastLearnedWord = NULL;
        vi->lastLearnedWordId = 0;

        /* scheme details buffers */
        vi->scheme_language_code = strbuf_init(2);
        vi->scheme_identifier = strbuf_init(10);
        vi->scheme_display_name = strbuf_init(10);
        vi->scheme_author = strbuf_init(25);
        vi->scheme_compiled_date = strbuf_init(10);

        /* configuration options */
        vi->config_use_dead_consonants = 0;
        vi->config_ignore_duplicate_tokens = 1;
        vi->config_use_indic_digits = 0;
        vi->_config_mostly_learning_new_words = 0;

        /* suggestions */
        vi->known_words = NULL;

        /* instance pool */
        vi->tokens_pool = NULL;
        vi->arrays_pool = NULL;
        vi->words_pool = NULL;

        vi->strings_pool = NULL;

        /* Result of tokenization will be stored inside */
        vi->tokens = varray_init();

        /* Prepared statements */
        vi->tokenize_using_pattern = NULL;
        vi->tokenize_using_value = NULL;
        vi->tokenize_using_value_and_match_type = NULL;
        vi->can_find_more_matches_using_pattern = NULL;
        vi->can_find_more_matches_using_value = NULL;
        vi->learn_word = NULL;
        vi->learn_pattern = NULL;
        vi->get_word = NULL;
        vi->get_suggestions = NULL;
        vi->get_best_match = NULL;
        vi->get_matches_for_word = NULL;
        vi->possible_to_find_matches = NULL;
        vi->update_confidence = NULL;
        vi->update_learned_flag = NULL;
        vi->delete_pattern = NULL;
        vi->delete_word = NULL;
        vi->export_words = NULL;
        vi->learned_words_count = NULL;

        vi->tokens_cache = NULL;
        vi->noMatchesCache = NULL;
        vi->tokenizationPossibility = NULL;
    }
    return vi;
}

int
varnam_init(const char *scheme_file, varnam **handle, char **msg)
{
    int rc;
    varnam *c = NULL;
    struct varnam_internal *vi;
    size_t filename_length;

    *handle = NULL;
    *msg = NULL;

    if(scheme_file == NULL)
        return VARNAM_ARGS_ERROR;

    c = (varnam *) xmalloc(sizeof (varnam));
    if(!c)
        return VARNAM_MEMORY_ERROR;

    c->scheme_file = NULL;
    c->suggestions_file = NULL;
    c->internal = NULL;

    vi = initialize_internal();
    if(!vi)
        return VARNAM_MEMORY_ERROR;

    vi->message = (char *) xmalloc(sizeof (char) * VARNAM_LIB_TEMP_BUFFER_SIZE);
    if(!vi->message)
        return VARNAM_MEMORY_ERROR;

    filename_length = strlen(scheme_file);
    c->scheme_file = (char *) xmalloc(filename_length + 1);
    if(!c->scheme_file)
        return VARNAM_MEMORY_ERROR;

    strncpy(c->scheme_file, scheme_file, filename_length + 1);
    c->internal = vi;

    rc = sqlite3_open(scheme_file, &vi->db);
    if( rc ) {
        asprintf(msg, "Can't open %s: %s\n", scheme_file, sqlite3_errmsg(vi->db));
        varnam_destroy (c);
        return VARNAM_STORAGE_ERROR;
    }

    rc = ensure_schema_exists(c, msg);
    if (rc != VARNAM_SUCCESS) {
        varnam_destroy (c);
        return rc;
    }

    rc = varnam_register_renderer (c, "ml-unicode", &ml_unicode_renderer, &ml_unicode_rtl_renderer);
    if (rc != VARNAM_SUCCESS) {
        varnam_destroy (c);
        return rc;
    }

    *handle = c;
    return VARNAM_SUCCESS;
}

const char*
varnam_get_scheme_file (varnam *handle)
{
  return handle->scheme_file;
}

const char*
varnam_get_suggestions_file (varnam *handle)
{
  return handle->suggestions_file;
}

static strbuf*
find_symbols_file_path (const char *langCode)
{
  int i;
  strbuf *path;
  const char* symbolsFileSearchPath[] = {
    "/usr/local/share/varnam/vst",
    "/usr/share/varnam/vst",
    "schemes"
  };

  path = strbuf_init (50);
  if (varnam_symbols_dir != NULL) {
    strbuf_addf (path, "%s/%s.vst", strbuf_to_s (varnam_symbols_dir), langCode);
    if (!is_path_exists (strbuf_to_s (path)))
        return NULL;

    return path;
  }

  for (i = 0; i < ARRAY_SIZE (symbolsFileSearchPath); i++) {
    strbuf_addf (path, "%s/%s.vst", symbolsFileSearchPath[i], langCode);
    if (is_path_exists (strbuf_to_s (path)))
      return path;

    strbuf_clear (path);
  }

  return NULL;
}

static bool
make_directory (const char *dirName)
{
  int rc;
  strbuf *command;

  command = strbuf_init (20);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
  strbuf_addf (command, "mkdir %s", dirName);
#else
  strbuf_addf (command, "mkdir -p %s", dirName);
#endif
  rc = system (strbuf_to_s (command));
  strbuf_destroy (command);
  if (rc == 0)
    return true;
  else
    return false;
}

static strbuf*
find_learnings_file_path (const char *langCode)
{
  char *tmp;
  strbuf *path;

  path = strbuf_init (20);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
  tmp = getenv ("APPDATA");
  if (tmp != NULL) {
    strbuf_addf (path, "%s\\varnam\\suggestions\\", tmp);
    if (!is_directory (strbuf_to_s (path))) {
      strbuf_clear (path);
    }
  }
#else
  tmp = getenv ("XDG_DATA_HOME");
  if (tmp == NULL) {
    tmp = getenv ("HOME");
    if (tmp != NULL) {
      strbuf_addf (path, "%s/.local/share/varnam/suggestions/", tmp);
    }
  }
  else {
    strbuf_addf (path, "%s/varnam/suggestions/", tmp);
  }

  if (!strbuf_is_blank (path)) {
    if (is_path_exists (strbuf_to_s (path))) {
        if (!is_directory (strbuf_to_s (path))) {
            /* Suggestions will be configured in the current directory */
           strbuf_clear (path);
        }
    }
    else {
        if (!make_directory (strbuf_to_s (path))) {
            /* Suggestions will be configured in the current directory */
            strbuf_clear (path);
        }
    }
  }
#endif
  
  strbuf_addf (path, "%s.vst.learnings", langCode);
  return path;
}

int
varnam_init_from_lang(const char *langCode, varnam **handle, char **errorMessage)
{
  strbuf *symbolsFilePath, *learningsFilePath, *error;
  int rc;

  if (langCode == NULL)
    return VARNAM_ARGS_ERROR;

  *handle = NULL;
  *errorMessage = NULL;

  symbolsFilePath = find_symbols_file_path (langCode);
  if (symbolsFilePath == NULL) {
    error = strbuf_init (20);
    strbuf_addf (error, "Failed to find symbols file for: %s", langCode);
    *errorMessage = strbuf_detach (error);
    return VARNAM_ERROR;
  }

  if (varnam_suggestions_dir != NULL) {
      learningsFilePath = strbuf_init (20);
      strbuf_add (learningsFilePath, strbuf_to_s (varnam_suggestions_dir));
  }
  else {
      learningsFilePath = find_learnings_file_path (langCode);
  }

  rc = varnam_init (strbuf_to_s (symbolsFilePath), handle, errorMessage);
  if (rc == VARNAM_SUCCESS) {
    rc = varnam_config (*handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, strbuf_to_s (learningsFilePath));
    if (rc != VARNAM_SUCCESS) {
        error = strbuf_init (20);
        strbuf_add (error, varnam_get_last_error (*handle));
        *errorMessage = strbuf_detach (error);
        varnam_destroy (*handle);
        *handle = NULL;
    }
  }

  strbuf_destroy (symbolsFilePath);
  strbuf_destroy (learningsFilePath);

  return rc;
}

const char*
varnam_version()
{
    return VARNAM_VERSION;
}

int
varnam_register_renderer(
    varnam *handle,
    const char *scheme_id,
    int (*tl)(varnam *handle, vtoken *previous, vtoken *current,  strbuf *output),
    int (*rtl)(varnam *handle, vtoken *previous, vtoken *current,  strbuf *output))
{
    vtoken_renderer *r;

    if (handle == NULL)
        return VARNAM_ARGS_ERROR;

    if (v_->renderers == NULL)
        v_->renderers = varray_init();

    r = xmalloc (sizeof (vtoken_renderer));
    if (r == NULL)
        return VARNAM_ERROR;

    r->scheme_id = scheme_id;
    r->tl = tl;
    r->rtl = rtl;

    varray_push (v_->renderers, r);
    return VARNAM_SUCCESS;
}

int
varnam_set_scheme_details(
    varnam *handle,
    const char *language_code,
    const char *identifier,
    const char *display_name,
    const char *author,
    const char *compiled_date)
{
    int rc;

    set_last_error (handle, NULL);

    if (language_code != NULL && strlen(language_code) > 0)
    {
        if (strlen(language_code) != 2)
        {
            set_last_error (handle, "Language code should be one of ISO 639-1 two letter codes.");
            return VARNAM_ERROR;
        }

        rc = vst_add_metadata (handle, VARNAM_METADATA_SCHEME_LANGUAGE_CODE, language_code);
        if (rc != VARNAM_SUCCESS)
            return rc;

        varnam_log (handle, "Set language code to : %s", language_code);
    }

    if (identifier != NULL && strlen(identifier) > 0)
    {
        rc = vst_add_metadata (handle, VARNAM_METADATA_SCHEME_IDENTIFIER, identifier);
        if (rc != VARNAM_SUCCESS)
            return rc;

        varnam_log (handle, "Set language identifier to : %s", identifier);
    }

    if (display_name != NULL && strlen(display_name) > 0)
    {
        rc = vst_add_metadata (handle, VARNAM_METADATA_SCHEME_DISPLAY_NAME, display_name);
        if (rc != VARNAM_SUCCESS)
            return rc;

        varnam_log (handle, "Set language display name to : %s", display_name);
    }

    if (author != NULL && strlen(author) > 0)
    {
        rc = vst_add_metadata (handle, VARNAM_METADATA_SCHEME_AUTHOR, author);
        if (rc != VARNAM_SUCCESS)
            return rc;

        varnam_log (handle, "Set author to : %s", author);
    }

    if (compiled_date != NULL && strlen(compiled_date) > 0)
    {
        rc = vst_add_metadata (handle, VARNAM_METADATA_SCHEME_COMPILED_DATE, compiled_date);
        if (rc != VARNAM_SUCCESS)
            return rc;

        varnam_log (handle, "Set compiled date to : %s", compiled_date);
    }

    return VARNAM_SUCCESS;
}

static const char*
get_scheme_details(varnam *handle, const char* key, struct strbuf *buffer)
{
    struct strbuf *value = buffer;

    if (handle == NULL)
        return NULL;

    if (strbuf_is_blank (value))
    {
        vst_get_metadata (handle, key, value);
    }

    return strbuf_to_s (value);
}

const char*
varnam_get_scheme_language_code(varnam *handle)
{
    return get_scheme_details (handle,
                               VARNAM_METADATA_SCHEME_LANGUAGE_CODE,
                               handle->internal->scheme_language_code);
}

const char*
varnam_get_scheme_identifier(varnam *handle)
{
    return get_scheme_details (handle,
                               VARNAM_METADATA_SCHEME_IDENTIFIER,
                               handle->internal->scheme_identifier);
}

const char*
varnam_get_scheme_display_name(varnam *handle)
{
    return get_scheme_details (handle,
                               VARNAM_METADATA_SCHEME_DISPLAY_NAME,
                               handle->internal->scheme_display_name);
}

const char*
varnam_get_scheme_author(varnam *handle)
{
    return get_scheme_details (handle,
                               VARNAM_METADATA_SCHEME_AUTHOR,
                               handle->internal->scheme_author);
}

const char*
varnam_get_scheme_compiled_date(varnam *handle)
{
    return get_scheme_details (handle,
                               VARNAM_METADATA_SCHEME_COMPILED_DATE,
                               handle->internal->scheme_compiled_date);
}

const char*
varnam_get_last_error(varnam *handle)
{
    if (handle == NULL)
        return NULL;

    return handle->internal->last_error->buffer;
}

int
varnam_enable_logging(varnam *handle, int log_type, void (*callback)(const char*))
{
    if (handle == NULL)
        return VARNAM_ARGS_ERROR;

    if (log_type != VARNAM_LOG_DEFAULT && log_type != VARNAM_LOG_DEBUG)
    {
        set_last_error (handle, "Incorrect log type. Valid values are VARNAM_LOG_DEFAULT or VARNAM_LOG_DEBUG");
        return VARNAM_ERROR;
    }

    handle->internal->log_level = log_type;
    handle->internal->log_callback = callback;

    return VARNAM_SUCCESS;
}

/* Checks if the string has inherent 'a' sound. If yes, we can infer dead consonant from it */
static int can_generate_dead_consonant(const char *string, size_t len)
{
    if(len <= 1)
        return 0;
    return string[len - 2] != 'a' && string[len - 1] == 'a';
}

int
varnam_create_token(
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
    int buffered)
{
    int rc;
    size_t pattern_len;
    char p[VARNAM_SYMBOL_MAX], v1[VARNAM_SYMBOL_MAX], v2[VARNAM_SYMBOL_MAX];
    struct token *virama;

    set_last_error (handle, NULL);

    if (handle == NULL || pattern == NULL || value1 == NULL)
        return VARNAM_ARGS_ERROR;

    if (strlen(pattern) > VARNAM_SYMBOL_MAX ||
        strlen(value1) > VARNAM_SYMBOL_MAX  ||
        (value2 != NULL && strlen(value2) > VARNAM_SYMBOL_MAX) ||
        (value3 != NULL && strlen(value3) > VARNAM_SYMBOL_MAX) ||
        (tag != NULL && strlen(tag) > VARNAM_SYMBOL_MAX))
    {
        set_last_error (handle, "Length of pattern, tag, value1 or value2, value3 should be less than VARNAM_SYMBOL_MAX");
        return VARNAM_ARGS_ERROR;
    }

    if (match_type != VARNAM_MATCH_EXACT && match_type != VARNAM_MATCH_POSSIBILITY)
    {
        set_last_error (handle, "match_type should be either VARNAM_MATCH_EXACT or VARNAM_MATCH_POSSIBILITY");
        return VARNAM_ARGS_ERROR;
    }

    if (accept_condition != VARNAM_TOKEN_ACCEPT_ALL &&
            accept_condition != VARNAM_TOKEN_ACCEPT_IF_STARTS_WITH &&
            accept_condition != VARNAM_TOKEN_ACCEPT_IF_IN_BETWEEN &&
            accept_condition != VARNAM_TOKEN_ACCEPT_IF_ENDS_WITH) {
        set_last_error (handle, "Invalid accept condition specified. It should be one of VARNAM_TOKEN_ACCEPT_XXX");
        return VARNAM_ARGS_ERROR;
    }

    if (buffered)
    {
        rc = vst_start_buffering (handle);
        if (rc != VARNAM_SUCCESS)
            return rc;
    }

    pattern_len = strlen(pattern);

    if (token_type == VARNAM_TOKEN_CONSONANT &&
        handle->internal->config_use_dead_consonants)
    {
        rc = vst_get_virama(handle, &virama);
        if (rc != VARNAM_SUCCESS)
            return rc;
        else if (virama == NULL)
        {
            set_last_error (handle, "Virama needs to be set before auto generating dead consonants");
            return VARNAM_ERROR;
        }

        if (utf8_ends_with(value1, virama->value1))
        {
            token_type = VARNAM_TOKEN_DEAD_CONSONANT;
        }
        else if (can_generate_dead_consonant(pattern, pattern_len))
        {
            substr(p, pattern, 1, (int) (pattern_len - 1));
            portable_snprintf(v1, VARNAM_SYMBOL_MAX, "%s%s", value1, virama->value1);

            if (value2 != NULL)
                portable_snprintf(v2, VARNAM_SYMBOL_MAX, "%s%s", value2, virama->value1);
            else
                v2[0] = '\0';

            rc = vst_persist_token (handle, p, v1, v2, value3, tag, VARNAM_TOKEN_DEAD_CONSONANT, match_type, priority, accept_condition);
            if (rc != VARNAM_SUCCESS)
            {
                if (buffered) vst_discard_changes(handle);
                return rc;
            }
        }
    }

    if (token_type == VARNAM_TOKEN_NON_JOINER)
        value1 = value2 = ZWNJ();

    if (token_type == VARNAM_TOKEN_JOINER)
        value1 = value2 = ZWJ();

    rc = vst_persist_token (handle, pattern, value1, value2, value3, tag, token_type, match_type, priority, accept_condition);
    if (rc != VARNAM_SUCCESS)
    {
        if (buffered) vst_discard_changes(handle);
        return rc;
    }

    if (!buffered) {
        rc = vst_make_prefix_tree (handle);
        if (rc != VARNAM_SUCCESS) return rc;

        rc = vst_stamp_version (handle);
        if (rc != VARNAM_SUCCESS) return rc;
    }

    return rc;
}

int
varnam_get_all_tokens(
    varnam *handle,
    int token_type,
    varray **tokens)
{
    if (handle == NULL)
        return VARNAM_ARGS_ERROR;

    *tokens = v_->tokens;
    return vst_get_all_tokens (handle, token_type, v_->tokens);
}

int
varnam_flush_buffer(varnam *handle)
{
    int rc;

    if (handle == NULL)
        return VARNAM_ARGS_ERROR;

    rc = vst_make_prefix_tree (handle);
    if (rc != VARNAM_SUCCESS)
        return rc;

    rc = vst_stamp_version (handle);
    if (rc != VARNAM_SUCCESS)
        return rc;

    return vst_flush_changes(handle);
}

static int
enable_suggestions(varnam *handle, const char *file)
{
    int rc;
    strbuf *tmp;

    if (v_->known_words != NULL) {
        sqlite3_close (v_->known_words);
        v_->known_words = NULL;
    }

    if(is_file_exists(file) == 1)
    {
        printf("\nFile invalidated\n");
        set_last_error(handle,"can't open %s. File does not exist",file);
        return VARNAM_MISUSE;
    }

    if (file == NULL)
        return VARNAM_MISUSE;

    rc = sqlite3_open(file, &v_->known_words);
    if( rc )
    {
        set_last_error (handle, "Can't open %s: %s\n", file, sqlite3_errmsg(v_->known_words));
        v_->known_words = NULL;
        return VARNAM_ERROR;
    }

    rc = vwt_ensure_schema_exists (handle);
    if (rc != VARNAM_SUCCESS) {
      return rc;
    }

    tmp = strbuf_init (20);
    strbuf_add (tmp, file);
    handle->suggestions_file = strbuf_detach (tmp);

    return VARNAM_SUCCESS;
}

int
varnam_config(varnam *handle, int type, ...)
{
    va_list args;
    int rc = VARNAM_SUCCESS;

    if (handle == NULL)
        return VARNAM_ARGS_ERROR;

    set_last_error (handle, NULL);

    va_start (args, type);
    switch (type)
    {
    case VARNAM_CONFIG_USE_DEAD_CONSONANTS:
        v_->config_use_dead_consonants = va_arg(args, int);
        break;
    case VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN:
        v_->config_ignore_duplicate_tokens = va_arg(args, int);
        break;
    case VARNAM_CONFIG_USE_INDIC_DIGITS:
        v_->config_use_indic_digits = va_arg(args, int);
        break;
    case VARNAM_CONFIG_ENABLE_SUGGESTIONS:
        rc = enable_suggestions (handle, va_arg(args, const char*));
        break;
    default:
        set_last_error (handle, "Invalid configuration key");
        rc = VARNAM_INVALID_CONFIG;
    }

    va_end (args);

    return rc;
}

int
varnam_get_info (varnam *handle, bool detailed, vinfo **info)
{
    /* only array details are implemeted */
    vinfo *i = xmalloc (sizeof (vinfo));
    i->scheme_file = handle->scheme_file;
    i->symbols = 0;
    i->words = 0;

    i->tokens_in_memory = varray_length (v_->tokens_pool->array);
    i->arrays_in_memory = varray_length (v_->arrays_pool->array);

    *info = i;

    return VARNAM_SUCCESS;
}

static void
destroy_array(void *a)
{
    varray *array = (varray*) a;
    if (array != NULL)
        varray_free (array, NULL);
}

static void
clear_cache (vcache_entry **cache)
{
    vcache_entry *current, *tmp;
    HASH_ITER(hh, *cache, current, tmp) {
        HASH_DEL(*cache, current);  /* delete; users advances to next */
        xfree (current->key);
        if (current->cb != NULL) {
            current->cb (current->value);
        }
        xfree (current);
    }
}

void
varnam_destroy(varnam *handle)
{
    if (handle == NULL)
        return;

    destroy_varnam_internal (handle->internal);
    xfree(handle->scheme_file);
    xfree(handle->suggestions_file);
    xfree(handle);
    strbuf_destroy (varnam_suggestions_dir);
    strbuf_destroy (varnam_symbols_dir);
}

static void
destroy_varnam_internal(struct varnam_internal* vi)
{
    destroy_all_statements (vi);
    destroy_token (vi->virama);
    vpool_free (vi->tokens_pool, &destroy_token);
    vpool_free (vi->strings_pool, &strbuf_destroy);
    vpool_free (vi->words_pool, &destroy_word);
    vpool_free (vi->arrays_pool, &destroy_array);
    varray_free (vi->tokens, NULL);
    varray_free (vi->renderers, &xfree);
    xfree(vi->message);
    strbuf_destroy (vi->last_error);
    strbuf_destroy (vi->scheme_language_code);
    strbuf_destroy (vi->scheme_identifier);
    strbuf_destroy (vi->scheme_display_name);
    strbuf_destroy (vi->scheme_author);
    strbuf_destroy (vi->scheme_compiled_date);
    strbuf_destroy (vi->log_message);
    strbuf_destroy (vi->lastLearnedWord);
    sqlite3_close(vi->db);
    if (vi->known_words != NULL)
        sqlite3_close(vi->known_words);

    clear_cache (&vi->tokens_cache);
    clear_cache (&vi->noMatchesCache);
    clear_cache (&vi->tokenizationPossibility);
    xfree(vi);
}
