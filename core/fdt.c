
/*
 * Flattend Device Tree (DTB) Parser
 */

#include "fdt.h"

#include <core.h>
#include <core/string.h>
#include <core/types.h>
#include <test.h>

#include "initfunc.h"
#include "mm.h"
#include "panic.h"

extern struct fdt_driver *__init_1_start[], *__init_1_end[];

struct fdt_reserve_entry {
	u64 address;
	u64 size;
};

u32
swap_u32 (u32 val)
{
	val = ((val << 8) & 0xff00ff00) | ((val >> 8) & 0xff00ff);
	return (val << 16) | (val >> 16);
}

#define FDT_ADDR_CELLS_DEFAULT 2
#define FDT_SIZE_CELLS_DEFAULT 1

static void
align_4byte (struct fdt *fdt)
{
	while ((u64)fdt->struct_current & 0b11) {
		fdt->struct_current++;
	}
}

static struct fdt *g_fdt;

test_static struct fdt *
fdt_setup_struct (struct fdt_header *header)
{
	struct fdt *fdt = alloc (sizeof (struct fdt));

	fdt->header = header;
#define FDT_MAGIC_LITTLE 0xedfe0dd0
	if (fdt->header->magic != FDT_MAGIC_LITTLE) {
		return NULL;
	}

	u32 off_struct = swap_u32 (fdt->header->off_dt_struct);
	u32 off_strings = swap_u32 (fdt->header->off_dt_strings);
	// u32 off_mem_rsv = swap_u32(fdt.header->off_mem_rsvmap);
	fdt->version = swap_u32 (fdt->header->version);
	// u32 size_struct = swap_u32(fdt.header->size_dt_struct);
	// u32 size_strings = swap_u32(fdt.header->size_dt_strings);

	// structure block
	fdt->struct_base = (void *)fdt->header + off_struct;
	fdt->struct_current = fdt->struct_base;

	fdt->string_base = (void *)fdt->header + off_strings;

	return fdt;
}

static struct fdt_node *
fdt_parse_begin_node (struct fdt *fdt)
{
	struct fdt_node *node =
		(struct fdt_node *)alloc (sizeof (struct fdt_node));
	if (!node) {
		panic ("failed to allocate memory");
	}

	fdt->struct_current += sizeof (u32);

	node->name = (char *)fdt->struct_current;

	while (*(char *)fdt->struct_current++) {
	}

	align_4byte (fdt);

	return node;
}

static struct fdt_prop *
fdt_get_compatible_prop (struct fdt_node *node)
{
	for (struct fdt_prop *prop = node->prop_head; prop; prop = prop->next) {
		if (!strcmp (prop->name, "compatible")) {
			return prop;
		}
	}

	return NULL;
}

static void
fdt_call_driver_init (struct fdt_node *node)
{
	struct fdt_driver **p, *q;
	struct fdt_prop *compatible_prop = fdt_get_compatible_prop (node);

	if (!compatible_prop) {
		return;
	}

	for (p = __init_1_start; p != __init_1_end; p++) {
		q = *p;
		// TODO change to compare with all of entries.
		if (!strcmp (q->compatible, compatible_prop->str_list)) {
			q->init (node);
			break;
		}
	}
}

static char *
fdt_get_string (struct fdt *fdt, u32 offset)
{
	return (char *)fdt->string_base + offset;
}

static void
fdt_parse_clock_freq (struct fdt_prop *prop, void *data)
{
	switch (prop->len) {
	case sizeof (u32):
		prop->val_u32 = swap_u32 (*(u32 *)data);
		break;
	case sizeof (u64):
		not_yet_implemented ();
		break;
	default:
		panic ("%s: unknown prop length", __func__);
	}
}

static void
fdt_parse_range (struct fdt_prop *prop, void *data)
{
	if (prop->len == 0) {
		prop->type = FDT_PROP_EMPTY;
		return;
	}

	not_yet_implemented ();
}

