#include "types.h"
#include "riscv.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "virtio.h"
#include "fs.h"
#include "buf.h"
#include "defs.h"

#define R(r) ((volatile uint32*)(VIRTIO0+(r)))

static struct disk{
    struct virtq_desc* desc;
    struct virtq_avail* avail; // 操作系统通知物理设备
    struct virtq_used* used; // 物理设备通知操作系统

    char free[NUM]; // is a descriptor free? 0 表示正在使用
    uint16 used_idx;

    // 共8个描述符，跟踪每个描述符状态信息
    // 表示读写块成功的结构
    struct{
        struct buf* b;
        char status;
    }info[NUM];

    struct virtio_blk_req ops[NUM];
    // todo 磁盘增加锁
    //struct spinlock vdisk_lock;
}memdisk;

/*
static int alloc_desc();
static void free_desc(int i);
static int alloc3_desc(int* idx);
static void free_chain(int i);
void virtio_disk_init();
void virtio_disk_rw(struct buf* b,int write);
void virtio_disk_intr();
void virtio_disk_test();
*/
// 分配1个空的描述符
static int alloc_desc()
{
    for(int i=0; i<NUM; i++)
    {
        if(memdisk.free[i])
        {
            memdisk.free[i] = 0;
            return i;
        }
    }
    return -1;
}

// 释放指定描述符
static void free_desc(int i)
{
    if(i >= NUM)
        panic("free_desc 1");
    if(memdisk.free[i])
        panic("free_desc 2");

    memdisk.desc[i].addr = 0;
    memdisk.desc[i].len = 0;
    memdisk.desc[i].flags = 0;
    memdisk.desc[i].next = 0;
    memdisk.free[i] = 1; // 设置为未使用状态
}

// 分配3个描述符
static int alloc3_desc(int* idx)
{
    for(int i=0; i<3; i++)
    {
        idx[i] = alloc_desc();
        if(idx[i] < 0) // 发生错误，则释放
        {
            for(int j=0; j<i; j++)
            {
                free_desc(idx[j]);
                return -1;
            }
        }
    }
    return 0;
}

// 释放特定描述符链
static void free_chain(int i)
{
    while(1)
    {
        int flag = memdisk.desc[i].flags;
        int nxt = memdisk.desc[i].next;
        free_desc(i);

        if(flag & VRING_DESC_F_NEXT)
            i=nxt;
        else
            break;
    }
}

