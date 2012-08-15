/* tests.h - defines all available tests
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef TESTS_H_INCLUDED_125727
#define TESTS_H_INCLUDED_125727

extern int strbuf_test(int argc, char **argv);

/**
 * this tests the initialization of varnam library
 **/
extern int test_varnam_init(int argc, char **argv);

/**
 * this tests the basic transliteration process
 **/
extern int basic_transliteration(int argc, char **argv);

/**
 * this tests the transliteration for malayalam
 **/
extern int ml_unicode_transliteration(int argc, char **argv);

/**
 * this tests the reverse transliteration for malayalam
 **/
extern int ml_unicode_reverse_transliteration(int argc, char **argv);

/**
 * tests vst file generation, token creation etc
 **/
extern int test_vst_file_creation(int argc, char **argv);

/**
 * tests the learning module
 **/
extern int test_varnam_learn(int argc, char **argv);

extern int test_varnam_learn_from_file(int argc, char **argv);

#endif
