
- [Introduction](#introduction)
- [Installing](#installing-libvarnam)
    - [Dependencies](#dependencies)
    - [Installation](#installation)
- [Getting started](#getting-started)
- [Public API](#public-api)
- [Supported languages](#supported-languages)
- [Adding a new language](#adding-a-new-language)
  - [Metadata](#metadata)
  - [Syntax](#syntax)
  - [Symbol types](#symbol-types)
  - [Other functions](#other-functions)
    - [infer_dead_consonants](#infer_dead_consonants)
    - [generate_cv](#generate_cv)
    - [list](#list)
    - [combine](#combine)
    - [Setting priority for a token](#setting-priority-for-a-token)
    - [Setting accept condition for a token](#setting-accept-condition-for-a-token)
- [Contributing](#contributing)
- [Contact](#contact)
- [License](#copyright)

# Introduction

`libvarnam` is a cross platform, self learning, open source library which support transliteration and reverse transliteration for Indian languages. At the core is a C shared library providing algorithms and patterns for transliteration. `libvarnam` has a simple learning module built-in which can learn words to improve the transliteration experience. 

# Installing libvarnam

## Dependencies

<table>
  <tr>
    <td>libvarnam</td><td>Standard C libraries</td>
  </tr>
  <tr>
    <td>varnamc (libvarnam's command line client)</td><td><ul><li>Ruby >= 1.9.3</li><li>ffi (gem install ffi)</li></ul></td>
  </tr>
  <tr>
    <td>libvarnam's unit tests</td><td>Check (http://check.sourceforge.net)</td>
  </tr>
</table>

## Installation

`libvarnam` uses `CMake` as build system. `libvarnam` doesn't have any external dependencies. So building it is easy.

```shell
$ cmake .
$ make
$ make install
```

# Getting started

You can use `varnamc` which is a command line client to `libvarnam` to quickly try out `libvarnam`. 

```shell
$ ./varnamc
varnamc : no actions specified
Usage: varnamc options args
    -l, --library FILE               Sets the varnam library
    -v, --verbose                    Enable verbose output
    -t, --transliterate TEXT         Transliterate the given text
        --indic-digits               Turns on indic digit rendering while transliterating
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
        --version                    Display version
```

Each supported language will have a scheme file under the `schemes` directory. This scheme file is in plain text format and needs to be compiled before using. To compile a scheme file, use the following command. 

```shell
$ ./varnamc --compile schemes/<SCHEME_FILE_NAME>
```

This will generate a file named <SCHEME_FILE_NAME>.vst (*V*arnam *S*ymbol *T*able) which is a binary file which contains all symbols defined in the scheme file. Running `make install` will install VST files and this allows `varnamc` to be used outside the source directory. 

You can now start using `libvarnam`. To transliterate a word.

```shell
$ ./varnamc --symbols ml --transliterate navaneeth
```

Above command uses Malayalam symbols and transliterate the text 'navaneeth'.

Similarly if you want varnam to learn some word

```shell
$ ./varnamc --symbols ml --learn വർണം
```

# Public API

`api.h` defines the public API for `libvarnam`. Take a look at api.h in the source for available functions.

In short, `libvarnam` can be initialized using `varnam_init()`. `varnam_init()` will initialize a handle which needs to be passed to all other functions. `varnam_transliterate()` can transliterate a word. `varnam_learn()` can be used to learn a word. 

Following example shows a simple usage of `libvarnam`.

```c
#include <varnam.h>

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

On a GNU/Linux machine, above example can be compiled using the following command:

```shell
gcc `pkg-config --cflags --libs varnam` -o example example.c
```

# Supported languages

* Assamese (Experimental)
* Bengali (Experimental)
* Gujarati (Experimental)
* Hindi
* Kannada
* Malayalam
* Marathi (Experimental)
* Nepali (Experimental)
* Odia (Experimental)
* Punjabi (Experimental)
* Sanskrit (Experimental)
* Tamil (Experimental)
* Telugu

# Adding a new language

A new language can be added to `libvarnam` by adding a new scheme file. A scheme file is a simple Ruby file which can be used to specify the symbols for a language. The best way to write a new scheme file is to refer to an existing one. All the scheme files are stored under `schemes/` directory.

## Metadata

A scheme file often starts with metadata.

<table>
<tr><td>language_code</td><td>Language code for the scheme</td></tr>
<tr><td>identifier</td><td>A unique identifier to identify this scheme</td></tr>
<tr><td>display_name</td><td>Friendly name for this scheme</td></tr>
<tr><td>author</td><td>Author of the scheme file</td></tr>
</table>

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

## Other functions

Following functions are available in a scheme file.

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

### generate_cv

When this function is called, varnam will autogenerate consonant-vowel combinations. Consider the following statements.

```ruby
vowels 'aa' => ['आ', 'ा']

consonants 'ka' => 'क'

generate_cv
```
In this case, varnam will generate consonant-vowel combinations like, `kaa` => 'का'

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

### Setting priority for a token

When defining a token, you can assign some priority to it. When varnam does the tokenization, high priority tokens will appear first in the list.

```ruby
consonants({:priority => :high}, 'ka' => 'क')
```

This will generate consonant `ka` with priority set to `high`.

### Setting accept condition for a token

Each token can have an optional accept condition. Accept condition can have 1 of 3 possible values. `starts_with`, `ends_with` and `in_between`. 

```ruby
consonants({:accept_if => :starts_with}, 'ka' => 'क')
```

In this case, varnam will accept token `ka` only if the pattern starts with `ka`.

# Contributing

Thank you for your interest. You can look at [issues](https://github.com/navaneeth/libvarnam/issues) and pick one which you find interesting to work with. Submit a pull request after the fix.

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
    <td>Email</td><td>varnamproject-discuss [at] nongnu.org</td>
  </tr>
</table>

# Copyright

Copyright (C) Navaneeth.K.N

This is part of libvarnam. See LICENSE.txt for the license

# The MIT License (MIT)

Copyright (c) 2013 Navaneeth.K.N

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

