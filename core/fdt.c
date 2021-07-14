
/*
 * Flattend Device Tree (DTB) Parser
 */

#include <core/types.h>
#include "fdt.h"
#include "initfunc.h"
#include "mm.h"
#include "panic.h"
#include "string.h"

extern struct fdt_driver *__init_1_start[], *__init_1_end[];

struct fdt_reserve_entry {
    u64 address;
    u64 size;
};

u32 swap_u32(u32 val)
{
    val = ((val << 8) & 0xff00ff00) | ((val >> 8) & 0xff00ff);
    return (val << 16) | (val >> 16);
}

enum {
    FDT_BEGIN_NODE = 0x00000001,
    FDT_END_NODE = 0x00000002,
    FDT_PROP = 0x00000003,
    FDT_NOP = 0x00000004,
    FDT_END = 0x00000009,
};

struct fdt_header {
    u32 magic;
    u32 totalsize;
    u32 off_dt_struct;
    u32 off_dt_strings;
    u32 off_mem_rsvmap;
    u32 version;
    u32 last_comp_version;
    u32 boot_cpuid_phys;
    u32 size_dt_strings;
    u32 size_dt_struct;
};

struct fdt {
    struct fdt_header *header;
    void *struct_base;
    void *struct_current;
    void *string_base;
    void *mem_rsv_base;
    u32 version;
};



#define FDT_ADDR_CELLS_DEFAULT 2
#define FDT_SIZE_CELLS_DEFAULT 1

static void align_4byte(struct fdt *fdt)
{
    while((u64)fdt->struct_current & 0b11) {
        fdt->struct_current++;
    }
}

static struct fdt fdt;
static int fdt_setup_struct(struct fdt_header* header)
{
    fdt.header = header;
#define FDT_MAGIC_LITTLE 0xedfe0dd0
    if(fdt.header->magic != FDT_MAGIC_LITTLE) {
        return 1;
    }

    u32 off_struct = swap_u32(fdt.header->off_dt_struct);
    u32 off_strings = swap_u32(fdt.header->off_dt_strings);
    //u32 off_mem_rsv = swap_u32(fdt.header->off_mem_rsvmap);
    fdt.version = swap_u32(fdt.header->version);
    //u32 size_struct = swap_u32(fdt.header->size_dt_struct);
    //u32 size_strings = swap_u32(fdt.header->size_dt_strings);

    // structure block
    fdt.struct_base = (void *)fdt.header + off_struct;
    fdt.struct_current = fdt.struct_base;

    fdt.string_base = (void *)fdt.header + off_strings;

    return 0;
}

static struct fdt_node *fdt_parse_begin_node(struct fdt *fdt)
{
    struct fdt_node* node = (struct fdt_node *)alloc(sizeof (struct fdt_node));
    if (!node) {
        panic("failed to allocate memory");
    }

    fdt->struct_current += sizeof(u32);

    node->name = (char*)fdt->struct_current;
    //warnx("node name: %s", node->name);

    while(*(char *)fdt->struct_current++) {
    }

    align_4byte(fdt);

    return node;
}

static struct fdt_prop* fdt_get_compatible_prop(struct fdt_node *node)
{
    for (struct fdt_prop* prop = node->prop_head; prop; prop = prop->next) {
        if(!strcmp(prop->name, "compatible")) {
            return prop;
        }
    }

    return NULL;
}

static void fdt_call_driver_init(struct fdt_node *node)
{
    struct fdt_driver **p, *q;
    struct fdt_prop* compatible_prop = fdt_get_compatible_prop(node);

    if (!compatible_prop) {
        return;
    }

	for (p = __init_1_start; p != __init_1_end; p++) {
        q = *p;
        if (!strcmp(q->compatible, compatible_prop->str_list)) {
            q->init(node);
            break;
        }
    }
}

static char* fdt_get_string(struct fdt *fdt, u32 offset)
{
    return (char*)fdt->string_base + offset;
}

static struct prop_table_entry {
    char *name;
    enum fdt_prop_type type;
} prop_table[] = {
    // memory node
    {"device_type", FDT_PROP_STRING},
    {"reg", FDT_PROP_ARRAY},
    {"compatible", FDT_PROP_STR_LIST},
    {"#size-cells", FDT_PROP_U32},
    {"#address-cells", FDT_PROP_U32},
    {},
};