static struct prop_table_entry {
	char *name;
	enum fdt_prop_type type;
	void (*parse) (struct fdt_prop *prop, void *data);
} prop_table[] = {
	// memory node
	{"device_type", FDT_PROP_STRING},
	{"reg", FDT_PROP_ARRAY},
	{"compatible", FDT_PROP_STR_LIST},
	{"#size-cells", FDT_PROP_U32},
	{"#address-cells", FDT_PROP_U32},
	{"interrupt-parent", FDT_PROP_PHANDLE},
	{"clock-names", FDT_PROP_STR_LIST},
	{"clocks", FDT_PROP_PH_LIST},
	{"interrupts", FDT_PROP_U32_LIST},
	{"phandle", FDT_PROP_PHANDLE},
	{"clock-output-names", FDT_PROP_STR_LIST},
	{"clock-frequency", FDT_PROP_ENC_ARY, fdt_parse_clock_freq},
	{"#clock-cells", FDT_PROP_U32},
	{"ranges", FDT_PROP_ENC_ARY, fdt_parse_range},
	{"interrupt-controller", FDT_PROP_EMPTY},
	{"#interrupt-cells", FDT_PROP_U32},
	{"always-on", FDT_PROP_EMPTY},
	{"stdout-path", FDT_PROP_STRING},
	{"stdin-path", FDT_PROP_STRING},
	{"bootargs", FDT_PROP_STRING},
	{},
};

static void
fdt_prop_parse_value (struct fdt_prop *prop, void *data,
		      struct fdt_node *parent)
{
	struct prop_table_entry *entry = &prop_table[0];
	while (entry->name) {
		if (!strncmp (prop->name, entry->name,
			      strlen (entry->name) + 1)) {
			prop->type = entry->type;
			goto found;
		}
		entry++;
	}
	panic ("%s: unknown prop found: %s\n", __func__, prop->name);
	return;

found:
	switch (prop->type) {
	case FDT_PROP_STRING: {
		prop->string = (char *)data;
		break;
	}
	case FDT_PROP_STR_LIST: {
		prop->str_list = (char *)data;
		break;
	}
	case FDT_PROP_U32: {
		ASSERT (prop->len == sizeof (u32));
		prop->val_u32 = swap_u32 (*(u32 *)data);
		break;
	}
	case FDT_PROP_ARRAY: {
		prop->base = data;
		break;
	}
	case FDT_PROP_PHANDLE: {
		prop->phandle = swap_u32 (*(u32 *)data);
		break;
	}
	case FDT_PROP_PH_LIST: {
		prop->base = data;
		break;
	}
	case FDT_PROP_U32_LIST: {
		prop->base = data;
		break;
	}
	case FDT_PROP_ENC_ARY: {
		entry->parse (prop, data);
		break;
	}
	case FDT_PROP_EMPTY:
		break;
	case FDT_PROP_U64:
		panic ("not yet implemented fdt prop type");
	}

	if (!strncmp (prop->name, "#size-cells", sizeof "#size-cells")) {
		parent->size_cells = prop->val_u32;
		parent->cells_type |= CELLS_SIZE;
	}

	if (!strncmp (prop->name, "#address-cells", sizeof "#address-cells")) {
		parent->address_cells = prop->val_u32;
		parent->cells_type |= CELLS_ADDR;
	}
}

static struct fdt_prop *
fdt_parse_prop (struct fdt *fdt, struct fdt_node *parent)
{
	u32 nameoff;
	struct fdt_prop *prop = alloc (sizeof (struct fdt_prop));

	fdt->struct_current += sizeof (u32);

	prop->len = swap_u32 (*(u32 *)fdt->struct_current);
	fdt->struct_current += sizeof (u32);

	nameoff = swap_u32 (*(u32 *)fdt->struct_current);
	fdt->struct_current += sizeof (u32);

	prop->name = fdt_get_string (fdt, nameoff);
	prop->parent = parent;

	fdt_prop_parse_value (prop, fdt->struct_current, parent);

	fdt->struct_current += prop->len;
	align_4byte (fdt);

	return prop;
}

