/* tests.h - defines all available tests
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


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

extern int test_varnam_export(int argc, char **argv);

#endif
