#ifndef IFUNC_H
#define IFUNC_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_FILENAME 32
#define MAX_VARNAME 20
#define MAX_INPUT_SIZE 512

enum errorSet {
    SUCCESS,
    EMPTYINFILE,
    INVALIDAGRSNUM,
    EEXIT,
    MEMERROR,
    FILENEXISTS
};

extern char *source;

/* Inspect the file mentioned as input file:
 * check whether it's empty;
 * check whether it exists;
 * On success, returns the size of the file */
off_t inspect_inputFile();

/* Inspect the filename entered as the name of an input file:
 * check whether it's empty;
 * On success returns true, else - false */
bool inspect_filename(char filename[MAX_FILENAME]);


/* Get the input from a file or from a keyboard */
int get_input(int argc, char **argv);

int get_input_from_keyboard();
int get_input_from_file();

#endif