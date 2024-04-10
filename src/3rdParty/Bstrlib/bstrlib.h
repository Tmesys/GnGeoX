/*
 * This source file is part of the bstring string library.  This code was
 * written by Paul Hsieh in 2002-2015, and is covered by the BSD open source
 * license and the GPL. Refer to the accompanying documentation for details
 * on usage and license.
 */

/*
 * bstrlib.h
 *
 * This file is the interface for the core bstring functions.
 */

#ifndef BSTRLIB_INCLUDE
#define BSTRLIB_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#if !defined (BSTRLIB_VSNP_OK) && !defined (BSTRLIB_NOVSNP)
# if defined (__TURBOC__) && !defined (__BORLANDC__)
#  define BSTRLIB_NOVSNP
# endif
#endif

#define BSTR_ERR (-1)
#define BSTR_OK (0)
#define BSTR_BS_BUFF_LENGTH_GET (0)

typedef struct tagbstring *bstring;
typedef struct bstrList *bstringList;
typedef const struct tagbstring *const_bstring;
typedef const struct bstrList *const_bstringList;

/* Copy functions */
#define cstr2bstr bfromcstr
extern bstring bfromcstr ( const char * );
extern bstring bfromcstralloc ( int, const char * );
extern bstring blk2bstr ( const void *, int );
extern char *bstr2cstr ( const_bstring, char );
extern int bcstrfree ( char * );
extern bstring bstrcpy ( const_bstring );
extern int bassign ( bstring, const_bstring );
extern int bassignmidstr ( bstring, const_bstring, int, int );
extern int bassigncstr ( bstring, const char * );
extern int bassignblk ( bstring, const void *, int );

/* Destroy function */
extern int bdestroy ( bstring );

/* Space allocation hinting functions */
extern int balloc ( bstring, int );
extern int ballocmin ( bstring, int );

/* Substring extraction */
extern bstring bmidstr ( const_bstring, int, int );

/* Various standard manipulations */
extern int bconcat ( bstring, const_bstring );
extern int bconchar ( bstring, char );
extern int bcatcstr ( bstring, const char * );
extern int bcatblk ( bstring, const void *, int );
extern int binsert ( bstring, int, const_bstring, unsigned char );
extern int binsertch ( bstring, int, int, unsigned char );
extern int breplace ( bstring, int, int, const_bstring, unsigned char );
extern int bdelete ( bstring, int, int );
extern int bsetstr ( bstring, int, const_bstring, unsigned char );
extern int btrunc ( bstring, int );

/* Scan/search functions */
extern int bstricmp ( const_bstring, const_bstring );
extern int bstricmpcstr ( const_bstring, const char * );
extern int bstrnicmp ( const_bstring, const_bstring, int );
extern int biseqcaseless ( const_bstring, const_bstring );
extern int bisstemeqcaselessblk ( const_bstring, const void *, int );
extern int biseq ( const_bstring, const_bstring );
extern int bisstemeqblk ( const_bstring, const void *, int );
extern int biseqcstr ( const_bstring, const char * );
extern int biseqcstrcaseless ( const_bstring, const char * );
extern int bstrcmp ( const_bstring, const_bstring );
extern int bstrcmpcstr ( const_bstring, const char * );
extern int bstrncmp ( const_bstring, const_bstring, int );
extern int binstr ( const_bstring, int, const_bstring );
extern int binstrcstr ( const_bstring, int, const char * );
extern int binstrr ( const_bstring, int, const_bstring );
extern int binstrcaseless ( const_bstring, int, const_bstring );
extern int binstrrcaseless ( const_bstring, int, const_bstring );
extern int bstrchrp ( const_bstring, int, int );
extern int bstrrchrp ( const_bstring, int, int );
#define bstrchr(b,c) bstrchrp ((b), (c), 0)
#define bstrrchr(b,c) bstrrchrp ((b), (c), blength(b)-1)
extern int binchr ( const_bstring, int pos, const_bstring );
extern int binchrr ( const_bstring, int pos, const_bstring );
extern int bninchr ( const_bstring, int pos, const_bstring );
extern int bninchrr ( const_bstring, int pos, const_bstring );
extern int bfindreplace ( bstring, const_bstring, const_bstring, int );
extern int bfindreplacecstr ( bstring, const char *, const char *, int );
extern int bfindreplacecaseless ( bstring, const_bstring, const_bstring, int );

/* List of string container functions */
struct bstrList
{
    int qty, mlen;
    bstring *entry;
};
extern struct bstrList *bstrListCreate ( void );
extern int bstrListDestroy ( struct bstrList *sl );
extern int bstrListAlloc ( struct bstrList *sl, int msz );
extern int bstrListAllocMin ( struct bstrList *sl, int msz );

/* String split and join functions */
extern struct bstrList *bsplit ( const_bstring str, unsigned char splitChar );
extern struct bstrList *bsplits ( const_bstring str, const_bstring splitStr );
extern struct bstrList *bsplitstr ( const_bstring str, const_bstring splitStr );
extern bstring bjoin ( const struct bstrList *bl, const_bstring sep );
extern int bsplitcb ( const_bstring str, unsigned char splitChar, int pos,
                      int ( * cb ) ( void *parm, int ofs, int len ), void *parm );
