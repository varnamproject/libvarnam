#include "varnam.h"

#include <argp.h>
#include <stdbool.h>

static char doc[] = "an Indic language transliteration library";
static char args_doc[] = "";
static struct argp_option options[] = { 
    { "symbols", 's', "VALUE", 0, "Sets the symbols file"},
    { "text", 't', "TEXT", 0, "Transliterate the given text"},
    { "learn", 'n', "TEXT", 0, "Learn the given text"},
    { 0 } 
};

struct arguments {
  char *symbols;
  char *text;
  char *learn;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
    case 's': arguments->symbols = arg; break;
    case 't': arguments->text = arg; break;
    case 'n': arguments->learn = arg; break;
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
void ensure_single_word(char *text) {
  int i = 0;
  int s_length = strlen(text);
  while(i < s_length)
  {
    if(text[i++] == ' ') {
      printf("varnamc : Expected a single word.");
      exit(1);
    }
  }
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
int learn(varnam *handle, char *text)
{
  ensure_single_word(text);

  int rc = varnam_learn (handle, text);
	if (rc != VARNAM_SUCCESS)
	{
		printf("%s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		exit(1);
	}
	printf("Learned %s", text);
  exit(0);
}

int main(int argc, char *argv[])
{
  struct arguments arguments;

  arguments.symbols = NULL;
  arguments.text = NULL;
  arguments.learn = NULL;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  int rc;
  varnam *handle;
  char *msg;

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
  }

  /* 0 means program executed successfully */
  return 0;
}