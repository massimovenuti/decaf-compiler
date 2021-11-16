#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 100

typedef struct s_symbol {
    char *ident;
    struct s_symbol *next;
} *Symbol;

typedef struct s_table
{
    Symbol entry[HASH_SIZE];
    struct s_table *next;
} *Table;

Symbol new_symbol();
Symbol add_symbol(Symbol sym, const char *ident);
Symbol search_symbol(Symbol sym, const char *ident);
void destroy_symbol(Symbol sym);
void print_symbol(Symbol sym);

Table new_table();
Table add_table(Table prev_scope);
Table add_entry(Table curr_scope, const char *ident);
Table pop_table(Table curr_scope);
int get_table_scope(Table curr_scope);
int search_entry_scope(Table curr_scope, const char *ident);
Symbol search_entry(Table curr_scope, const char *ident);
void print_table(Table curr_scope);

# endif
