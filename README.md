- [Introduction](#introduction)
- [News](#news)
- [Installing](#installing)
- [Getting started](#getting_started)
- [Public API](#public_api)
- [Supported languages](#supported_languages)
- [Adding a new language](#adding_a_new_language)
  - [Metadata](#adding_a_new_language_metadata)
  - [Syntax](#adding_a_new_language_syntax)
  - [Symbol types](#adding_a_new_language_symbol_types)
  - [Other functions](#adding_a_new_language_other_functions)
    - [infer_dead_consonants](#adding_a_new_language_other_functions_infer_dead_consonants)
    - [generate_cv](#adding_a_new_language_other_functions_generate_cv)
    - [combine](#adding_a_new_language_other_functions_combine)
    - [list](#adding_a_new_language_other_functions_list)
    - [Setting priority for a token](#adding_a_new_language_other_functions_setting_priority_for_a_token)
    - [Setting accept condition for a token](#adding_a_new_language_other_functions_setting_accept_condition_for_a_token)
- [Contributing](#contributing)
- [Contact](#contact)

<a name="introduction" />
# Introduction

`libvarnam` is a cross platform, self learning, open source library which support transliteration and reverse transliteration for Indian languages. At the core is a C shared library providing algorithms and patterns for transliteration. `libvarnam` has a simple learning module built-in which can learn words to improve the transliteration experience. 

<a name="news" />
# News

* Sunday   Jun 02, 2013 - v2.1.0 released
* Saturday Mar 30, 2013 - v2.0.0 released
* Thursday Nov 01, 2012 - v1.0.1 released

<a name="installing" />
# Installing libvarnam

`libvarnam` uses `CMake` as build system. `libvarnam` doesn't have any external dependencies. So building it is easy.

```shell
$ cmake .
$ make
$ make install
```
By default, `libvarnam` installs a build which contains `tests` and `examples`. You can build just the library by,

```shell
$ cmake . -DBUILD_TESTS=false -DBUILD_EXAMPLES=false -DBUILD_TOOLS=false
$ make
$ make install
```

<a name="getting_started" />
# Getting started

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

Above command uses Malayalam symbols and transliterate the text 'navaneeth'.

Similarly if you want varnam to learn some word

```shell
$ ./varnamc --symbols ml --learn വർണം
```

<a name="public_api" />
# Public API

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

<a name="supported_languages" />
# Supported languages

* Hindi
* Malayalam
* Gujarati (Experimental)

<a name="adding_a_new_language" />
# Adding a new language

A new language can be added to `libvarnam` by adding a new scheme file. A scheme file is a simple Ruby file which can be used to specify the symbols for a language. The best way to write a new scheme file is to refer to an existing one. All the scheme files are stored under `schemes/` directory.

<a name="adding_a_new_language_metadata" />
## Metadata

A scheme file often starts with metadata.

<table>
<tr><td>language_code</td><td>Language code for the scheme</td></tr>
<tr><td>identifier</td><td>A unique identifier to identify this scheme</td></tr>
<tr><td>display_name</td><td>Friendly name for this scheme</td></tr>
<tr><td>author</td><td>Author of the scheme file</td></tr>
</table>

<a name="adding_a_new_language_syntax" />
## Syntax

```ruby
<symbol-type> options, symbols
```

`options` and `symbols` should be valid Ruby hashes. `options` is optional argument and can contain the following values.

```ruby
options = {:accept_if => starts_with | ends_with | in_between, :priority => 0..9}
```

`symbols` should be a hash with patterns as keys and replacement as values. It can have the following form.

```ruby
'a' => 'a-value', 'b' => 'b-value'
['a', 'aa'] => 'b-value'
```

Given the above mapping, varnam will replace token `a` with `a-value` and token `b` with `b-value`. Multiple patterns can be specified in an array. In this case, both `a` and `aa` will resolve to `b-value`.

<a name="adding_a_new_language_symbol_types" />
## Symbol types

The following functions are available in the scheme files to define different types of symbols. 

* vowels
* consonants - Usually specified with the inherent 'a' sound.
* consonant_vowel_combinations
* anusvara
* visarga
* virama
* symbols
* numbers
* others

<a name="adding_a_new_language_other_functions" />
## Other functions

Following functions are available in a scheme file.

<a name="adding_a_new_language_other_functions_infer_dead_consonants" />
### infer_dead_consonants

Usage

```ruby
infer_dead_consonants true
```

When this option is set, varnam will infer dead consonant from a consonant definition. Consider the following statements. 

```ruby
infer_dead_consonants true

consonants 'ka' => 'क'
```

In this case, varnam will create a consonant `ka` which will resolve to `क` and a dead consonant `k` which resolves to `क्`.

<a name="adding_a_new_language_other_functions_generate_cv" />
### generate_cv

When this function is called, varnam will autogenerate consonant-vowel combinations. Consider the following statements.


```ruby
vowels 'aa' => ['आ', 'ा']

consonants 'ka' => 'क'

generate_cv
```
In this case, varnam will generate consonant-vowel combinations like, `kaa` => 'का'

<a name="adding_a_new_language_other_functions_list" />
### list

Creates a custom list and adds the tokens into the list.

```ruby
list :consonants_with_inherent_a_sound do
   consonants 'ka' => 'क'
end

# Token 'ka' will be added to the custom list named 'consonants_with_inherent_a_sound'. To read it,
consonants_with_inherent_a_sound.each do |c|
  puts c
end
```
<a name="adding_a_new_language_other_functions_combine" />
### combine

`combine` function can be used to generate combination of tokens. Consider the following scheme file for *Hindi*.

```ruby
consonants "k" => "क",
           ["kh", ["gh"]] => "ख",
           ["gh", ["kh"]] => "घ",

# Generating ka, kha etc
consonants(combine get_consonants, ["*a"] => ["*1"])
```

It takes a list as the first argument and hash as the second argument. List could be any custom defined lists created using the `list` function or it could be any built-in list. In the above example, `combine` will iterate over the list `get_consonants` and replace the wildcard character `*` with current pattern and `*1` with value1. For values, you can use `*1`, `*2` and `*3` for getting `value1`, `value2` and `value3`.

`combine` function returns a hash that can be passed to token creation functions like `consonants` or `vowels`.

<a name="adding_a_new_language_other_functions_setting_priority_for_a_token" />
### Setting priority for a token

When defining a token, you can assign some priority to it. When varnam does the tokenization, high priority tokens will appear first in the list.

```ruby
consonants({:priority => :high}, 'ka' => 'क')
```

This will generate consonant `ka` with priority set to `high`.

<a name="adding_a_new_language_other_functions_setting_accept_condition_for_a_token" />
### Setting accept condition for a token

Each token can have an optional accept condition. Accept condition can have 1 of 3 possible values. `starts_with`, `ends_with` and `in_between`. 

```ruby
consonants({:accept_if => :starts_with}, 'ka' => 'क')
```

In this case, varnam will accept token `ka` only if the pattern starts with `ka`.

<a name="contributing" />
# Contributing

Thank you for your interest. You can look at [issues](https://github.com/navaneeth/libvarnam/issues) and pick one which you find interesting to work with. Submit a pull request after the fix.

<a name="contact" />
# Contact

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
    <td>Email</td><td>varnamproject [at] googlegroups.com</td>
  </tr>
</table>
