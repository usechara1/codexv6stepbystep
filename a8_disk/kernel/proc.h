#ifndef __PROC_H__
#define __PROC_H__

struct cpu{
    int noff; //嵌套深度 Depth of push_off() nesting.
    int intena; //中断关闭前的中断状态 interrup enable?
};

#endif //__PROC_H__
