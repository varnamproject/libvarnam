/* 
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include <string.h>
#include <stdarg.h>

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
    }
    return vi;
}

int
varnam_init(const char *scheme_file, varnam **handle, char **msg)
{
    int rc;
    varnam *c;
    struct varnam_internal *vi;
    size_t filename_length;

    if(scheme_file == NULL)
        return VARNAM_ARGS_ERROR;

    c = (varnam *) xmalloc(sizeof (varnam));
    if(!c)
        return VARNAM_MEMORY_ERROR;

    vi = initialize_internal();
    if(!vi)
        return VARNAM_MEMORY_ERROR;

    vi->message = (char *) xmalloc(sizeof (char) * VARNAM_LIB_TEMP_BUFFER_SIZE);
    if(!vi->message)
        return VARNAM_MEMORY_ERROR;

    rc = sqlite3_open(scheme_file, &vi->db);
    if( rc ) {
        asprintf(msg, "Can't open %s: %s\n", scheme_file, sqlite3_errmsg(vi->db));
        sqlite3_close(vi->db);
        return VARNAM_STORAGE_ERROR;
    }

    filename_length = strlen(scheme_file);
    c->scheme_file = (char *) xmalloc(filename_length + 1);
    if(!c->scheme_file)
        return VARNAM_MEMORY_ERROR;

    strncpy(c->scheme_file, scheme_file, filename_length + 1);
    c->internal = vi;

    rc = ensure_schema_exists(c, msg);
    if (rc != VARNAM_SUCCESS)
        return rc;

    rc = varnam_register_renderer (c, "ml-unicode", &ml_unicode_renderer, &ml_unicode_rtl_renderer);
    if (rc != VARNAM_SUCCESS)
        return rc;

    *handle = c;
    return VARNAM_SUCCESS;
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
    if (handle == NULL)
        return VARNAM_ARGS_ERROR;

    return vst_flush_changes(handle);
}

static int
enable_suggestions(varnam *handle, const char *file)
{
    int rc;

    if (v_->known_words != NULL) {
        sqlite3_close (v_->known_words);
        v_->known_words = NULL;
    }

    if (file == NULL)
        return VARNAM_SUCCESS;

    rc = sqlite3_open(file, &v_->known_words);
    if( rc )
    {
        set_last_error (handle, "Can't open %s: %s\n", file, sqlite3_errmsg(v_->known_words));
        sqlite3_close (v_->known_words);
        v_->known_words = NULL;
        return VARNAM_ERROR;
    }

    varnam_debug (handle, "%s will be used to store known words", file);

    return vwt_ensure_schema_exists (handle);
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

void
varnam_destroy(varnam *handle)
{
    struct varnam_internal *vi;

    if (handle == NULL)
        return;

    vi = handle->internal;

    destroy_all_statements (handle);

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

    sqlite3_close(handle->internal->db);
    if (handle->internal->known_words != NULL)
        sqlite3_close(handle->internal->known_words);

    xfree(handle->internal);
    xfree(handle->scheme_file);
    xfree(handle);
}