test_static struct fdt *
fdt_parse (struct fdt_header *header)
{
	struct fdt *fdt = fdt_setup_struct (header);
	if (!fdt)
		panic ("cannot found fdt");

	struct fdt_node *parent = NULL;

	while (1) {
		u32 type = *(u32 *)fdt->struct_current;

		switch (swap_u32 (type)) {
		case FDT_BEGIN_NODE: {
			struct fdt_node *node = fdt_parse_begin_node (fdt);
			if (!parent) {
				parent = node;
				parent->name = "root";
				parent->address_cells = FDT_ADDR_CELLS_DEFAULT;
				parent->size_cells = FDT_SIZE_CELLS_DEFAULT;
			} else {
				node->parent = parent;
				node->cells_type = CELLS_PARENT;

				if (parent->node_head) {
					node->next = NULL;
					parent->node_tail->next = node;
					parent->node_tail = node;
				} else {
					parent->node_head = node;
					parent->node_tail = node;
				}
				parent = node;
			}
			printf ("%s :%s\n", __func__, node->name);
			break;
		}
		case FDT_PROP: {
			struct fdt_prop *prop = fdt_parse_prop (fdt, parent);
			if (parent->prop_head) {
				prop->next = parent->prop_head->next;
				parent->prop_head->next = prop;
			} else {
				parent->prop_head = prop;
			}
			break;
		}
		case FDT_END_NODE: {
			fdt->struct_current += sizeof (u32);
			fdt_call_driver_init (parent);

			if (parent->parent) {
				parent = parent->parent;
			}
			break;
		}
		case FDT_END: {
			fdt->struct_current = 0;
			goto done;
		}
		case FDT_NOP:
			panic ("not yet implemented");
			break;
		default:
			panic ("invalid fdt structure type found");
			break;
		}
	}

done:
	fdt->root_node = parent;

	return fdt;
}

static u32
fdt_get_totalsize (struct fdt_header *header)
{
	return swap_u32 (header->totalsize);
}

struct fdt_header *
fdt_copy_to_local (void *base)
{
	u32 totalsize;
	struct fdt_header *dest;
	struct fdt_header *header = (struct fdt_header *)base;

#define FDT_MAGIC_LITTLE 0xedfe0dd0
	if (header->magic != FDT_MAGIC_LITTLE) {
		return NULL;
	}

	totalsize = fdt_get_totalsize (header);

	u32 pages = (totalsize / PAGESIZE) + !!(totalsize & PAGESIZE_MASK);

	alloc_pages ((void **)&dest, NULL, pages);

	memcpy (dest, base, totalsize);

	return dest;
}

void *get_fdt_base (void);

static void
fdt_init_driver (void)
{
	struct fdt_header *orig = (struct fdt_header *)get_fdt_base ();
	struct fdt_header *new;

	new = fdt_copy_to_local (orig);

	g_fdt = fdt_parse (new);
}

static struct fdt_node *
fdt_search_node (struct fdt_node *parent, char *name)
{
	for (struct fdt_node *n = parent; n; n = n->next) {
		if (!strcmp (n->name, name))
			return n;

		if (n->node_head)
			return fdt_search_node (n->node_head, name);
	}

	return NULL;
}

struct fdt_node *
fdt_get_node (char *name)
{
	return fdt_search_node (g_fdt->root_node, name);
}

struct fdt_prop *
fdt_get_prop (struct fdt_node *node, char *name)
{
	for (struct fdt_prop *p = node->prop_head; p; p = p->next) {
		if (!strcmp (p->name, name))
			return p;
	}

	return NULL;
}

static u32
fdt_get_addr_cells (struct fdt_node *node)
{
	if (node->cells_type & CELLS_ADDR)
		return node->address_cells;
	else
		return fdt_get_addr_cells (node->parent);
}

static u32
fdt_get_size_cells (struct fdt_node *node)
{
	if (node->cells_type & CELLS_SIZE)
		return node->size_cells;
	else
		return fdt_get_size_cells (node->parent);
}