// 初始化虚拟磁盘设备
void virtio_disk_init()
{
    /*
    1：异常排除
    2：告知设备，已经被发现，并且找到了可用的驱动 
    3：和设备协商一些特性 如果协商失败，那么需要禁止安装驱动。 
    4：分配3个数据结构，并告知驱动
    5：告知设备队列大小
    6：告知设备协商完毕 
    */

    // todo
    //initlock(&disk.vdisk_lock, "virtio_disk");

    uint32 status = 0;
    if(*R(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        *R(VIRTIO_MMIO_VERSION) != 2 ||
        *R(VIRTIO_MMIO_DEVICE_ID) != 2 ||
        *R(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551){
        panic("could not find virtio disk");
    }

    // reset device
    *R(VIRTIO_MMIO_STATUS) = status;
    
    // set ACKNOWLEDGE status bit
    status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    *R(VIRTIO_MMIO_STATUS) = status;

    // set DRIVER status bit
    status |= VIRTIO_CONFIG_S_DRIVER;
    *R(VIRTIO_MMIO_STATUS) = status;

    // negotiate features
    // 去除这些特性
    uint64 features = *R(VIRTIO_MMIO_DEVICE_FEATURES);
    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
    features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
    *R(VIRTIO_MMIO_DRIVER_FEATURES) = features;

    // tell device that feature negotiation is complete
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    *R(VIRTIO_MMIO_STATUS) = status;

    // re-read status to ensure FEATURES_OK is set.
    status = *R(VIRTIO_MMIO_STATUS);
    if(!(status & VIRTIO_CONFIG_S_FEATURES_OK))
        panic("virtio disk FEATURES_OK unset");

    // initialize queue 0
    *R(VIRTIO_MMIO_QUEUE_SEL) = 0;

    // ensure queue 0 is not in use
    if(*R(VIRTIO_MMIO_QUEUE_READY))
        panic("virtio disk sould not be ready");

    // check maximum queue size
    uint32 max = *R(VIRTIO_MMIO_QUEUE_NUM_MAX);
    if(max == 0)
        panic("virtio disk has no queue 0");
    if(max < NUM)
        panic("virtio disk max queue too short");

    // allocate and zero queue memory
    memdisk.desc = kalloc();
    memdisk.avail = kalloc();
    memdisk.used = kalloc();
    if(!memdisk.desc || !memdisk.avail || !memdisk.used)
        panic("virtio disk kalloc");
    memset(memdisk.desc,0,PGSIZE);
    memset(memdisk.avail,0,PGSIZE);
    memset(memdisk.used,0,PGSIZE);

    // set queue size
    *R(VIRTIO_MMIO_QUEUE_NUM) = NUM;
    
    // write physical addresses.
    *R(VIRTIO_MMIO_QUEUE_DESC_LOW) = (uint64)memdisk.desc;
    *R(VIRTIO_MMIO_QUEUE_DESC_HIGH) = (uint64)memdisk.desc >> 32;
    *R(VIRTIO_MMIO_DRIVER_DESC_LOW) = (uint64)memdisk.avail;
    *R(VIRTIO_MMIO_DRIVER_DESC_HIGH) = (uint64)memdisk.avail >> 32;
    *R(VIRTIO_MMIO_DEVICE_DESC_LOW) = (uint64)memdisk.used;
    *R(VIRTIO_MMIO_DEVICE_DESC_HIGH) = (uint64)memdisk.used >> 32;

    // queue is ready
    *R(VIRTIO_MMIO_QUEUE_READY) = 0x1;

    // all NUM descriptors start out unused.
    for(int i=0; i<NUM; i++)
        memdisk.free[i] = 1;

    // tell device we're completely ready
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    *R(VIRTIO_MMIO_STATUS) = status;

    // plic.c and trap.c arrange for interrupts from VIRTIO0_IRQ.
}

// 读写磁盘块
void virtio_disk_rw(struct buf* b,int write)
{
    uint64 sector = b->blockno*(BSIZE/512);

    //three descriptors: type/reserved/sector;data;1byte status result

    int idx[3];
    while(1)
    {
        if(alloc3_desc(idx) == 0)
        {
            break;
        }
        // todo 若请求不成功，需要睡眠当前线程并等待
    }

    // 格式化3个描述符，即赋值
    struct virtio_blk_req* buf0 = &memdisk.ops[idx[0]];
    if(write)
        buf0->type = VIRTIO_BLK_T_OUT;
    else
        buf0->type = VIRTIO_BLK_T_IN;
    buf0->reserved = 0;
    buf0->sector = sector; // 扇区号

    memdisk.desc[idx[0]].addr = (uint64)buf0;
    memdisk.desc[idx[0]].len = sizeof(struct virtio_blk_req);
    memdisk.desc[idx[0]].flags = VRING_DESC_F_NEXT;
    memdisk.desc[idx[0]].next = idx[1];

    memdisk.desc[idx[1]].addr = (uint64)b->data;
    memdisk.desc[idx[1]].len = BSIZE;
    if(write)
        memdisk.desc[idx[1]].flags = 0; // device reads b->data
    else
        memdisk.desc[idx[1]].flags = VRING_DESC_F_WRITE; // device writes b->data
    memdisk.desc[idx[1]].flags |= VRING_DESC_F_NEXT;

    memdisk.desc[idx[1]].next = idx[2];

    memdisk.info[idx[0]].status = 0xff; // device writes 0 on success
    memdisk.desc[idx[2]].addr = (uint64) &memdisk.info[idx[0]].status;
    memdisk.desc[idx[2]].len = 1;
    memdisk.desc[idx[2]].flags = VRING_DESC_F_WRITE; // device writes the status
    memdisk.desc[idx[2]].next = 0;

    // record struct buf for virtio_memdisk_intr().
    b->disk = 1; // 在中断中将次值置为0,表示处理完毕
    memdisk.info[idx[0]].b = b;

    // tell the device the first index in our chain of descriptors.
    memdisk.avail->ring[memdisk.avail->idx % NUM] = idx[0];

    __sync_synchronize();

    // tell the device another avail ring entry is available.
    memdisk.avail->idx += 1; // not % NUM ...

    __sync_synchronize();

    *R(VIRTIO_MMIO_QUEUE_NOTIFY) = 0; // value is queue number

    //printf("VA1\n");
    // Wait for virtio_memdisk_intr() to say request has finished.
    while(b->disk== 1) {
        // todo 增加睡眠。等待中断处理完成后，将次值设置为0
        //sleep(b, &memdisk.vmemdisk_lock);
        ;
    }
    //printf("VA2\n");
    memdisk.info[idx[0]].b = 0;
    free_chain(idx[0]);

    //release(&memdisk.vmemdisk_lock);
}

// 磁盘中断处理函数
void virtio_disk_intr()
{
    // todo 增加锁

    // 告知虚拟磁盘设备我们可以相应下一个中断(1次可以完成多个)
    *R(VIRTIO_MMIO_INTERRUPT_ACK) = *R(VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3;

    __sync_synchronize();

    // 当设备处理完一个请求时，设备自动将memdisk.used->idx加1
    while(memdisk.used_idx != memdisk.used->idx)
    {
        __sync_synchronize();
        int id = memdisk.used->ring[memdisk.used_idx % NUM].id;

        if(memdisk.info[id].status !=0 )
            panic("virtio_disk_intr status");

        struct buf* b = memdisk.info[id].b;
        b->disk = 0; // disk is done with buf

        // todo 唤醒

        memdisk.used_idx += 1;
    }
}

// 测试磁盘读写
void virtio_disk_test()
{
    struct buf b_test[3];
    for(int i=0; i<3; i++)
    {
        b_test[i].dev = 1;
        b_test[i].blockno = i+1; //写入的磁盘号
        for(int j=0; j<BSIZE; j++)
            b_test[i].data[j] = 0x33+i;
    }

    printf("b_test write start...\n");
    virtio_disk_rw(&b_test[0],1);
    virtio_disk_rw(&b_test[1],1);
    virtio_disk_rw(&b_test[2],1);
    printf("b_test write end...\n");
}
