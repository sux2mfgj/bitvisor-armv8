#include <core.h>
#include <core/fdt.h>
#include <core/initfunc.h>
#include <core/putchar.h>

// data register
#define UART_DR 0x00
// flag register
#define UART_FR 0x18
// Integer Baud Rate Register
#define UART_IBRD 0x24
// Fractional Baud Rate Register
#define UART_FBRD 0x28
// line control register
#define UART_LCR_H 0x2c
// control register
#define UART_CR 0x30
// interrupt mask set/clear register
#define UART_IMSC 0x38
// interrupt clear register
#define UART_ICR 0x44

#define UART_FR_CTS (0b1 << 0)
#define UART_FR_DSR (0b1 << 1)
#define UART_FR_DCD (0b1 << 2)
#define UART_FR_BUSY (0b1 << 3)
#define UART_FR_RXFE (0b1 << 4)
#define UART_FR_TXFF (0b1 << 5)
#define UART_FR_TXFE (0b1 << 6)

#define UART_LCR_H_BRK_OFFSET 0
#define UART_LCR_H_BRK_MASK 0b1
#define UART_LCR_H_PEN_OFFSET 1
#define UART_LCR_H_PEN_MASK 0b1
#define UART_LCR_H_PEN (UART_LCR_H_PEN_MASK << UART_LCR_H_PEN_OFFSET)
#define UART_LCR_H_EPS_OFFSET 2
#define UART_LCR_H_EPS_MASK 0b1
#define UART_LCR_H_EPS (UART_LCR_H_EPS_MASK << UART_LCR_H_EPS_OFFSET)
#define UART_LCR_H_STP2_OFFSET 3
#define UART_LCR_H_STP2_MASK 0b1
#define UART_LCR_H_STP2 (UART_LCR_H_STP2_MASK << UART_LCR_H_STP2_OFFSET)
// fifo enable
#define UART_LCR_H_FEN_OFFSET 4
#define UART_LCR_H_FEN_MASK 0b1
#define UART_LCR_H_FEN (UART_LCR_H_FEN_MASK << UART_LCR_H_FEN_OFFSET)
#define UART_LCR_H_WLEN_OFFSET 5
#define UART_LCR_H_WLEN_MASK 0b11
#define UART_LCR_H_WLEN_8 (0b11 << UART_LCR_H_WLEN_OFFSET)
#define UART_LCR_H_WLEN_7 (0b10 << UART_LCR_H_WLEN_OFFSET)
#define UART_LCR_H_WLEN_6 (0b01 << UART_LCR_H_WLEN_OFFSET)
#define UART_LCR_H_WLEN_5 (0b00 << UART_LCR_H_WLEN_OFFSET)
enum uart_wlen {
	UART_WLEN_8,
	UART_WLEN_7,
	UART_WLEN_6,
	UART_WLEN_5,
};
#define UART_LCR_H_SPS_OFFSET 7
#define UART_LCR_H_SPS_MASK 0b1
#define UART_LCR_H_SPS_DIS (0b0 << UART_LCR_H_SPS_OFFSET)
#define UART_LCR_H_SPS_ETH (0b1 << UART_LCR_H_SPS_OFFSET)

#define UART_CR_EN (0b1)
#define UART_CR_TXE (0b1 << 8)
#define UART_CR_RXE (0b1 << 9)

#define UART_IMSC_RXIM (0b1 << 4)
#define UART_IMSC_TXIM (0b1 << 5)
#define UART_IMSC_RTIM (0b1 << 6)

struct uart_dev {
	u64 mmio_base;
};

static struct uart_dev uart_dev;

static u32
uart_read_32 (struct uart_dev *dev, size_t offset)
{
	return *(u32 *)(dev->mmio_base + offset);
}

static void
uart_write_32 (struct uart_dev *dev, size_t offset, u32 value)
{
	*(u32 *)(dev->mmio_base + offset) = value;
}

static void
_uart_putchar (struct uart_dev *dev, unsigned char c)
{
	while (uart_read_32 (dev, UART_FR) & (1 << 5)) {
	}
	uart_write_32 (dev, UART_DR, (u32)c);
}

