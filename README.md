What is libvarnam
-------------------
'libvarnam' is a library which support transliteration and reverse transliteration. This documentation is work in progress.

Building libvarnam
-------------------
`libvarnam` uses `CMake` as build system. `libvarnam` doesn't have any external dependencies. So building it is easy.

```shell
$ cmake .
$ make
$ make install
```

Trying with varnamc
-------------------------

`varnamc` is a command line client to `libvarnam`. 

```shell
$ ./varnamc
varnamc : no actions specified
Usage: varnamc [options] language_code args
    -l, --library FILE               Sets the varnam library
    -v, --verbose                    Enable verbose output
    -t, --transliterate TEXT         Transliterate the given text
    -r, --reverse-transliterate TEXT Reverse transliterate the given text
    -n, --learn [TEXT]               Learn given text. Use --files option together with this to learn from file
    -a, --train PATTERN=WORD         Train varnam to use PATTERN for WORD
    -f, --files files                Reads from the specified files
    -s, --symbols VALUE              Sets the symbols file
    -c, --compile FILE               Compile symbols file
    -d, --output-dir dir             Sets the output directory
    -h, --help                       Display this screen
```

```shell
$ ./varnamc --symbols ml --transliterate navaneeth
```

Above command uses malayalam symbols and transliterate the text 'navaneeth'.

Usage
------

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

More details
============
Development updates to `libvarnam` will be posted at [varnam blog](http://navaneeth.github.com/libvarnam) and [website](http://www.varnamproject.com). `libvarnam` is under active development and no stable version is out yet.

