#ifndef __BUF_H__
#define __BUF_H__

struct buf {
    int valid;   // has data been read from disk?
    volatile int disk;    // does disk "own" buf?
    //int disk;    // does disk "own" buf?
    uint dev;
    uint blockno;
    // todo 增加睡眠锁
    //struct sleeplock lock;
    uint refcnt;
    struct buf *prev; // LRU cache list
    struct buf *next;
    uchar data[BSIZE];
};

#endif //__BUF_H__
