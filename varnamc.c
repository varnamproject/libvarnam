#include "varnam.h"

#include <argp.h>
#include <stdbool.h>

static char doc[] = "an Indic language transliteration library";
static char args_doc[] = "";
static struct argp_option options[] = { 
    { "symbols", 's', "VALUE", 0, "Sets the symbols file"},
    { "text", 't', "TEXT", 0, "Transliterate the given text"},
    { "learn", 'n', "TEXT", 0, "Learn the given text"},
    { "train", 'a', "PATTERN=WORD", 0, "Train the given text"},
    { 0 } 
};

struct arguments {
  char *symbols;
  char *text;
  char *learn;
  char *train;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 's': arguments->symbols = arg; break;
    case 't': arguments->text = arg; break;
    case 'n': arguments->learn = arg; break;
    case 'a': arguments->train = arg; break;
    case ARGP_KEY_ARG: return 0;
    default: return ARGP_ERR_UNKNOWN;
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
int split( char * str, char delim, char ***array ) {
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

static void
print_transliteration_output(const char *pattern, varray *words)
{
    int i;
    vword *word;
    for (i = 0; i < varray_length (words); i++)
    {
        word = varray_get (words, i);
        printf ("  %s. Confidence %d\n", word->text, word->confidence);
    }
}

/**
 * Transliterate a word
 */
int transliterate(varnam *handle, char *text)
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
	print_transliteration_output ("malayalam", words);
  exit(0);
}

/**
 * Learn a word
 */
int learn(varnam *handle, char *word)
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

int main(int argc, char *argv[])
{
  struct arguments arguments;
  int rc;
  varnam *handle;
  char *msg;

  arguments.symbols = NULL;
  arguments.text = NULL;
  arguments.learn = NULL;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  if (arguments.symbols == NULL) {
    printf("varnamc : Can't load symbols file. Use --symbols option to specify the symbols file");
    return 0;
  }

	/* Initialization */
	rc = varnam_init_from_id (arguments.symbols, &handle, &msg);

  if (rc != VARNAM_SUCCESS)
  {
		printf("Initialization failed. Reason - %s", msg);
		free (msg);
		return 1;
	}

  if (arguments.text != NULL)
  {
    transliterate(handle, arguments.text);
  } else if (arguments.learn != NULL)
  {
    learn(handle, arguments.learn);
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
  }

  /* 0 means program executed successfully */
  return 0;
}