int
fdt_get_reg_value (struct fdt_node *node, int index, enum FDT_REG_TYPE type,
		   u64 *value)
{
	for (struct fdt_prop *p = node->prop_head; p; p = p->next) {
		if (!strcmp (p->name, "reg")) {
			ASSERT (p->type == FDT_PROP_ARRAY);

			u64 address = 0;
			u64 size = 0;
			u32 address_cells = fdt_get_addr_cells (node);
			u32 size_cells = fdt_get_size_cells (node);

			void *base = p->base +
				     sizeof (u32) * address_cells * index +
				     sizeof (u32) * size_cells * index;

			// check a range of the data
			if (p->base + p->len <= base) {
				return -1;
			}

			if (address_cells >= 1) {
				address = swap_u32 (*(u32 *)base);
				base += sizeof (u32);
			}

			if (address_cells == 2) {
				address =
					address << 32 | swap_u32 (*(u32 *)base);
				base += sizeof (u32);
			}

			if (size_cells >= 1) {
				size = swap_u32 (*(u32 *)base);
				base += sizeof (u32);
			}

			if (size_cells == 2) {
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

static u32
fdt_calc_prop_size (struct fdt_prop *prop)
{
	u32 size = 0;
	// node type + length + nameoff
	size += sizeof (u32) * 3;
	size += prop->len + (-prop->len & 0b11);

	return size;
}

static u32
fdt_calc_node_size (struct fdt_node *node)
{
	u32 size = 0;

	// type of node (FDT_BEGIN_NODE)
	size += sizeof (u32);
	// aligned 4bytes string length
	u32 tmp = strlen (node->name) + 1;
	size += tmp + (-tmp & 0b11);

	// prop nodes
	for (struct fdt_prop *p = node->prop_head; p; p = p->next) {
		u32 psize = fdt_calc_prop_size (p);
		size += psize;
		printf ("%s: node: %s: prop: %s: size: %d\n", __func__,
			node->name, p->name, psize);
	}

	// type of node (FDT_END_NODE)
	size += sizeof (u32);

	printf ("%s: node: %s size: %d\n", __func__, node->name, size);

	return size;
}

test_static u32
fdt_calc_structblock_totalsize (struct fdt_node *node)
{
	u32 totalsize = 0;
	for (struct fdt_node *n = node; n; n = n->next) {
		totalsize += fdt_calc_node_size (n);

		if (n->node_head)
			totalsize +=
				fdt_calc_structblock_totalsize (n->node_head);
	}

	return totalsize;
}

static void
_fdt_load_structblock (struct fdt_node *node, void **base)
{
	// load FDT_BEGIN_NODE
	*(u32 *)*base = swap_u32 (FDT_BEGIN_NODE);
	*base += sizeof (u32);
	u32 len = strlen (node->name) + 1;
	memcpy ((char *)*base, node->name, len);
	*base += len + (-len & 0b11);

	// add nodes for properties
	not_yet_implemented ();
	for (struct fdt_prop *p = node->prop_head; p; p = p->next) {
	}

	// load FDT_END_NODE
	*(u32 *)*base = swap_u32 (FDT_END_NODE);
	*base += sizeof (u32);
}

static void
fdt_load_structblock (struct fdt_node *node, void **base)
{
	not_yet_implemented ();
	for (struct fdt_node *n = node; n; n = n->next) {
	}

	// load FDT_END
}

static u32
fdt_calc_prop_string_totalsize (struct fdt_prop *prop)
{
	u32 totalsize = 0;

	for (struct fdt_prop *p = prop; p; p = p->next) {
		totalsize += strlen (p->name) + 1;
	}

	return totalsize;
}

test_static u32
fdt_calc_stringblock_totalsize (struct fdt_node *node)
{
	u32 totalsize = 0;

	for (struct fdt_node *n = node; n; n = n->next) {
		totalsize += fdt_calc_prop_string_totalsize (n->prop_head);

		if (n->node_head)
			totalsize +=
				fdt_calc_stringblock_totalsize (n->node_head);
	}

	return totalsize;
}

static void
fdt_load_stringblock (struct fdt_node *node, void *base)
{
}

static void
fdt_load_to (void *dest)
{
	struct fdt_header *header;
	// TODO update header
	memcpy (dest, g_fdt->header, sizeof g_fdt->header);
	header = dest;

	// fdt.root_node
	u32 struct_block_totalsize =
		fdt_calc_structblock_totalsize (g_fdt->root_node);

	u32 string_block_totalsize =
		fdt_calc_stringblock_totalsize (g_fdt->root_node);

#define FDT_MEM_RSVMAP_OFFSET 0x30
	u32 offset = FDT_MEM_RSVMAP_OFFSET;
	header->off_mem_rsvmap = swap_u32 (offset);
	offset += 0x10;

	header->off_dt_struct = swap_u32 (offset);
	header->size_dt_struct = swap_u32 (struct_block_totalsize);
	// TODO load struct block to (void*)header + offset;
	offset += struct_block_totalsize;
	offset += (-offset & 0x0f); // align 0x10

	header->off_dt_strings = swap_u32 (offset);
	header->size_dt_strings = swap_u32 (string_block_totalsize);
	// TODO load string block to (void*)header + offset;
	offset += string_block_totalsize;
	offset += (-offset & 0x0f); // align 0x10

	header->totalsize = offset;

	not_yet_implemented ();
}

// should be update fdt (conceal, etc) before calling this function
static void
fdt_load_for_guest (void)
{
	fdt_load_to (get_fdt_base ());
}

INITFUNC ("driver0", fdt_init_driver);
INITFUNC ("bsp0", fdt_load_for_guest);
