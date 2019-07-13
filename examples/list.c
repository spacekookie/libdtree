#include <bowl.h>

int main()
{

    // Root node which contains a list
    struct bowl *root;
    bowl_malloc(&root, ARRAY);

    struct bowl *a;
    data_malloc(&a, LITERAL, "Destroy capitalism");
    bowl_append(root, a);

    struct bowl *b;
    data_malloc(&b, INTEGER, 1312);
    bowl_append(root, b);

    struct bowl *c;
    data_malloc(&c, LITERAL, "Alerta, Antifascista!");
    bowl_append(root, c);

    return bowl_free(root);
}
