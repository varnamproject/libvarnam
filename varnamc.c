#include "varnam.h"

#include <argp.h>
#include <stdbool.h>

static char doc[] = "an Indic language transliteration library";
static char args_doc[] = "";
static struct argp_option options[] = { 
    {"symbols", 's', "VALUE", 0, "Sets the symbols file"},
    {"transliterate", 't', "TEXT", 0, "Transliterate the given text"},
    {"details", 'd', "", OPTION_ARG_OPTIONAL, "Detailed transliteration output. Use with --transliterate"},
    {"reverse-transliterate", 'r', "TEXT", 0, "Reverse transliterate the given text"},
    {"learn", 'n', "TEXT", 0, "Learn the given text"},
    {"learn-from", 'f', "FILE", 0, "Reads from the specified file"},
    {"train", 'a', "PATTERN=WORD", 0, "Train the given text"},
    {"import-learnings-from", 'i', "FILE", 0, "Import learned data from the specified file"},
    {"export-full", 'e', "FILE", 0, "Export words and patterns to the specified directory"},
    {"version", 'v', "", OPTION_ARG_OPTIONAL, "Display version"},
    {0} 
};

struct arguments {
  char *symbols;
  char *transliterate;
  bool details;
  char *reverse_transliterate;
  char *learn;
  char *learn_from;
  char *train;
  char *import_learnings_from;
  char *export_full;
  bool version;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 's':
    arguments->symbols = arg;
    break;
  case 't':
    arguments->transliterate = arg;
    break;
  case 'd':
    arguments->details = true;
    break;
  case 'r':
    arguments->reverse_transliterate = arg;
    break;
  case 'n':
    arguments->learn = arg;
    break;
  case 'f':
    arguments->learn_from = arg;
    break;
  case 'a':
    arguments->train = arg;
    break;
  case 'i':
    arguments->import_learnings_from = arg;
    break;
  case 'e':
    arguments->export_full = arg;
    break;
  case 'v':
    arguments->version = true;
    break;
  case ARGP_KEY_ARG:
    return 0;
  default:
    ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

/**
 * -----
 * Helper Functions
 * -----
 */

/* Check if text is not a sentence */
void ensure_single_word(char *text)
{
  int i = 0;
  size_t s_length = strlen(text);
  while(i < s_length)
  {
    if(text[i++] == ' ')
    {
      printf("varnamc : Expected a single word.");
      exit(1);
    }
  }
}


/**
 *  splits str on delim and dynamically allocates an array of pointers.
 *
 *  On error -1 is returned, check errno
 *  On success size of array is returned, which may be 0 on an empty string
 *  or 1 if no delim was found.  
 *
 *  You could rewrite this to return the char ** array instead and upon NULL
 *  know it's an allocation problem but I did the triple array here.  Note that
 *  upon the hitting two delim's in a row "foo,,bar" the array would be:
 *  { "foo", NULL, "bar" } 
 * 
 *  You need to define the semantics of a trailing delim Like "foo," is that a
 *  2 count array or an array of one?  I choose the two count with the second entry
 *  set to NULL since it's valueless.
 *  Modifies str so make a copy if this is a problem
 */
int split( char * str, char delim, char ***array )
{
  char *p;
  char **res;
  int count=0;
  int k=0;

  p = str;
  /* Count occurance of delim in string */
  while( (p = strchr(p, delim)) != NULL ) {
    *p = 0; /* Null terminate the deliminator. */
    p++; /* Skip past our new null */
    count++;
  }

  /* allocate dynamic array */
  res = calloc( 1, count * sizeof(char *));
  if( !res ) return -1;

  p = str;
  for(k = 0; k < count + 1; k++){
    if( *p ) res[k] = p; /* Copy start of string */
    p = strchr(p, 0 ); /* Look for next null */
    p++; /* Start of next string */
  }

  *array = res;

  return count + 1;
}

/**
 * -----
 * End Helper Functions
 * -----
 */

void print_transliteration_output(varray *words, bool details)
{
    int i;
    vword *word;
    for (i = 0; i < varray_length(words); i++)
    {
        if (i != 0)
          printf("\n");

        word = varray_get(words, i);
        if (details) {
          printf ("%s. Confidence %d", word->text, word->confidence);
        } else {
          printf ("%s", word->text);
        }
    }
}

/**
 * Transliterate a word
 */
void transliterate(varnam *handle, char *text, bool info)
{
  int rc;
  varray *words;

  ensure_single_word(text);

	rc = varnam_transliterate (handle, text, &words);
	if (rc != VARNAM_SUCCESS)
	{
		printf("Transliteration failed. Reason - %s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		exit(1);
	}
	print_transliteration_output (words, info);
  exit(0);
}

/**
 * Reverse transliterate a word
 */
void reverse_transliterate(varnam *handle, char *text)
{
  int rc;
  char *output;

  ensure_single_word(text);

  rc = varnam_reverse_transliterate (handle, text, &output);
	if (rc != VARNAM_SUCCESS)
	{
		printf("%s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		exit(1);
	}
	printf("%s", output);
  exit(0);
}

/**
 * Learn a word
 */
void learn(varnam *handle, char *word)
{
  int rc;
  ensure_single_word(word);

  rc = varnam_learn (handle, word);
	if (rc != VARNAM_SUCCESS)
	{
		printf("%s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		exit(1);
	}
	printf("Learned %s", word);
  exit(0);
}

int learn_counter = 0;
int learn_passed_counter = 0;
int learn_failed_counter = 0;
void learn_callback(varnam *handle, const char *word, int status, void *data)
{
  if (status == VARNAM_SUCCESS)
  {
    learn_passed_counter++;
  } else
  {
    printf("Failed to learn %s : %s\n", word, varnam_get_last_error(handle));
    learn_failed_counter++;
  }
  learn_counter++;
}

/**
 * Learn words from a file
 */
void learn_from(varnam *handle, char *file_path)
{
  int rc;

  rc = varnam_learn_from_file (handle, file_path, NULL, learn_callback, NULL);
	if (rc != VARNAM_SUCCESS)
	{
		printf("%s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		exit(1);
	}
  printf("Processed %d word(s). %d word(s) passed. %d word(s) failed.", learn_counter, learn_passed_counter, learn_failed_counter);
  exit(0);
}

/**
 * Train a word
 */
const char* perform_training(varnam *handle, char *pattern, char *word)
{
  int rc;

  ensure_single_word(pattern);
  ensure_single_word(word);

  rc = varnam_train(handle, pattern, word);
  if (rc != VARNAM_SUCCESS)
	{
		const char *error_message = varnam_get_last_error(handle);
		return error_message;
	}
  return NULL;
}

int train(varnam *handle, char *pattern, char *word)
{
  const char* error;

  ensure_single_word(word);
  
  error = perform_training(handle, pattern, word);
	if (error != NULL)
	{
		printf("%s", error);
    varnam_destroy(handle);
		exit(1);
	}
	printf("Success. %s will resolve to %s", pattern, word);
  exit(0);
}

/**
 * Import learnings from a file
 */
void import_learnings_from(varnam *handle, char *file_path)
{
  int rc;

  printf("Importing: %s\n", file_path);

  rc = varnam_import_learnings_from_file(handle, file_path);
  if (rc != VARNAM_SUCCESS)
	{
		const char *error_message = varnam_get_last_error(handle);
		printf("%s", error_message);
    exit(1);
	}

  printf("Done");
  exit(0);
}

void export_callback(int total_words, int total_processed, const char* current_word)
{
  float progress = (float) total_processed / total_words * 100;
  printf("\rExporting %d%%", (int) progress);
}

/**
 * Export words & patterns to a directory
 */
void export_full(varnam *handle, char *dir_path)
{
  int rc;

  if (!is_directory(dir_path)) {
    printf("varnamc : Output directory not found");
    exit(1);
  }

  printf("Exporting words from '%s' to '%s'\n", varnam_get_suggestions_file(handle), dir_path);

  rc = varnam_export_words(handle, 30000, dir_path, VARNAM_EXPORT_FULL, export_callback);
  if (rc != VARNAM_SUCCESS)
	{
		const char *error_message = varnam_get_last_error(handle);
		printf("Export failed. %s\n", error_message);
    exit(1);
	}

  printf("\nExported words to %s", dir_path);
  exit(0);
}

int main(int argc, char *argv[])
{
  struct arguments arguments = {NULL};
  int rc;
  varnam *handle;
  char *msg;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  if (arguments.version)
  {
    printf("libvarnam version %s", varnam_version());
    exit(0);
  } else if (arguments.symbols == NULL)
  {
    printf("varnamc : Can't load symbols file. Use --symbols option to specify the symbols file");
    exit(0);
  }

	/* Initialization */
	rc = varnam_init_from_id (arguments.symbols, &handle, &msg);

  if (rc != VARNAM_SUCCESS)
  {
		printf("Initialization failed. Reason - %s", msg);
		free (msg);
		exit(1);
	}

  if (arguments.transliterate != NULL)
  {
    transliterate(handle, arguments.transliterate, arguments.details);
  } else if (arguments.reverse_transliterate != NULL)
  {
    reverse_transliterate(handle, arguments.reverse_transliterate);
  } else if (arguments.learn != NULL)
  {
    learn(handle, arguments.learn);
  } else if (arguments.learn_from != NULL)
  {
    learn_from(handle, arguments.learn_from);
  } else if (arguments.train != NULL)
  {
    char **tokens;
    int count = split(arguments.train, '=', &tokens);

    if (count == 2)
    {
      train(handle, tokens[0], tokens[1]);
    } else
    {
      printf("varnamc : Incorrect arguments");
    }
  } else if (arguments.import_learnings_from != NULL)
  {
    import_learnings_from(handle, arguments.import_learnings_from);
  } else if (arguments.export_full != NULL)
  {
    export_full(handle, arguments.export_full);
  }

  /* 0 means program executed successfully */
  return 0;
}