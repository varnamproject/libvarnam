What is libvarnam
-------------------
'libvarnam' is a library which support transliteration and reverse transliteration for Indian languages. 

News
-------------------
* Saturday Mar 30, 2013 - v2.0.0 released
* Thursday, Nov 1, 2012 - v1.0.1 released

Installing libvarnam
-------------------
`libvarnam` uses `CMake` as build system. `libvarnam` doesn't have any external dependencies. So building it is easy.

```shell
$ cmake .
$ make
$ make install
```

Getting started
-------------------------

You can use `varnamc` which is a command line client to `libvarnam` to quickly try out `libvarnam`. 

```shell
$ ./varnamc
varnamc : no actions specified
Usage: varnamc [options] language_code args
    -l, --library FILE               Sets the varnam library
    -v, --verbose                    Enable verbose output
    -t, --transliterate TEXT         Transliterate the given text
    -r, --reverse-transliterate TEXT Reverse transliterate the given text
    -n, --learn [TEXT]               Learn the given text
    -a, --train PATTERN=WORD         Train varnam to use PATTERN for WORD
    -f, --learn-from FILE|DIRECTORY  Reads from the specified file/directory
        --train-from FILE|DIRECTORY  Reads the specified file/directory and trains all the words specified
    -e, --export-words               Export words to the output directory
    -s, --symbols VALUE              Sets the symbols file
    -c, --compile FILE               Compile symbols file
        --learnings-file FILE        Specify the file to store all learnings
        --detect-language WORD       Detect language of the word
    -d, --output-dir dir             Sets the output directory
    -h, --help                       Display this screen
```

Each supported language will have a scheme file under the `schemes` directory. This scheme file is in plain text format and needs to be compiled before using. To compile a scheme file, use the following command. 

```shell
$ ./varnamc --compile schemes/<SCHEME_FILE_NAME>
```

This will generate a file named <SCHEME_FILE_NAME>.vst (*V*arnam *S*ymbol *T*able) which is a binary file which contains all symbols defined in the scheme file.  

You can now start using `libvarnam`. To transliterate a word.

```shell
$ ./varnamc --symbols ml --transliterate navaneeth
```

Above command uses malayalam symbols and transliterate the text 'navaneeth'.

Similarly if you want varnam to learn some word

```shell
$ ./varnamc --symbols ml --learn വർണം
```

Public API
-------------

`api.h` defines the public API for `libvarnam`. Take a look at [api.h](https://github.com/navaneeth/libvarnam/blob/master/api.h) for available functions.

In short, `libvarnam` can be initialized using `varnam_init()`. `varnam_init()` will initialize a handle which needs to be passed to all other functions. `varnam_transliterate()` can transliterate a word. `varnam_learn()` can be used to learn a word. 

Following example shows a simple usage of `libvarnam`.

```c
#include "varnam.h"

int main(int args, char **argv)
{
  int rc, i;
  char *error;
  varnam *handle;
  varray *result;
  vword *word;

  rc = varnam_init("path/to/ml-unicode.vst", &handle, &error);
  if (rc != VARNAM_SUCCESS)
  {
     printf ("Initialization failed. %s\n", error);
     return 1;
  }

  rc = varnam_transliterate (handle, "navaneeth", &result);
  if (rc != VARNAM_SUCCESS)
  {
     printf ("Transliteration failed. %s\n", varnam_get_last_error(handle));
     return 1;
  }

  for (i = 0; i < varray_length (result); i++)
  {
     word = varray_get (result, i);
     printf ("%s\n", word->text);
  }

  return 0;
}
```

Supported languages
-------------------
* Malayalam
* Gujarati (Experimental)
* Tamil (Experimental)

Adding a new language
---------------------

A new language can be added to `libvarnam` by adding a new scheme file. 

Contact
--------------------
<table>
  <tr>
    <td>Website</td><td>[www.varnamproject.com](http://www.varnamproject.com)</td>
  </tr>
  <tr>
    <td>IRC</td><td>#varnamproject at freenode</td>
  </tr>
  <tr>
    <td>Questions</td><td>Tweet your questions to @navaneethkn</td>
  </tr>
  <tr>
    <td>Email</td><td>navaneethkn [at] gmail</td>
  </tr>
</table>
