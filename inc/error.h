#ifndef INC_ERROR_H
#define INC_ERROR_H

enum {
    E_UNSPECIFIED = 1,  // 未定义的问题
    E_BAD_PROC,         // PROC不存在或操作非法
    E_INVAL,            // 不合法的参数
    E_NO_MEM,           // 内存不足
    E_NO_FREE_PROC,     // PROC申请数量已达上限
    E_FAULT,            // Memory fault
    E_FILE_NF,          // 文件不存在

    MAXERROR
};

#endif
