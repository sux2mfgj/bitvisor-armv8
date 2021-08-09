#ifndef _CORE_FDT_H
#define _CORE_FDT_H

#include <core/types.h>

enum fdt_prop_type {
    FDT_PROP_EMPTY,
    FDT_PROP_U32,
    FDT_PROP_U64,
    FDT_PROP_STRING,
    FDT_PROP_ARRAY,
    FDT_PROP_PHANDLE,
    FDT_PROP_STR_LIST,
};

enum fdt_node_type {
	FDT_BEGIN_NODE = 0x00000001,
	FDT_END_NODE = 0x00000002,
	FDT_PROP = 0x00000003,
	FDT_NOP = 0x00000004,
	FDT_END = 0x00000009,
};

struct fdt_prop {
    struct fdt_prop *next;
    struct fdt_node *parent;
    char* name;
    enum fdt_prop_type type;
    u32 len;
    union {
        u32 val_u32;
        u64 val_u64;
        char* string;
        void* base;
        u32 phandle;
        char *str_list;
    };
};

struct fdt_node {
    char* name;
    enum fdt_node_type type;
    struct fdt_node *next;
    struct fdt_prop *prop_head;
    struct fdt_node *parent;
    struct fdt_node *node_head, *node_tail;
    u32 address_cells, size_cells;
    enum {
        CELLS_PARENT = 0b00, // use parent cells value
        CELLS_BOTH = 0b11, // use own cells value
        CELLS_ADDR = 0b10, // just use own addr cell
        CELLS_SIZE = 0b01, // just use own size cell
    } cells_type;
};

struct fdt_driver {
    char *compatible;
    void (*init)(struct fdt_node* node);
};

#define FDT_DRIVER(id, driver) struct fdt_driver *__initfdt_##id \
    __attribute__ ((__section__ (".init1"), aligned (1))) = driver

enum FDT_REG_TYPE {
    FDT_REG_ADDRESS,
    FDT_REG_SIZE,
};

int fdt_get_reg_value(struct fdt_node* node, int index, enum FDT_REG_TYPE type, u64 *value);

#endif