static void
uart_putchar (unsigned char c)
{
	_uart_putchar (&uart_dev, c);
}

char
_uart_getchar (struct uart_dev *dev)
{
	while (uart_read_32 (dev, UART_FR) & (1 << 4)) {
	}
	return (char)uart_read_32 (dev, UART_DR);
}

char
uart_getchar (void)
{
	return _uart_getchar (&uart_dev);
}

static void
uart_clear_interrupt (struct uart_dev *dev)
{
	uart_write_32 (dev, UART_ICR, 0x7ff);
}

static bool
uart_irq_handler (u32 irq)
{
	char c = _uart_getchar (&uart_dev);
	// echo back
	_uart_putchar (&uart_dev, c);

	// clear rx interrupt
	uart_write_32 (&uart_dev, UART_ICR, (1 << 4));

	return true;
}

static void
uart_disable (struct uart_dev *dev)
{
	uart_write_32 (dev, UART_CR, 0);
	// TODO write 0 to CR
}

static void
uart_enable (struct uart_dev *dev)
{
	uart_write_32 (dev, UART_CR, UART_CR_EN | UART_CR_TXE | UART_CR_RXE);
}

static void
uart_disable_parity (struct uart_dev *dev)
{
	u32 value;

	value = uart_read_32 (dev, UART_LCR_H);
	value &= ~UART_LCR_H_PEN;
	uart_write_32 (dev, UART_LCR_H, value);
}

static void
uart_set_word_length (struct uart_dev *dev, enum uart_wlen len)
{
	u32 value = 0;
	value = uart_read_32 (dev, UART_LCR_H);
	value &= ~UART_LCR_H_WLEN_MASK;
	switch (len) {
	case UART_WLEN_8:
		value |= UART_LCR_H_WLEN_8;
		break;
	case UART_WLEN_7:
		value |= UART_LCR_H_WLEN_7;
		break;
	case UART_WLEN_6:
		value |= UART_LCR_H_WLEN_6;
		break;
	case UART_WLEN_5:
		value |= UART_LCR_H_WLEN_5;
		break;
	}
	value &= ~UART_LCR_H_FEN_MASK;

	uart_write_32 (dev, UART_LCR_H, value);
}

static void
init_uart (void *base)
{
	struct uart_dev *dev = &uart_dev;
	dev->mmio_base = (u64)base;

	uart_disable (dev);
	uart_disable_parity (dev);
	uart_set_word_length (dev, UART_WLEN_8);
	uart_clear_interrupt (dev);

	// enable rx interrupt
	// uart_write_32(UART_IMSC, UART_IMSC_RXIM);

	// irq_register_handler(33, uart_irq_handler);

	uart_enable (dev);
}

static void
uart_fdt_init (struct fdt_node *node)
{
	int res;
	u64 tmp;
	res = fdt_get_reg_value (node, 0, FDT_REG_ADDRESS, &tmp);
	if (res) {
		panic ("invalid reg value of device tree");
	}

	init_uart ((void *)tmp);
	putchar_set_func (uart_putchar, NULL);
}

static struct fdt_driver uart_fdt_driver = {
	.compatible = "arm,pl011",
	.init = uart_fdt_init,
};

FDT_DRIVER (uart, &uart_fdt_driver);

#ifdef CONFIG_UART_EARLY_PUTC

static struct uart_dev uart_early_dev;

static void
uart_early_putc (unsigned char c)
{
	_uart_putchar (&uart_early_dev, c);
}

static void
uart_early_putc_init (void)
{
	struct uart_dev *dev = &uart_early_dev;

	dev->mmio_base = CONFIG_UART_EARLY_PUTC_MMIO_BASE;

	uart_disable (dev);
	uart_disable_parity (dev);
	uart_set_word_length (dev, UART_WLEN_8);
	uart_enable (dev);

	putchar_set_func (uart_early_putc, NULL);
}

INITFUNC ("global0", uart_early_putc_init);
#endif
