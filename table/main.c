# include "table.h"

int main(int argc, char **argv)
{
    Table st = new_table();
    st = add_table(st);
    st = add_entry(st, "rayan");
    st = add_entry(st, "victor");

    printf("> Table 1\n");
    print_table(st);
    printf("'rayan' is declared at scope %d\n", search_entry_scope(st, "rayan"));
    st = add_table(st);
    st = add_entry(st, "massimo");
    st = add_entry(st, "ahmet");
    st = add_entry(st, "alex");

    printf("\n> Table 2\n");
    print_table(st);
    printf("'ahmet' is declared at scope %d\n", search_entry_scope(st, "ahmet"));
    st = pop_table(st);

    printf("\n> Table 3\n");
    print_table(st);
    printf("'ahmet' is declared at scope %d\n", search_entry_scope(st, "ahmet"));
    st = pop_table(st);

    return EXIT_SUCCESS;
}
