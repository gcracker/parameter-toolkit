#ifdef __cplusplus
extern "C" {
#endif
#ifndef TCGEN_H
#define TCGEN_H

#include <glib.h>

/*
 * Type Declarations
 */
typedef guint32 F0type;
typedef guint32 F1type;
typedef guint64 F2type;
typedef guint64 F3type;
typedef guint64 F4type;
typedef guint32 F5type;
typedef guint32 F6type;
typedef guint32 F7type;
typedef guint32 PCtype;

/* 
 * Function declarations
 */
void TCgen_Init();

F0type F0Decode(const int code, F0type val);
F1type F1Decode(const int code, const PCtype pc, F1type val);
F2type F2Decode(const int code, const PCtype pc, F2type val);
F3type F3Decode(const int code, const PCtype pc, F3type val);
F4type F4Decode(const int code, const PCtype pc, F4type val);
F5type F5Decode(const int code, const PCtype pc, F5type val);
F6type F6Decode(const int code, const PCtype pc, F6type val);
F7type F7Decode(const int code, const PCtype pc, F7type val);

unsigned char F0Encode(const F0type val);
unsigned char F1Encode(const PCtype pc, const F1type val);
unsigned char F2Encode(const PCtype pc, const F2type val);
unsigned char F3Encode(const PCtype pc, const F3type val);
unsigned char F4Encode(const PCtype pc, const F4type val);
unsigned char F5Encode(const PCtype pc, const F5type val);
unsigned char F6Encode(const PCtype pc, const F6type val);
unsigned char F7Encode(const PCtype pc, const F7type val);

/* 
 * Convenience Function declarations
 */
#define TCgen_Decode_oper_id(code, val)             F0Decode(code, val)
#define TCgen_Decode_attr(code, pc, val)            F1Decode(code, pc, val)
#define TCgen_Decode_src_ea0(code, pc, val)         F2Decode(code, pc, val)
#define TCgen_Decode_src_ea1(code, pc, val)         F3Decode(code, pc, val)
#define TCgen_Decode_dst_ea0(code, pc, val)         F4Decode(code, pc, val)
#define TCgen_Decode_mem_read_size(code, pc, val)   F5Decode(code, pc, val)
#define TCgen_Decode_mem_write_size(code, pc, val)  F6Decode(code, pc, val)
#define TCgen_Decode_sys(code, pc, val)             F7Decode(code, pc, val)

#define TCgen_Encode_oper_id(val)             F0Encode(val)
#define TCgen_Encode_attr(pc, val)            F1Encode(pc, val)
#define TCgen_Encode_src_ea0(pc, val)         F2Encode(pc, val)
#define TCgen_Encode_src_ea1(pc, val)         F3Encode(pc, val)
#define TCgen_Encode_dst_ea0(pc, val)         F4Encode(pc, val)
#define TCgen_Encode_mem_read_size(pc, val)   F5Encode(pc, val)
#define TCgen_Encode_mem_write_size(pc, val)  F6Encode(pc, val)
#define TCgen_Encode_sys(pc, val)             F7Encode(pc, val)



#endif
#ifdef __cplusplus
}
#endif
