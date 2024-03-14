<font face="ubuntu mono">

# vm 

###### `kernel/vm.c`

## source code studying

### 代码大纲 

1. [tlbinit(void)](###tlbinit)
2. [vminit(void)]()
3. [walk(pagetable_t,uint64,int)](###walk)
4. []()
5. []()
6. []()
7. []()
8. []()
9. []()
10. []()
11. []()
12. []()
13. []()
14. []()
15. []()
16. []()
17. []()

### tlbinit 

Definition:
`void tlbinit( void )`

这个函数用于初始化TLB，实际上就是写入了几个CSR的内容，包括：

- STLBPS 设置STLB固定页大小为4KB 
- ASID 设置地址空间标识符为0（功能尚未知，大概率是为了忽略ASID，但是TLB需要ASID值）
- TLBREHI 设置重填TLB页大小为4KB（功能较复杂，参见手册）

### walk

Definition: 
`pte_t *walk( pagetable_t pagetable, uint64 va, int alloc )`

这个函数用软件方式遍历输入的页表， 将虚拟地址所在的0级页表项返回。如果alloc为1，并且在遍历多级页表的时候如果遇到未分配的页，就会尝试分配新的物理页。
