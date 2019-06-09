#include <bowl.h>

int main()
{

    // Root node which contains a list
    struct bowl *root;
    bowl_malloc(&root, ARRAY);

    struct bowl *a;
    data_malloc(&a, LITERAL, "Destroy capitalism");
    bowl_insert(root, a);

    struct bowl *b;
    data_malloc(&b, INTEGER, 1312);
    bowl_insert(root, b);

    struct bowl *c;
    data_malloc(&c, LITERAL, "Alerta, Antifascista!");
    bowl_insert(root, c);

    return bowl_free(root);
}