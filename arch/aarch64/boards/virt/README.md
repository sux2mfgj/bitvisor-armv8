

##### memory map

in case of `-m 1G`
```
0x00000000
fdt provided by qemu

0x40000000 - base of ram
u-boot and guest (linux, bare metal systems, etc)

0x78000000 - base of bitvisor, and end of ram for the guest
bitvisor
0x80000000 - end of ram
```


