#include <bowl.h>

int main()
{

    // Root node which contains a list
    struct bowl *root;
    bowl_malloc(&root, HASH);

    struct bowl *a;
    data_malloc(&a, LITERAL, "Destroy capitalism");
    bowl_insert_key(root, "a", a);

    struct bowl *b;
    data_malloc(&b, INTEGER, 1312);
    bowl_insert_key(root, "b", b);

    struct bowl *c;
    data_malloc(&c, LITERAL, "Alerta, Antifascista!");
    bowl_insert_key(root, "c", c);

    return bowl_free(root);
}
