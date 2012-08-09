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

Compiling varnam schemes
-------------------------

To use `libvarnam`, you need to compile one of the scheme files.

```shell
$ ./varnamc --compile schemes/ml-unicode
```

Compilation generates a binary file named `ml-unicode.vst`.

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

