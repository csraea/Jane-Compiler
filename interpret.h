#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>


void init_interpret();

void error(const char *msg, KeySet K);

void check(const char *msg, KeySet K);

int match(const Symbol expected, KeySet K);

int ipow(int base, int exponentiation);

int term(KeySet K);
power(KeySet K);
mul(KeySet K);
expr(KeySet K);
void print(KeySet K),
iread(KeySet K);
stat(KeySet K);
program(KeySet K);
create_variable(KeySet K);
set_variable(KeySet K);