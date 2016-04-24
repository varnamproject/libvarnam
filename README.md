Introduction
============

`libvarnam` is a cross platform, self learning, open source library which support transliteration and reverse transliteration for Indian languages. At the core is a C shared library providing algorithms and patterns for transliteration. `libvarnam` has a simple learning module built-in which can learn words to improve the transliteration experience.

Installing libvarnam
====================

```shell
wget http://download.savannah.gnu.org/releases/varnamproject/libvarnam/source/libvarnam-$VERSION.tar.gz
tar -xvf libvarnam-$VERSION.tar.gz
cd libvarnam-$VERSION
cmake . && make
sudo make install
```

This will install `libvarnam` shared libraries and `varnamc` command line utility. `varnamc` can be used to quickly try out varnam.

Usage
=====

### Transliterate

Usage: varnamc -s lang_code -t word

```shell
varnamc -s ml -t varnam
 വർണം
 വർണമേറിയത്
```

### Reverse Transliterate

Usage: varnamc -s lang_code -r word

```shell
varnamc -s ml -r വർണം
 varnam
```

What next?
==========

If you just wanted to use varnam for input, you have the following options

-	[Varnam on iBUS](https://github.com/varnamproject/libvarnam-ibus) - For Linux
-	[Varnam online editor](https://www.varnamproject.com/editor) - Platform agnostic

If you are a programmer, you will be interested in `libvarnam`. You can use it to provide indian language support in your applications. `libvarnam` can be used from different programming languages.

Mozilla Public License
======================

Copyright (c) 2016 Navaneeth.K.N

This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
