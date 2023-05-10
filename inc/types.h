#ifndef INC_TYPES_H
#define INC_TYPES_H

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
typedef uint32_t physaddr_t;

typedef uint32_t pte_t;
typedef uint32_t pde_t;

typedef int32_t fid_t;

typedef uint32_t ppn_t;

typedef uint32_t size_t;
typedef int32_t ssize_t;

// off_t is used for file offsets and lengths.
typedef int32_t off_t;

// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)               \
    ({                                \
        uint32_t __a = (uint32_t)(a); \
        (typeof(a))(__a - __a % (n)); \
    })
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)                                         \
    ({                                                        \
        uint32_t __n = (uint32_t)(n);                         \
        (typeof(a))(ROUNDDOWN((uint32_t)(a) + __n - 1, __n)); \
    })

#endif /* !OS_INC_TYPES_H */