extern int bsplitscb ( const_bstring str, const_bstring splitStr, int pos,
                       int ( * cb ) ( void *parm, int ofs, int len ), void *parm );
extern int bsplitstrcb ( const_bstring str, const_bstring splitStr, int pos,
                         int ( * cb ) ( void *parm, int ofs, int len ), void *parm );

/* Miscellaneous functions */
extern int bpattern ( bstring b, int len );
extern int btoupper ( bstring b );
extern int btolower ( bstring b );
extern int bltrimws ( bstring b );
extern int brtrimws ( bstring b );
extern int btrimws ( bstring b );

#if !defined (BSTRLIB_NOVSNP)
extern bstring bformat ( const char *fmt, ... );
extern int bformata ( bstring b, const char *fmt, ... );
extern int bassignformat ( bstring b, const char *fmt, ... );
extern int bvcformata ( bstring b, int count, const char *fmt, va_list arglist );

#define bvformata(ret, b, fmt, lastarg) { \
        bstring bstrtmp_b = (b); \
        const char * bstrtmp_fmt = (fmt); \
        int bstrtmp_r = BSTR_ERR, bstrtmp_sz = 16; \
        for (;;) { \
            va_list bstrtmp_arglist; \
            va_start (bstrtmp_arglist, lastarg); \
            bstrtmp_r = bvcformata (bstrtmp_b, bstrtmp_sz, bstrtmp_fmt, bstrtmp_arglist); \
            va_end (bstrtmp_arglist); \
            if (bstrtmp_r >= 0) { /* Everything went ok */ \
                bstrtmp_r = BSTR_OK; \
                break; \
            } else if (-bstrtmp_r <= bstrtmp_sz) { /* A real error? */ \
                bstrtmp_r = BSTR_ERR; \
                break; \
            } \
            bstrtmp_sz = -bstrtmp_r; /* Doubled or target size */ \
        } \
        ret = bstrtmp_r; \
    }

#endif

typedef int ( * bNgetc ) ( void *parm );
typedef size_t ( * bNread ) ( void *buff, size_t elsize, size_t nelem, void *parm );

/* Input functions */
extern bstring bgets ( bNgetc getcPtr, void *parm, char terminator );
extern bstring bread ( bNread readPtr, void *parm );
extern int bgetsa ( bstring b, bNgetc getcPtr, void *parm, char terminator );
extern int bassigngets ( bstring b, bNgetc getcPtr, void *parm, char terminator );
extern int breada ( bstring b, bNread readPtr, void *parm );

/* Stream functions */
extern struct bStream *bsopen ( bNread readPtr, void *parm );
extern void *bsclose ( struct bStream *s );
extern int bsbufflength ( struct bStream *s, int sz );
extern int bsreadln ( bstring b, struct bStream *s, char terminator );
extern int bsreadlns ( bstring r, struct bStream *s, const_bstring term );
extern int bsread ( bstring b, struct bStream *s, int n );
extern int bsreadlna ( bstring b, struct bStream *s, char terminator );
extern int bsreadlnsa ( bstring r, struct bStream *s, const_bstring term );
extern int bsreada ( bstring b, struct bStream *s, int n );
extern int bsunread ( struct bStream *s, const_bstring b );
extern int bspeek ( bstring r, const struct bStream *s );
extern int bssplitscb ( struct bStream *s, const_bstring splitStr,
                        int ( * cb ) ( void *parm, int ofs, const_bstring entry ), void *parm );
extern int bssplitstrcb ( struct bStream *s, const_bstring splitStr,
                          int ( * cb ) ( void *parm, int ofs, const_bstring entry ), void *parm );
extern int bseof ( const struct bStream *s );

struct tagbstring
{
    int mlen;
    int slen;
    unsigned char *data;
};

/* Accessor macros */
#define blengthe(b, e)      (((b) == (void *)0 || (b)->slen < 0) ? (int)(e) : ((b)->slen))
#define blength(b)          (blengthe ((b), 0))
#define bdataofse(b, o, e)  (((b) == (void *)0 || (b)->data == (void*)0) ? (char *)(e) : ((char *)(b)->data) + (o))
#define bdataofs(b, o)      (bdataofse ((b), (o), (void *)0))
#define bdatae(b, e)        (bdataofse (b, 0, e))
#define bdata(b)            (bdataofs (b, 0))
#define bchare(b, p, e)     ((((unsigned)(p)) < (unsigned)blength(b)) ? ((b)->data[(p)]) : (e))
#define bchar(b, p)         bchare ((b), (p), '\0')

/* Static constant string initialization macro */
#define bsStaticMlen(q,m)   {(m), (int) sizeof(q)-1, (unsigned char *) ("" q "")}
#if defined(_MSC_VER)
# define bsStatic(q)        bsStaticMlen(q,-32)
#endif
#ifndef bsStatic
# define bsStatic(q)        bsStaticMlen(q,-__LINE__)
#endif

