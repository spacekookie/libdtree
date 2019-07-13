#include <bowl.h>

int main()
{
    err_t e;

    // Initialise a root node
    struct bowl *root;
    e = bowl_malloc(&root, ARRAY);
    if(e) return e;

    // First Node
    struct bowl *a;
    e = data_malloc(&a, LITERAL, "Destroy capitalism");
    if(e) return e;

    // Second Node
    struct bowl *b;
    e = data_malloc(&b, INTEGER, 1312);
    if(e) return e;

    // Third node is another ARRAY
    struct bowl *c;
    e = bowl_malloc(&c, ARRAY);
    if(e) return e;

    // Fourth node is another string
    struct bowl *d;
    e = data_malloc(&d, LITERAL, "Alerta, Antifascista!");
    if(e) return e;

    // Add the d node to c
    e = bowl_append(c, d);
    if(e) e;

    // Add other nodes to root
    e = bowl_append(root, a);
    if(e) return e;
    e = bowl_append(root, b);
    if(e) return e;
    e = bowl_append(root, c);
    if(e) return e;

    e = bowl_free(root);
    return e;
}
