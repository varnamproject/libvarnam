/* varnam.c

Copyright (C) 2010 Navaneeth.K.N

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string.h>
#include "module.h"
#include "util.h"

const char varnam_usage[] = 
    "varnam [--version] <command> [--help] | <args>\n";

const char varnam_version[] = 
    "Varnam version 0.0.1.a";

struct command_struct {
    const char *command;
    const char *description;
    int(*function)(int argc, char **argv);
};

static struct command_struct commands[] = {
        { "compile", "Compiles the provided scheme file to symbol table", compile_scheme_file },
        { "tl",      "Transliterate given input",                         transliterate_input }
};

static void varnam_print_usage(void)
{
    varnam_info("usage : %s", varnam_usage);
}

static void print_nchar(char c, size_t n)
{
    while(n--)
        fprintf(stdout, "%c", c);
}

static void varnam_print_commands()
{
    size_t i, longest = 0;
    varnam_info("Following commands are available : \n");

    for (i = 0; i < ARRAY_SIZE(commands); i++) {        
	struct command_struct *t  = commands + i;
        if( longest < strlen( t->command ) )
            longest = strlen( t->command );
    }

    for (i = 0; i < ARRAY_SIZE(commands); i++) { 
	struct command_struct *t  = commands + i;
        fprintf(stdout, "  %s    ", t->command);
        print_nchar(' ', longest - strlen( t->command ));
        fprintf(stdout, "%s\n", t->description);
    }
    varnam_info("");
}

static int execute_command(int argc, char **argv)
{
    const char *cmd_to_execute = argv[0];
    struct command_struct *cmd = 0;
    unsigned i;

    for (i = 0; i < ARRAY_SIZE(commands); i++) { 
	struct command_struct *t  = commands + i;
	if (strcmp(t->command, cmd_to_execute) == 0) {
            cmd = t;
            break;
        }
    }

    if(cmd == NULL) {
        varnam_error("varnam : unrecognized command '%s'.\n", cmd_to_execute);
        varnam_print_usage();
        varnam_print_commands();
        return VARNAM_ERROR;
    }
    argv++;
    argc--;
    return cmd->function(argc, argv);
}

int main(int argc, char **argv)
{
    if(argc == 1) {
        varnam_print_usage();
        varnam_print_commands();
        return VARNAM_OK;
    }
    else if(strcmp(argv[1], "--version") == 0) {
        varnam_info("%s\n", varnam_version);
        return VARNAM_OK;
    }
    argv++;
    argc--;
    return execute_command(argc, argv);
}