/* Static constant block parameter pair */
#define bsStaticBlkParms(q) ((void *)("" q "")), ((int) sizeof(q)-1)

/* Reference building macros */
#define cstr2tbstr btfromcstr
#define btfromcstr(t,s) {                                            \
        (t).data = (unsigned char *) (s);                                \
        (t).slen = ((t).data) ? ((int) (strlen) ((char *)(t).data)) : 0; \
        (t).mlen = -1;                                                   \
    }
#define blk2tbstr(t,s,l) {            \
        (t).data = (unsigned char *) (s); \
        (t).slen = l;                     \
        (t).mlen = -1;                    \
    }
#define btfromblk(t,s,l) blk2tbstr(t,s,l)
#define bmid2tbstr(t,b,p,l) {                                                \
        const_bstring bstrtmp_s = (b);                                           \
        if (bstrtmp_s && bstrtmp_s->data && bstrtmp_s->slen >= 0) {              \
            int bstrtmp_left = (p);                                              \
            int bstrtmp_len  = (l);                                              \
            if (bstrtmp_left < 0) {                                              \
                bstrtmp_len += bstrtmp_left;                                     \
                bstrtmp_left = 0;                                                \
            }                                                                    \
            if (bstrtmp_len > bstrtmp_s->slen - bstrtmp_left)                    \
                bstrtmp_len = bstrtmp_s->slen - bstrtmp_left;                    \
            if (bstrtmp_len <= 0) {                                              \
                (t).data = (unsigned char *)"";                                  \
                (t).slen = 0;                                                    \
            } else {                                                             \
                (t).data = bstrtmp_s->data + bstrtmp_left;                       \
                (t).slen = bstrtmp_len;                                          \
            }                                                                    \
        } else {                                                                 \
            (t).data = (unsigned char *)"";                                      \
            (t).slen = 0;                                                        \
        }                                                                        \
        (t).mlen = -__LINE__;                                                    \
    }
#define btfromblkltrimws(t,s,l) {                                            \
        int bstrtmp_idx = 0, bstrtmp_len = (l);                                  \
        unsigned char * bstrtmp_s = (s);                                         \
        if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
            for (; bstrtmp_idx < bstrtmp_len; bstrtmp_idx++) {                   \
                if (!isspace (bstrtmp_s[bstrtmp_idx])) break;                    \
            }                                                                    \
        }                                                                        \
        (t).data = bstrtmp_s + bstrtmp_idx;                                      \
        (t).slen = bstrtmp_len - bstrtmp_idx;                                    \
        (t).mlen = -__LINE__;                                                    \
    }
#define btfromblkrtrimws(t,s,l) {                                            \
        int bstrtmp_len = (l) - 1;                                               \
        unsigned char * bstrtmp_s = (s);                                         \
        if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
            for (; bstrtmp_len >= 0; bstrtmp_len--) {                            \
                if (!isspace (bstrtmp_s[bstrtmp_len])) break;                    \
            }                                                                    \
        }                                                                        \
        (t).data = bstrtmp_s;                                                    \
        (t).slen = bstrtmp_len + 1;                                              \
        (t).mlen = -__LINE__;                                                    \
    }
#define btfromblktrimws(t,s,l) {                                             \
        int bstrtmp_idx = 0, bstrtmp_len = (l) - 1;                              \
        unsigned char * bstrtmp_s = (s);                                         \
        if (bstrtmp_s && bstrtmp_len >= 0) {                                     \
            for (; bstrtmp_idx <= bstrtmp_len; bstrtmp_idx++) {                  \
                if (!isspace (bstrtmp_s[bstrtmp_idx])) break;                    \
            }                                                                    \
            for (; bstrtmp_len >= bstrtmp_idx; bstrtmp_len--) {                  \
                if (!isspace (bstrtmp_s[bstrtmp_len])) break;                    \
            }                                                                    \
        }                                                                        \
        (t).data = bstrtmp_s + bstrtmp_idx;                                      \
        (t).slen = bstrtmp_len + 1 - bstrtmp_idx;                                \
        (t).mlen = -__LINE__;                                                    \
    }

/* Write protection macros */
#define bwriteprotect(t)     { if ((t).mlen >=  0) (t).mlen = -1; }
#define bwriteallow(t)       { if ((t).mlen == -1) (t).mlen = (t).slen + ((t).slen == 0); }
#define biswriteprotected(t) ((t).mlen <= 0)

extern unsigned int bcrc32 ( bstring );
extern unsigned int bhash ( bstring );

/* Compile time version of bhash */
#define H1(s,i,x)   (x*65599u+(u8)s[(i)<sizeof(s)?sizeof(s)-1-(i):sizeof(s)])
#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
#define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))

#define HASH(s)    ((u32)(H256(s,0,0)^(H256(s,0,0)>>16)))


#ifdef __cplusplus
}
#endif

#endif
