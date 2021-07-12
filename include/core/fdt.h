#ifndef _CORE_FDT_H
#define _CORE_FDT_H

struct fdt_node {
    char* name;
    struct fdt_node *next;
    struct fdt_prop *prop_head;
    struct fdt_node *parent;
    struct fdt_node *node_head;
    u32 address_cells, size_cells;
};

struct fdt_driver {
    char *compatible;
    void (*init)(struct fdt_node* node);
};

#define FDT_DRIVER(id, driver) struct fdt_driver *__initfdt_##id \
    __attribute__ ((__section__ (".init1"), aligned (1))) = driver

#endif