static void fdt_prop_parse_value(struct fdt_prop *prop, void *data, struct fdt_node* parent)
{
    struct prop_table_entry *entry = &prop_table[0];
    while(entry->name) {

        if(!strncmp(prop->name, entry->name, strlen(entry->name) + 1)) {
            prop->type = entry->type;
            goto found;
        }
        entry++;
    }
    return;

    //warnx("sizeof reg %ld", sizeof "reg");
    if(!strncmp(prop->name, "reg", sizeof "reg")) {
        prop->address_cells = parent->address_cells;
        prop->size_cells = parent->size_cells;
    }

found:
    switch(prop->type) {
        case FDT_PROP_STRING:
        {
            prop->string = (char*)data;
            break;
        }
        case FDT_PROP_STR_LIST:
        {
            prop->str_list = (char*)data;
            break;
        }
        case FDT_PROP_U32:
        {
            prop->val_u32 = swap_u32(*(u32*)data);
            break;
        }
        case FDT_PROP_ARRAY:
        {
            prop->base = data;
            break;
        }
        case FDT_PROP_U64:
        case FDT_PROP_EMPTY:
        case FDT_PROP_PHANDLE:
            panic("not yet implemented fdt prop type");
    }

    if(!strncmp(prop->name, "#size-cells", sizeof "#size-cells")) {
        parent->size_cells = prop->val_u32;
    }

    if(!strncmp(prop->name, "#address-cells", sizeof "#address-cells")) {
        parent->address_cells = prop->val_u32;
    }
}

static struct fdt_prop *fdt_parse_prop(struct fdt *fdt, struct fdt_node* parent)
{
    u32 nameoff;
    struct fdt_prop *prop = alloc(sizeof (struct fdt_prop));

    fdt->struct_current += sizeof(u32);

    prop->len = swap_u32(*(u32*)fdt->struct_current);
    fdt->struct_current += sizeof(u32);

    nameoff = swap_u32(*(u32*)fdt->struct_current);
    fdt->struct_current += sizeof(u32);

    prop->name = fdt_get_string(fdt, nameoff);

    //warnx("name: %s, prop len %d", prop->name, prop->len);

    fdt_prop_parse_value(prop, fdt->struct_current, parent);

    fdt->struct_current += prop->len;
    align_4byte(fdt);

    return prop;
}

static void fdt_init_driver(void)
{
    int ret = fdt_setup_struct((struct fdt_header*)get_fdt_base());
    if (ret)
        panic("cannot found fdt");

    struct fdt_node* parent = NULL;

    while(1) {
        u32 type = *(u32*)fdt.struct_current;

        switch(swap_u32(type)) {
            case FDT_BEGIN_NODE:
            {
                struct fdt_node *node = fdt_parse_begin_node(&fdt);
                if(!parent) {
                    parent = node;
                    parent->address_cells = FDT_ADDR_CELLS_DEFAULT;
                    parent->size_cells = FDT_SIZE_CELLS_DEFAULT;
                } else {
                    node->parent = parent;
                    node->address_cells = parent->address_cells;
                    node->size_cells = parent->size_cells;

                    if(parent->node_head) {
                        struct fdt_node* next = parent->node_head->next;
                        parent->node_head->next = node;
                        node->next = next;
                    } else {
                        parent->node_head = node;
                    }
                    parent = node;
                }
                break;
            }
            case FDT_PROP:
            {
                struct fdt_prop *prop = fdt_parse_prop(&fdt, parent);
                prop->address_cells = parent->address_cells;
                prop->size_cells = parent->size_cells;
                if(parent->prop_head) {
                    prop->next = parent->prop_head->next;
                    parent->prop_head->next = prop;
                } else {
                    parent->prop_head = prop;
                }
                break;
            }
            case FDT_END_NODE:
            {
                fdt.struct_current += sizeof(u32);
                fdt_call_driver_init(parent);
                if (parent->parent) {
                    parent = parent->parent;
                }
                break;
            }
            case FDT_END:
            {
                fdt.struct_current = 0;
                goto done;
            }
            case FDT_NOP:
                panic("not yet implemented");
                break;
            default:
                panic("invalid fdt structure type found");
                break;
        }
    }

done:
}


int fdt_get_reg_value(struct fdt_node* node, int index, enum FDT_REG_TYPE type, u64 *value)
{
    for (struct fdt_prop* p = node->prop_head; p; p = p->next) {
        if(!strcmp(p->name, "reg")) {
		// assert(p->address_cells <= 2);
		// assert(p->size_cells <= 2);
		// assert(p->type == FDT_PROP_ARRAY);

		u64 address = 0;
		u64 size = 0;
		void *base = p->base + sizeof (u32) * p->address_cells * index +
			     sizeof (u32) * p->size_cells * index;

		// check a range of the data
		if (p->base + p->len <= base) {
			return -1;
		}

		if (p->address_cells >= 1) {
			address = swap_u32 (*(u32 *)base);
			base += sizeof (u32);
		}

		if (p->address_cells == 2) {
			address = address << 32 | swap_u32 (*(u32 *)base);
			base += sizeof (u32);
		}

		if (p->size_cells >= 1) {
			size = swap_u32 (*(u32 *)base);
			base += sizeof (u32);
		}

		if (p->size_cells == 2) {
			size = size << 32 | swap_u32 (*(u32 *)base);
			base += sizeof (u32);
		}

		switch (type) {
		case FDT_REG_ADDRESS:
			*value = address;
			return 0;
		case FDT_REG_SIZE:
			*value = size;
			return 0;
		}

		return -1;
	}
    }

    return -1;
}

INITFUNC ("driver0", fdt_init_driver);
