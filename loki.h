#ifndef loki_h
#define loki_h


#ifndef LK_NS_BEGIN
# ifdef __cplusplus
#   define LK_NS_BEGIN extern "C" {
#   define LK_NS_END   }
# else
#   define LK_NS_BEGIN
#   define LK_NS_END
# endif
#endif /* LK_NS_BEGIN */

#ifdef LK_STATIC_API
# ifndef LOKI_IMPLEMENTATION
#  define LOKI_IMPLEMENTATION
# endif
# if __GNUC__
#   define LK_API static __attribute((unused))
# else
#   define LK_API static
# endif
#endif

#if !defined(LK_API) && defined(_WIN32)
# if defined(LOKI_IMPLEMENTATION) || defined(LOKI_MODULE)
#   define LK_API __declspec(dllexport)
# else
#   define LK_API __declspec(dllimport)
# endif
#endif

#ifndef LK_API
# define LK_API extern
#endif

#ifndef LKMOD_API
# define LKMOD_API LK_API
#endif

#ifndef LK_PATH
# ifdef _WIN32
#   define LK_PATH "!\\services\\?.dll;" "!\\..\\services\\?.dll;" "!\\?.dll"
# else
#   define LK_PATH "services/?.so;" "../services/?.so;" "./?.so"
# endif
#endif /* LK_PATH */

#ifndef LK_PREFIX
# define LK_PREFIX "loki_service_"
#endif /* LK_PREFIX */

#define LK_OK      (0)
#define LK_WEAK    (1)
#define LK_ERR     (-1)
#define LK_TIMEOUT (-2)


#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>


LK_NS_BEGIN

typedef struct lk_State   lk_State;
typedef struct lk_Service lk_Service;
typedef struct lk_Slot    lk_Slot;

typedef struct lk_Signal  lk_Signal;
typedef struct lk_Reg     lk_Reg;

typedef int lk_ServiceHandler (lk_State *S);
typedef int lk_SignalHandler  (lk_State *S, void *ud, lk_Slot *slot, lk_Signal *sig);
typedef int lk_Handler        (lk_State *S, void *ud);


/* global routines */

LK_API lk_State *lk_newstate (const char *name);

LK_API void lk_waitclose (lk_State *S);

LK_API void lk_setthreads (lk_State *S, int threads);
LK_API void lk_setpath    (lk_State *S, const char *path);

LK_API int lk_start (lk_State *S);

LK_API int lk_pcall   (lk_State *S, lk_Handler *h, void *ud);
LK_API int lk_discard (lk_State *S);

LK_API int lk_addcleanup (lk_State *S, lk_Handler *h, void *ud);

LK_API char *lk_getconfig (lk_State *S, const char *key);
LK_API void  lk_setconfig (lk_State *S, const char *key, const char *value);

LK_API void lk_log(lk_State *S, const char *fmt, ...);
LK_API void lk_vlog(lk_State *S, const char *fmt, va_list l);

#define lk_str_(str) # str
#define lk_str(str) lk_str_(str)
#define lk_loc(str) __FILE__ ":" lk_str(__LINE__) ": " str


/* service routines */

LK_API lk_Service *lk_self (lk_State *S);

LK_API void lk_close (lk_State *S);

LK_API void lk_preload (lk_State *S, const char *name, lk_ServiceHandler *h);

LK_API lk_Service *lk_require  (lk_State *S, const char *name);
LK_API lk_Service *lk_requiref (lk_State *S, const char *name, lk_ServiceHandler *h);

LK_API void *lk_data    (lk_Service *svr);
LK_API void  lk_setdata (lk_Service *svr, void *data);

LK_API lk_SignalHandler *lk_refactor (lk_Service *svr, void **pud);
LK_API void  lk_setrefactor (lk_Service *svr, lk_SignalHandler *h, void *ud);


/* message routines */

#define LK_SIGNAL { NULL, NULL, 0, 0, 0, 0 }

LK_API lk_Slot *lk_newslot (lk_State *S, const char *name, lk_SignalHandler *h, void *ud);
LK_API lk_Slot *lk_slot    (lk_State *S, const char *name);

LK_API int lk_emit       (lk_Slot *slot, const lk_Signal *sig);
LK_API int lk_emitdata   (lk_Slot *slot, unsigned type, unsigned session, const void *data, size_t size);
LK_API int lk_emitstring (lk_Slot *slot, unsigned type, unsigned session, const char *s);

LK_API lk_Slot *lk_newpoll (lk_State *S, const char *name, lk_SignalHandler *h, void *ud);
LK_API int      lk_wait    (lk_Slot *slot, lk_Signal *sig, int waitms);

LK_API const char *lk_name    (lk_Slot *slot);
LK_API lk_Service *lk_service (lk_Slot *slot);
LK_API lk_State   *lk_state   (lk_Slot *slot);

LK_API lk_SignalHandler *lk_slothandler (lk_Slot *slot, void **pud);
LK_API void lk_setslothandler (lk_Slot *slot, lk_SignalHandler *h, void *ud);

LK_API int lk_register (lk_State *S, const lk_Reg *slots, void *ud);


/* memory management */

LK_API void *lk_malloc  (lk_State *S, size_t size);
LK_API void *lk_realloc (lk_State *S, void *ptr, size_t size);
LK_API void  lk_free    (lk_State *S, void *ptr);
LK_API char *lk_strdup  (lk_State *S, const char *s);
LK_API char *lk_strncpy (char *dst, size_t n, const char *s);


/* buffer routines */

typedef struct lk_Buffer lk_Buffer;

#define lk_buffer(B)      ((B)->buff)
#define lk_buffsize(B)    ((B)->size)
#define lk_resetbuffer(B) ((B)->size = 0)
#define lk_addchar(B,ch)  (*lk_prepbuffsize((B), 1) = (ch), ++(B)->size)
#define lk_addstring(B,s) lk_addlstring((B),(s),strlen(s))

LK_API void lk_initbuffer (lk_State *S, lk_Buffer *b);
LK_API void lk_freebuffer (lk_Buffer *b);

LK_API char *lk_prepbuffsize (lk_Buffer *B, size_t len);

LK_API int lk_addlstring  (lk_Buffer *B, const char *s, size_t len);
LK_API int lk_addvfstring (lk_Buffer *B, const char *fmt, va_list l);
LK_API int lk_addfstring  (lk_Buffer *B, const char *fmt, ...);

LK_API void lk_replacebuffer (lk_Buffer *B, char origch, char newch);

LK_API const char *lk_buffresult (lk_Buffer *B);


/* hash table routines */

typedef struct lk_Entry lk_Entry;
typedef struct lk_Table lk_Table;

LK_API void lk_inittable (lk_State *S, lk_Table *t);
LK_API void lk_freetable (lk_Table *t);

LK_API size_t lk_resizetable (lk_Table *t, size_t len);

LK_API lk_Entry *lk_getentry (lk_Table *t, const char *key);
LK_API lk_Entry *lk_setentry (lk_Table *t, const char *key);
LK_API void      lk_delentry (lk_Table *t, lk_Entry *e);

LK_API int lk_nextentry (lk_Table *t, lk_Entry **pentry);


/* public structures */

#define LK_BUFFERSIZE 1024

struct lk_Signal {
    lk_Service *src;
    void *data;
    unsigned copy : 1;
    unsigned type : 7;
    unsigned size : 24;
    unsigned session;
};

struct lk_Reg {
    const char *name;
    lk_SignalHandler *handler;
};

struct lk_Buffer {
    size_t size;
    size_t capacity;
    lk_State *S;
    char *buff;
    char init_buff[LK_BUFFERSIZE];
};

struct lk_Entry {
    int next;
    unsigned hash;
    const char *key;
    void *value;
};

struct lk_Table {
    size_t   size;
    size_t   lastfree;
    lk_State *S;
    lk_Entry *hash;
};


LK_NS_END

#endif /* loki_h */


#ifndef lk_thread_h
#define lk_thread_h

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
# include <Windows.h>

typedef HMODULE           lk_Module;
typedef DWORD             lk_TlsKey;
typedef CRITICAL_SECTION  lk_Lock;
typedef HANDLE            lk_Event;
typedef HANDLE            lk_Thread;
typedef volatile LONG     lk_Atomic;

#define lk_loadlib(name)  LoadLibraryA(name)
#define lk_freelib(mod)   FreeLibrary(mod)
#define lk_getaddr(mod, name) GetProcAddress(mod, name)

#define lk_inittls(key)   ((*(key) = TlsAlloc()) != TLS_OUT_OF_INDEXES)
#define lk_freetls(key)   TlsFree(key)
#define lk_gettls(key)    TlsGetValue(key)
#define lk_settls(key, p) TlsSetValue((key),(p))

#define lk_initlock(lock) (InitializeCriticalSection(lock), 1)
#define lk_freelock(lock) DeleteCriticalSection(&(lock))
#define lk_trylock(lock)  TryEnterCriticalSection(&(lock))
#define lk_lock(lock)     EnterCriticalSection(&(lock))
#define lk_unlock(lock)   LeaveCriticalSection(&(lock))

#define lk_inc(atomic)    InterlockedIncrement(&(atomic))
#define lk_finc(atomic)   InterlockedExchangeAdd(&(atomic), 1)
#define lk_dec(atomic)    InterlockedDecrement(&(atomic))
#define lk_fdec(atomic)   InterlockedExchangeSubtract(&(atomic), 1)
#define lk_add(atomic,n)  InterlockedAdd(&(atomic), (n))
#define lk_sub(atomic,n)  InterlockedAdd(&(atomic), -(n))

#define lk_initevent(evt) \
    ((*(evt)=CreateEvent(NULL,FALSE,FALSE,NULL))!=NULL)
#define lk_freeevent(evt) CloseHandle(evt)
#define lk_signal(evt)    SetEvent(evt)

#define lk_initthread(t,f,ud) \
    ((*(t)=CreateThread(NULL,0,(f),(ud),0,NULL))!=NULL)
#define lk_waitthread(t)  \
    (WaitForSingleObject((t), INFINITE),(void)CloseHandle(t))
#define lk_freethread(t)  CloseHandle(t)

#else /* POSIX systems */

#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/time.h>

typedef void             *lk_Module;
typedef pthread_key_t     lk_TlsKey;
typedef pthread_mutex_t   lk_Lock;
typedef pthread_cond_t    lk_Event;
typedef pthread_t         lk_Thread;
typedef volatile unsigned lk_Atomic;

#define lk_loadlib(name)  dlopen((name), RTLD_NOW|RTLD_GLOBAL)
#define lk_freelib(mod)   dlclose(mod)
#define lk_getaddr(mod, name) dlsym(mod, name)

#define lk_inittls(key)   (pthread_key_create((key), NULL) == 0)
#define lk_freetls(key)   pthread_key_delete(key)
#define lk_gettls(key)    pthread_getspecific(key)
#define lk_settls(key, p) pthread_setspecific((key), (p))

#define lk_initlock(lock) (pthread_mutex_init(lock, NULL) == 0)
#define lk_freelock(lock) pthread_mutex_destroy(&(lock))
#define lk_trylock(lock)  pthread_mutex_trylock(&(lock))
#define lk_lock(lock)     pthread_mutex_lock(&(lock))
#define lk_unlock(lock)   pthread_mutex_unlock(&(lock))

#define lk_inc(atomic)    __sync_add_and_fetch(&(atomic), 1)
#define lk_finc(atomic)   __sync_fetch_and_add(&(atomic), 1)
#define lk_dec(atomic)    __sync_sub_and_fetch(&(atomic), 1)
#define lk_fdec(atomic)   __sync_fetch_and_sub(&(atomic), 1)
#define lk_add(atomic,n)  __sync_add_and_fetch(&(atomic), n)
#define lk_sub(atomic,n)  __sync_sub_and_fetch(&(atomic), n)

#define lk_initevent(evt) (pthread_cond_init((evt), NULL) == 0)
#define lk_freeevent(evt) pthread_cond_destroy(&(evt))
#define lk_signal(evt)    pthread_cond_signal(&(evt))

#define lk_initthread(t,f,ud) (pthread_create((t),NULL,(f),(ud)) == 0)
#define lk_waitthread(t)  pthread_join((t),NULL)
#define lk_freethread(t)  pthread_cancel(t)

#endif

#endif /* lk_thread_h */


#ifndef lk_queue_h
#define lk_queue_h

#define lkQ_entry(T) T *next
#define lkQ_type(T)  struct { T *first; T *last; }

#define lkQ_init(h)  ((h)->first = (h)->last = NULL)
#define lkQ_empty(h) ((h)->first == NULL)

#define lkQ_front(h) ((h)->first)
#define lkQ_back(h)  ((h)->last)

#define lkQ_head(h)  \
    (lkQ_empty(h) ? NULL : ((h)->last->next = NULL, (h)->first))

#define lkQ_enqueue(h, n)              do { \
    if (lkQ_empty(h))                       \
        (h)->first = (h)->last = (n);       \
    else {                                  \
        (h)->last->next = n;                \
        (h)->last = n; }                  } while (0)

#define lkQ_dequeue(h, n)              do { \
    (n) = (h)->first;                       \
    if ((n) == (h)->last)                   \
        (h)->first = (h)->last = NULL;      \
    else                                    \
        (h)->first = (h)->first->next;    } while (0)

#define lkQ_merge(h1, h2)              do { \
    if (!lkQ_empty(h2)) {                   \
        if (lkQ_empty(h1))                  \
            (h1)->first = (h2)->first;      \
        else                                \
            (h1)->last->next = (h2)->first; \
        (h1)->last = (h2)->last; }        } while (0)

#endif /* lk_queue_h */


#ifndef lk_context_h
#define lk_context_h

# if defined(__cplusplus) && !defined(LK_USE_LONGJMP)
#   define lk_throw(S,c) throw(c)
#   define lk_try(S,c,a) try { a; } catch(...) \
                         { if ((c)->status == 0) (c)->status = LK_ERR; }
#   define lk_JmpBuf     int  /* dummy variable */

# elif _WIN32 /* ISO C handling with long jumps */
#   define lk_throw(S,c) longjmp((c)->b, 1)
#   define lk_try(S,c,a) if (setjmp((c)->b) == 0) { a; }
#   define lk_JmpBuf     jmp_buf

# else /* in POSIX, try _longjmp/_setjmp (more efficient) */
#   define lk_throw(L,c) _longjmp((c)->b, 1)
#   define lk_try(L,c,a) if (_setjmp((c)->b) == 0) { a; }
#   define lk_JmpBuf     jmp_buf
# endif

LK_NS_BEGIN

typedef struct lk_Cleanup {
    lkQ_entry(struct lk_Cleanup);
    lk_Handler *h;
    void *ud;
} lk_Cleanup;

typedef struct lk_Context {
    struct lk_Context *prev;
    lk_State *S;
    lk_Service *current;
    lk_Cleanup *cleanups;
    lk_JmpBuf b;
    volatile int status; /* error code */
} lk_Context;

LK_API void lk_pushcontext (lk_State *S, lk_Context *ctx, lk_Service *svr);
LK_API void lk_popcontext (lk_State *S, lk_Context *ctx);

LK_NS_END

#endif /* lk_context_h */


#if defined(LOKI_IMPLEMENTATION) && !defined(lk_implemented)
#define lk_implemented


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LK_MAX_THREADS    32
#define LK_MAX_NAMESIZE   32
#define LK_MAX_SLOTNAME   63
#define LK_HASHLIMIT      5
#define LK_MIN_HASHSIZE   8
#define LK_MAX_SIZET      (~(size_t)0 - 100)

#define LK_INITIALING 0
#define LK_WORKING    1
#define LK_SLEEPING   2
#define LK_STOPPING   3


LK_NS_BEGIN

/* structures */

typedef struct lk_Poll lk_Poll;

typedef struct lk_SignalNode {
    lkQ_entry(struct lk_SignalNode);
    lk_Slot    *slot;
    lk_Signal   data;
} lk_SignalNode;

struct lk_Slot {
    char name[LK_MAX_SLOTNAME];
    char ispoll; /* 0:slot */
    lk_SignalHandler *handler;
    void *ud;
    lk_State   *S;
    lk_Service *service;
    lk_Slot    *next_slot; /* all slots in same service */
};

struct lk_Poll {
    lk_Slot base;
    lk_Thread   thread;
    lk_Event    event;
    lk_Lock     lock;
    lkQ_type(lk_SignalNode) signals;
    int status;
};

struct lk_Service {
    lk_Slot slot;
    lk_ServiceHandler *handler;
    lk_SignalHandler  *refactor;
    void *ud;
    void *data;
    lkQ_type(lk_SignalNode) signals;
    lkQ_entry(lk_Service);
    lk_Slot  *slots;
    lk_Slot  *polls;
    volatile unsigned status;
    unsigned weak    : 1;
    unsigned inqueue : 1;
    lk_Atomic pending; /* pending signals in queue */
    lk_Module module;
    lk_Lock   lock;
};

struct lk_State {
    lk_Service root;
    lk_Table preload;
    lk_Table slots;
    lk_Table config;
    lk_Buffer path;
    lk_Slot  *logger;
    lk_Slot  *monitor;
    lkQ_type(lk_Cleanup)    freed_cleanups;
    lkQ_type(lk_SignalNode) freed_signals;
    lkQ_type(lk_Service)    active_services;
    volatile unsigned status;
    size_t  nthreads;
    size_t  nservices;
    lk_TlsKey tls_index;
    lk_Lock   lock;
    lk_Event  event;
    lk_Thread threads[LK_MAX_THREADS];
};


/* memory management */

LK_API void lk_free (lk_State *S, void *ptr)
{ (void)S; free(ptr); }

static int lkM_outofmemory (lk_State *S) {
    fprintf(stderr, "out of memory\n");
    return lk_discard(S);
}

LK_API void *lk_malloc (lk_State *S, size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) lkM_outofmemory(S);
    return ptr;
}

LK_API void *lk_realloc (lk_State *S, void *ptr, size_t size) {
    void *newptr = realloc(ptr, size);
    if (newptr == NULL) lkM_outofmemory(S);
    return newptr;
}

LK_API char *lk_strdup (lk_State *S, const char *s) {
    size_t len = strlen(s) + 1;
    char *newstr = (char*)lk_malloc(S, len);
    memcpy(newstr, s, len);
    return newstr;
}

LK_API char *lk_strncpy (char *dst, size_t n, const char *s) {
    size_t len = strlen(s);
    if (len >= n - 1) {
        memcpy(dst, s, n-1);
        dst[n-1] = '\0';
    }
    else {
        memcpy(dst, s, len);
        memset(dst+len, 0, n-len);
    }
    return dst;
}


/* buffer routines */

LK_API void lk_initbuffer (lk_State *S, lk_Buffer *B) {
    B->size = 0;
    B->capacity = LK_BUFFERSIZE;
    B->S = S;
    B->buff = B->init_buff;
}

LK_API void lk_freebuffer (lk_Buffer *B) {
    if (B->buff != B->init_buff)
        lk_free(B->S, B->buff);
    lk_initbuffer(B->S, B);
}

LK_API char *lk_prepbuffsize (lk_Buffer *B, size_t len) {
    if (B->size + len > B->capacity) {
        void *newptr;
        size_t newsize = LK_BUFFERSIZE;
        while (newsize < B->size + len && newsize < ~(size_t)0/2)
            newsize *= 2;
        if (B->buff != B->init_buff) {
            newptr = lk_realloc(B->S, B->buff, newsize);
        }
        else {
            newptr = lk_malloc(B->S, newsize);
            memcpy(newptr, B->buff, B->size);
        }
        B->buff = (char*)newptr;
        B->capacity = newsize;
    }
    return &B->buff[B->size];
}

LK_API int lk_addlstring (lk_Buffer *B, const char *s, size_t len) {
    memcpy(lk_prepbuffsize(B, len), s, len);
    return B->size += len;
}

LK_API int lk_addvfstring (lk_Buffer *B, const char *fmt, va_list l) {
    const size_t init_size = 80;
    char *ptr = lk_prepbuffsize(B, init_size+1);
    size_t len;
    va_list l_count;
#ifdef va_copy
    va_copy(l_count, l);
#else
    __va_copy(l_count, l);
#endif
    len = vsnprintf(ptr, init_size, fmt, l_count);
    va_end(l_count);
    if (len <= 0) return 0;
    if (len > init_size) {
        if ((ptr = lk_prepbuffsize(B, len+1)) == NULL)
            return 0;
        vsnprintf(ptr, len, fmt, l);
    }
    return B->size += len;
}

LK_API int lk_addfstring (lk_Buffer *B, const char *fmt, ...) {
    int ret;
    va_list l;
    va_start(l, fmt);
    ret = lk_addvfstring(B, fmt, l);
    va_end(l);
    return ret;
}

LK_API void lk_replacebuffer (lk_Buffer *B, char origch, char newch) {
    size_t i;
    for (i = 0; i < B->size; ++i)
        if (B->buff[i] == origch)
            B->buff[i] = newch;
}

LK_API const char *lk_buffresult (lk_Buffer *B) {
    char *result = (char*)lk_malloc(B->S, B->size + 1);
    memcpy(result, B->buff, B->size);
    result[B->size] = '\0';
    lk_freebuffer(B);
    return result;
}


/* hashtable routines */

static size_t lkH_hashsize (size_t len) {
    size_t newsize = LK_MIN_HASHSIZE;
    while (newsize < LK_MAX_SIZET/2 && newsize < len)
        newsize <<= 1;
    return newsize;
}

static size_t lkH_countsize (lk_Table *t) {
    size_t i, count = 0;
    for (i = 0; i < t->size; ++i) {
        if (t->hash[i].key != NULL)
            ++count;
    }
    return count;
}

static lk_Entry *lkH_mainposition (lk_Table *t, lk_Entry *entry) {
    assert((t->size & (t->size - 1)) == 0);
    return &t->hash[entry->hash & (t->size - 1)];
}

static lk_Entry *lkH_newkey (lk_Table *t, lk_Entry *entry) {
    lk_Entry *mp;
    if (t->size == 0 && lk_resizetable(t, LK_MIN_HASHSIZE) == 0) 
        return NULL;
redo:
    mp = lkH_mainposition(t, entry);
    if (mp->key != 0) {
        lk_Entry *f = NULL, *othern;
        while (t->lastfree > 0) {
            lk_Entry *e = &t->hash[--t->lastfree];
            if (e->key == 0)  { f = e; break; }
        }
        if (f == NULL) {
            if (lk_resizetable(t, lkH_countsize(t)*2) == 0)
                return NULL;
            goto redo; /* return lkH_newkey(t, entry); */
        }
        assert(f->hash == 0);
        othern = lkH_mainposition(t, mp);
        if (othern != mp) {
            while (othern + othern->next != mp)
                othern += othern->next;
            othern->next = f - othern;
            *f = *mp;
            if (mp->next != 0) {
                f->next += mp - f;
                mp->next = 0;
            }
        }
        else {
            if (mp->next != 0)
                f->next = (mp + mp->next) - f;
            else assert(f->next == 0);
            mp->next = f - mp;
            mp = f;
        }
    }
    mp->key = entry->key;
    mp->hash = entry->hash;
    mp->value = entry->value;
    return mp;
}

static unsigned lkH_calchash (const char *s, size_t len) {
    size_t l1;
    size_t step = (len >> LK_HASHLIMIT) + 1;
    unsigned h = (unsigned)len;
    for (l1 = len; l1 >= step; l1 -= step)
        h ^= (h<<5) + (h>>2) + (unsigned char)s[l1 - 1];
    return h;
}

LK_API void lk_inittable (lk_State *S, lk_Table *t) {
    t->size = t->lastfree = 0;
    t->S = S;
    t->hash = NULL;
}

LK_API void lk_freetable (lk_Table *t) {
    if (t->hash != NULL) lk_free(t->S, t->hash);
    lk_inittable(t->S, t);
}

LK_API size_t lk_resizetable (lk_Table *t, size_t len) {
    size_t i;
    lk_Table new_map;
    new_map.S = t->S;
    new_map.size = lkH_hashsize(len);
    new_map.hash = (lk_Entry*)lk_malloc(t->S, new_map.size*sizeof(lk_Entry));
    new_map.lastfree = new_map.size;
    memset(new_map.hash, 0, sizeof(lk_Entry)*new_map.size);
    for (i = 0; i < t->size; ++i) {
        if (t->hash[i].key != NULL)
            lkH_newkey(&new_map, &t->hash[i]);
    }
    lk_free(t->S, t->hash);
    *t = new_map;
    return t->size;
}

LK_API lk_Entry *lk_getentry (lk_Table *t, const char *key) {
    unsigned hash;
    lk_Entry *e;
    if (t->size == 0 || key == NULL) return NULL;
    hash = lkH_calchash(key, strlen(key));
    assert((t->size & (t->size - 1)) == 0);
    e = &t->hash[hash & (t->size - 1)];
    while (1) {
        int next = e->next;
        if (e->hash == hash && key && e->key && strcmp(e->key, key) == 0)
            return e;
        if (next == 0) return NULL;
        e += next;
    }
    return NULL;
}

LK_API lk_Entry *lk_setentry (lk_Table *t, const char *key) {
    lk_Entry e, *ret;
    if (key == NULL) return NULL;
    if ((ret = lk_getentry(t, key)) != NULL)
        return ret;
    e.key = key;
    e.hash = lkH_calchash(key, strlen(key));
    e.value = 0;
    return lkH_newkey(t, &e);
}

LK_API void lk_delentry (lk_Table *t, lk_Entry *e) {
    lk_free(t->S, (void*)e->key);
    e->hash = 0;
    e->key = NULL;
    e->value = NULL;
}

LK_API int lk_nextentry (lk_Table *t, lk_Entry **pentry) {
    size_t i = *pentry ? *pentry - &t->hash[0] + 1 : 0;
    for (; i < t->size; ++i) {
        if (t->hash[i].key != NULL) {
            *pentry = &t->hash[i];
            return 1;
        }
    }
    return 0;
}


/* platform specific routines */

static void lkT_dispatchL (lk_State *S, lk_Service *svr);

static void lkT_enqueue (lk_State *S, lk_Service *svr)
{ lkQ_enqueue(&S->active_services, svr); svr->inqueue = 1; }

static void lkT_dequeue (lk_State *S, lk_Service **svr)
{ lkQ_dequeue(&S->active_services, *svr); if (*svr) (*svr)->inqueue = 0; }

#ifdef _WIN32

static size_t lkP_getcpucount (void) {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

static DWORD WINAPI lkP_poller (void *lpParameter) {
    lk_Context ctx;
    lk_Slot *slot  = (lk_Slot*)lpParameter;
    lk_Poll *poll = (lk_Poll*)slot;
    lk_State *S = slot->S;
    lk_pushcontext(S, &ctx, slot->service);
    lk_try(S, &ctx, slot->handler(S, slot->ud, slot, NULL));
    lk_popcontext(S, &ctx);
    lk_lock(poll->lock);
    poll->status = LK_STOPPING;
    lk_unlock(poll->lock);
    return 0;
}

static DWORD WINAPI lkP_worker (void *lpParameter) {
    lk_Context ctx;
    int status = LK_WORKING;
    lk_State *S  = (lk_State*)lpParameter;
    lk_pushcontext(S, &ctx, NULL);
    while (status == LK_WORKING) {
        lk_Service *svr;
        WaitForSingleObject(S->event, INFINITE);
        lk_lock(S->lock);
        lkT_dequeue(S, &svr);
        if ((status = S->status) == LK_STOPPING)
            lk_signal(S->event);
        lk_unlock(S->lock);
        while (svr != NULL) {
            ctx.current = svr;
            lkT_dispatchL(S, svr);
            ctx.current = NULL;
            lk_lock(S->lock);
            lkT_dequeue(S, &svr);
            lk_unlock(S->lock);
        }
    }
    lk_popcontext(S, &ctx);
    return 0;
}

LK_API void lk_waitclose (lk_State *S) {
    WaitForMultipleObjects(S->nthreads, S->threads, TRUE, INFINITE);
}

LK_API int lk_wait (lk_Slot *slot, lk_Signal* sig, int waitms) {
    lk_Poll *poll = (lk_Poll*)slot;
    lk_SignalNode *node;
    int status = LK_WORKING;
    if (!slot->ispoll) return LK_ERR;
    lk_lock(poll->lock);
    lkQ_dequeue(&poll->signals, node);
    status = poll->status;
    lk_unlock(poll->lock);
    if (waitms != 0 && (!node || status != LK_STOPPING)) {
        DWORD timeout = waitms < 0 ? INFINITE : (DWORD)waitms;
        WaitForSingleObject(poll->event, timeout);
    }
    if (node) {
        if (sig) *sig = node->data;
        return LK_OK;
    }
    return status == LK_STOPPING ? LK_ERR : LK_TIMEOUT;
}

#else

static size_t lkP_getcpucount (void) {
    return (size_t)sysconf(_SC_NPROCESSORS_ONLN);
}

static void *lkP_poller (void *ud) {
    lk_Context ctx;
    lk_Slot *slot  = (lk_Slot*)ud;
    lk_Poll *poll = (lk_Poll*)slot;
    lk_State *S = slot->S;
    lk_pushcontext(S, &ctx, slot->service);
    lk_try(S, &ctx, slot->handler(S, slot->ud, slot, NULL));
    lk_popcontext(S, &ctx);
    lk_lock(poll->lock);
    poll->status = LK_STOPPING;
    lk_unlock(poll->lock);
    return NULL;
}

static void *lkP_worker (void *ud) {
    lk_Context ctx;
    int status = LK_WORKING;
    lk_State *S  = (lk_State*)ud;
    lk_pushcontext(S, &ctx, NULL);
    lk_lock(S->lock);
    while (status == LK_WORKING) {
        lk_Service *svr;
        for (;;) {
            lkT_dequeue(S, &svr);
            status = S->status;
            if (svr != NULL || status != LK_WORKING)
                break;
            pthread_cond_wait(&S->event, &S->lock);
        }
        if (status == LK_STOPPING)
            pthread_cond_broadcast(&S->event);
        while (svr != NULL) {
            lk_unlock(S->lock);
            ctx.current = svr;
            lkT_dispatchL(S, svr);
            ctx.current = NULL;
            lk_lock(S->lock);
            lkT_dequeue(S, &svr);
        }
    }
    lk_unlock(S->lock);
    lk_popcontext(S, &ctx);
    return NULL;
}

LK_API void lk_waitclose (lk_State *S) {
    size_t i;
    for (i = 0; i < S->nthreads; ++i)
        pthread_join(S->threads[i], NULL);
    S->nthreads = 0;
}

LK_API int lk_wait (lk_Slot *slot, lk_Signal* sig, int waitms) {
    lk_Poll *poll = (lk_Poll*)slot;
    lk_SignalNode *node;
    int status = LK_WORKING;
    if (!slot->ispoll) return LK_ERR;
    lk_lock(poll->lock);
    lkQ_dequeue(&poll->signals, node);
    status = poll->status;
    if (waitms != 0 && (!node || status != LK_STOPPING)) {
        if (waitms < 0)
            pthread_cond_wait(&poll->event, &poll->lock);
        else {
            struct timeval tv;
            struct timespec ts;
            int sec  = waitms / 1000;
            int usec = waitms % 1000 * 1000;
            gettimeofday(&tv, NULL);
            if (tv.tv_usec + usec > 1000000) {
                sec += 1;
                usec = (tv.tv_usec + usec) - 1000000;
            }
            ts.tv_sec = tv.tv_sec + sec;
            ts.tv_nsec = (tv.tv_usec + usec) * 1000;
            pthread_cond_timedwait(&poll->event, &poll->lock, &ts);
            gettimeofday(&tv, NULL);
        }
    }
    lk_unlock(poll->lock);
    if (node) {
        if (sig) *sig = node->data;
        return LK_OK;
    }
    return status == LK_STOPPING ? LK_ERR : LK_TIMEOUT;
}

#endif


/* singal slot routines */

LK_API const char *lk_name    (lk_Slot *slot) { return slot->name;    }
LK_API lk_Service *lk_service (lk_Slot *slot) { return slot->service; }
LK_API lk_State   *lk_state   (lk_Slot *slot) { return slot->S;       }

static lk_Context *lkS_context (lk_State *S)
{ return (lk_Context*)lk_gettls(S->tls_index); }

static const char *lkS_slotname (lk_Buffer *B, lk_State *S, const char *name) {
    lk_Service *self = lk_self(S);
    lk_initbuffer(S, B);
    lk_addstring(B, lk_name((lk_Slot*)self));
    lk_addchar(B, '.');
    lk_addstring(B, name);
    lk_addchar(B, '\0');
    assert(lk_buffsize(B) < LK_MAX_SLOTNAME-1);
    return lk_buffer(B);
}

static lk_Slot *lkS_newslot (lk_State *S, size_t sz, const char *name) {
    lk_Slot *slot;
    lk_Entry *e = lk_setentry(&S->slots, name);
    lk_Service *svr = lk_self(S);
    if (e->value != NULL) return NULL;
    slot = (lk_Slot*)lk_malloc(S, sz);
    lk_strncpy(slot->name, LK_MAX_SLOTNAME, name);
    slot->ispoll = 0;
    slot->S = S;
    slot->service = svr;
    e->key = slot->name;
    e->value = slot;
    return slot;
}

LK_API lk_Slot *lk_newslot (lk_State *S, const char *name, lk_SignalHandler *h, void *ud) {
    lk_Slot *slot = NULL;
    lk_Buffer B;
    if (lk_self(S)->status == LK_STOPPING)
        return NULL;
    name = lkS_slotname(&B, S, name);
    lk_lock(S->lock);
    slot = lkS_newslot(S, sizeof(lk_Slot), name);
    slot->handler = h;
    slot->ud = ud;
    slot->next_slot = slot->service->slots;
    slot->service->slots = slot;
    lk_unlock(S->lock);
    lk_freebuffer(&B);
    return slot;
}

static int lkP_startpoll (lk_Poll *poll) {
    if (!lk_initlock(&poll->lock))   goto err_lock;
    if (!lk_initevent(&poll->event)) goto err_event;
    if (!lk_initthread(&poll->thread, lkP_poller, poll)) {
        lk_freeevent(poll->event);
err_event:
        lk_freelock(poll->lock);
err_lock:
        return LK_ERR;
    }
    return LK_OK;
}

static void lkS_stoppoll (lk_State *S, lk_Poll *poll) {
    lk_lock(poll->lock);
    poll->status = LK_STOPPING;
    lk_signal(poll->event);
    lk_unlock(poll->lock);
    lk_unlock(S->lock);
    lk_waitthread(poll->thread);
    lk_lock(S->lock);
    lk_freeevent(poll->event);
    lk_freelock(poll->lock);
}

LK_API lk_Slot *lk_newpoll (lk_State *S, const char *name, lk_SignalHandler *h, void *ud) {
    lk_Poll *poll;
    lk_Slot *slot = NULL;
    lk_Buffer B;
    if (lk_self(S)->status == LK_STOPPING)
        return NULL;
    name = lkS_slotname(&B, S, name);
    lk_lock(S->lock);
    slot = lkS_newslot(S, sizeof(lk_Poll), name);
    slot->ispoll = 1;
    slot->handler = h;
    slot->ud = ud;
    slot->next_slot = slot->service->polls;
    slot->service->polls = slot;
    poll = (lk_Poll*)slot;
    lkQ_init(&poll->signals);
    poll->status = LK_WORKING;
    if (lkP_startpoll((lk_Poll*)slot) != LK_OK) {
        lk_Entry *e = lk_getentry(&S->slots, name);
        assert(e && e->value);
        lk_delentry(&S->slots, e);
        slot = NULL;
    }
    lk_unlock(S->lock);
    lk_freebuffer(&B);
    return slot;
}

LK_API lk_Slot *lk_slot (lk_State *S, const char *name) {
    lk_Entry *e;
    lk_Slot  *slot = NULL;
    lk_lock(S->lock);
    e = lk_getentry(&S->slots, name);
    if (e != NULL) slot = (lk_Slot*)e->value;
    lk_unlock(S->lock);
    return slot;
}

LK_API lk_SignalHandler *lk_slothandler (lk_Slot *slot, void **pud) {
    lk_SignalHandler *h;
    lk_lock(slot->service->lock);
    if (pud) *pud = slot->ud;
    h = slot->handler;
    lk_unlock(slot->service->lock);
    return h;
}

LK_API void lk_setslothandler (lk_Slot *slot, lk_SignalHandler *h, void *ud) {
    lk_lock(slot->service->lock);
    slot->handler = h;
    slot->ud = ud;
    lk_unlock(slot->service->lock);
}

LK_API int lk_register (lk_State *S, const lk_Reg *slots, void *ud) {
    lk_Buffer B;
    const lk_Reg *begin = slots;
    lk_initbuffer(S, &B);
    lk_lock(S->lock);
    while (slots->name != NULL) {
        const char *name = slots->name;
        lk_Slot *slot = NULL;
        lk_resetbuffer(&B);
        name = lkS_slotname(&B, S, name);
        slot = lkS_newslot(S, sizeof(lk_Slot), name);
        slot->handler = slots->handler;
        slot->ud = ud;
        slot->next_slot = slot->service->slots;
        slot->service->slots = slot;
        ++slots;
    }
    lk_unlock(S->lock);
    lk_freebuffer(&B);
    return slots - begin;
}

static int lkS_emitpollL (lk_State *S, lk_Poll *poll, lk_SignalNode *node) {
    int ret = LK_ERR;
    lk_lock(poll->lock);
    if (poll->status == LK_STOPPING)
        lkQ_enqueue(&S->freed_signals, node);
    else {
        ret = LK_OK;
        lkQ_enqueue(&poll->signals, node);
        lk_signal(poll->event);
    }
    lk_unlock(poll->lock);
    return ret;
}

static int lkS_emitslotL (lk_State *S, lk_Service *svr, lk_SignalNode *node) {
    int ret = LK_ERR;
    lk_lock(svr->lock);
    if (svr->status == LK_STOPPING)
        lkQ_enqueue(&S->freed_signals, node);
    else {
        lkQ_enqueue(&svr->signals, node);
        if (svr->status == LK_SLEEPING) {
            lkT_enqueue(S, svr);
            svr->status = LK_WORKING;
            lk_signal(S->event);
        }
        ret = LK_OK;
    }
    lk_unlock(svr->lock);
    return ret;
}

LK_API int lk_emit (lk_Slot *slot, const lk_Signal *sig) {
    lk_State *S = slot->S;
    lk_Service *svr = slot->service, *self = lk_self(S);
    lk_SignalNode *node = NULL, tmp;
    int ret = LK_ERR;
    if (S->status == LK_STOPPING
            || self->status == LK_STOPPING
            || svr->status == LK_STOPPING) 
        return LK_ERR;
    memset(&tmp, 0, sizeof(tmp));
    tmp.slot = slot;
    tmp.data = *sig;
    if (tmp.data.src == NULL)
        tmp.data.src = self;
    lk_lock(S->lock);
    lkQ_dequeue(&S->freed_signals, node);
    if (node == NULL)
        node = (lk_SignalNode*)lk_malloc(S, sizeof(lk_SignalNode));
    *node = tmp;
    if (sig->copy) {
        char *buff = (char*)lk_malloc(S, sig->size+1);
        memcpy(buff, sig->data, sig->size);
        buff[sig->size] = '\0';
        node->data.data = buff;
    }
    ret = slot->ispoll ? lkS_emitpollL(S, (lk_Poll*)slot, node)
                       : lkS_emitslotL(S, svr, node);
    lk_unlock(S->lock);
    lk_inc(self->pending);
    return ret;
}

LK_API int lk_emitdata (lk_Slot *slot, unsigned type, unsigned session, const void *data, size_t size) {
    lk_Signal sig = LK_SIGNAL;
    sig.copy = 1;
    sig.type = type;
    sig.session = session;
    sig.size = size;
    sig.data = (char*)data;
    return lk_emit(slot, &sig);
}

LK_API int lk_emitstring (lk_Slot *slot, unsigned type, unsigned session, const char *s) {
    lk_Signal sig = LK_SIGNAL;
    sig.copy = 1;
    sig.type = type;
    sig.session = session;
    sig.size = strlen(s);
    sig.data = (char*)s;
    return lk_emit(slot, &sig);
}


/* service routines */

static int lkT_isservice (lk_Slot *slot)
{ return slot != NULL && slot->service == (lk_Service*)slot; }

LK_API void lk_popcontext (lk_State *S, lk_Context *ctx)
{ lk_settls(S->tls_index, ctx->prev); }

LK_API void lk_pushcontext (lk_State *S, lk_Context *ctx, lk_Service *svr) {
    ctx->prev = lkS_context(S);
    ctx->S = S;
    ctx->current = svr;
    ctx->cleanups = NULL;
    ctx->status = LK_OK;
    lk_settls(S->tls_index, ctx);
}

static int lkT_initservice (lk_State *S, lk_Service *svr, const char *name) {
    memset(svr, 0, sizeof(*svr));
    if (!lk_initlock(&svr->lock)) return 0;
    lk_strncpy(svr->slot.name, LK_MAX_NAMESIZE, name);
    svr->slot.S = S;
    svr->slot.service = svr;
    svr->slots = &svr->slot;
    svr->status = LK_INITIALING; /* avoid push it into queue before init */
    lkQ_init(&svr->signals);
    return 1;
}

static lk_Service *lkT_newservice (lk_State *S, const char *name, lk_ServiceHandler *h) {
    lk_Entry *e = lk_setentry(&S->slots, name);
    if (!lkT_isservice((lk_Slot*)e->value)) {
        lk_Service *svr;
        if (e->value != NULL) return NULL;
        assert(strlen(name) < LK_MAX_NAMESIZE);
        svr = (lk_Service*)lk_malloc(S, sizeof(lk_Service));
        if (!lkT_initservice(S, svr, name)) {
            lk_free(S, svr);
            return NULL;
        }
        svr->handler = h;
        e->key = svr->slot.name;
        e->value = svr;
        assert(svr->slot.name == (char*)svr);
    }
    return (lk_Service*)e->value;
}

static const char *lkT_makemodname (lk_Buffer *B, const char *p[4]) {
    const char *start = p[0], *end = p[1], *name = p[2], *nend = p[3];
    lk_resetbuffer(B);
    for (; start < end && *start != ';'; ++start) {
        if (*start == '?')
            lk_addlstring(B, name, nend-name);
        else
            lk_addchar(B, *start);
    }
    if (lk_buffsize(B) != 0)
        lk_addchar(B, '\0');
    return start;
}

static lk_Module lkT_findmodule (lk_State *S, const char *name, lk_ServiceHandler **h) {
    union {
        const char *p[4];
        struct { const char *start, *end, *name, *nend; } s;
    } u;
    const char *tmp, *dot = strchr(name, '.');
    lk_Module module;
    lk_Buffer B;
    u.s.start = lk_buffer(&S->path);
    u.s.end   = u.s.start + lk_buffsize(&S->path);
    u.s.name  = name;
    u.s.nend  = name + strlen(name);
    lk_initbuffer(S, &B);

redo:
    tmp = u.s.start;
    u.s.start = lkT_makemodname(&B, u.p);
    if (lk_buffsize(&B) == 0) goto next;
    module = lk_loadlib(lk_buffer(&B));
    if (module == NULL) {
        if (dot) {
            u.s.start = tmp;
            u.s.start = lkT_makemodname(&B, u.p);
            module = lk_loadlib(lk_buffer(&B));
        }
        if (module == NULL) goto next;
    }
    lk_resetbuffer(&B);
    lk_addstring(&B, LK_PREFIX);
    lk_addstring(&B, name);
    if (dot) lk_replacebuffer(&B, '.', '_');
    *h = (lk_ServiceHandler*)lk_getaddr(module, lk_buffer(&B));
    if (h == NULL) {
        lk_freelib(module);
        goto next;   
    }
    return module;

next:
    if (*u.s.start == ';') {
        ++u.s.start;
        goto redo;
    }
    return NULL;
}

static lk_Service *lkT_newmodule (lk_State *S, const char *name) {
    lk_Entry *pe, *e = lk_setentry(&S->slots, name);
    lk_ServiceHandler *h;
    lk_Service *svr;
    lk_Module module;
    assert(strlen(name) < LK_MAX_NAMESIZE);
    if (lkT_isservice((lk_Slot*)e->value))
        return (lk_Service*)e->value;
    pe = lk_getentry(&S->preload, name);
    if (pe != NULL && pe->value != NULL) {
        h = (lk_ServiceHandler*)(ptrdiff_t)pe->value;
        lk_delentry(&S->slots, pe);
        return lkT_newservice(S, name, h);
    }
    module = lkT_findmodule(S, name, &h);
    if (module == NULL) return NULL;
    svr = (lk_Service*)lk_malloc(S, sizeof(lk_Service));
    if (!lkT_initservice(S, svr, name)) {
        lk_freelib(module);
        lk_free(S, svr);
        return NULL;
    }
    svr->module = module;
    svr->handler = h;
    e->key = svr->slot.name;
    e->value = svr;
    return svr;
}

static void lkT_freeslotsL (lk_State *S, lk_Service *svr) {
    lk_Entry *e;
    lk_lock(S->lock);
    while (svr->slots != NULL) {
        lk_Slot *next = svr->slots->next_slot;
        e = lk_getentry(&S->slots, svr->slots->name);
        assert(e && (lk_Slot*)e->value == svr->slots);
        if (svr->slots != &svr->slot)
            lk_delentry(&S->slots, e);
        else {
            e->hash = 0;
            e->key = NULL;
            e->value = NULL;
        }
        svr->slots = next;
    }
    while (svr->polls != NULL) {
        lk_Slot *next = svr->polls->next_slot;
        e = lk_getentry(&S->slots, svr->polls->name);
        assert(e && (lk_Slot*)e->value == svr->polls);
        lkS_stoppoll(S, (lk_Poll*)e->value);
        lk_delentry(&S->slots, e);
        svr->polls = next;
    }
    lk_unlock(S->lock);
}

static void lkT_delserviceL (lk_State *S, lk_Service *svr) {
    lk_Context ctx;
    assert(lkQ_empty(&svr->signals));
    if (svr->slot.handler) {
        lk_pushcontext(S, &ctx, svr);
        lk_try(S, &ctx, svr->slot.handler(S, svr->slot.ud, &svr->slot, NULL));
        lk_popcontext(S, &ctx);
    }
    assert(lkQ_empty(&svr->signals));
    lkT_freeslotsL(S, svr);
    assert(lkQ_empty(&svr->signals));
    lk_freelock(svr->lock);
    if (svr->module != NULL)
        lk_freelib(svr->module);
    lk_lock(S->lock);
    if (&svr->slot == S->logger)  S->logger = NULL;
    if (&svr->slot == S->monitor) S->monitor = NULL;
    if (S->monitor)
        lk_emitdata(S->monitor, 1, 0, &svr, sizeof(svr));
    if (svr != &S->root) lk_free(S, svr);
    if (!svr->weak) --S->nservices;
    if (S->nservices == 0 ||
            (S->nservices == 1 && S->root.slot.handler == NULL)) {
        S->status = LK_STOPPING;
        lk_signal(S->event);
    }
    lk_unlock(S->lock);
}

static lk_Service *lkT_callhandlerL (lk_State *S, lk_Service *svr) {
    lk_Context ctx;
    int ret = LK_OK;
    int need_initialize;

    if (!svr) return NULL;
    lk_lock(svr->lock);
    need_initialize = (svr->status == LK_INITIALING);
    if (need_initialize)
        svr->status = LK_WORKING; /* do not add to queue before call handler */
    lk_unlock(svr->lock);
    if (!need_initialize) return svr;

    lk_pushcontext(S, &ctx, svr);
    lk_try(S, &ctx, ret = svr->handler(S));
    lk_popcontext(S, &ctx);
    if (ctx.status == LK_ERR || ret < 0) {
        lkT_delserviceL(S, svr);
        return NULL;
    }

    lk_lock(S->lock);
    lk_lock(svr->lock);
    if (ret == LK_WEAK) svr->weak = 1;
    else ++S->nservices;
    if (lkQ_empty(&svr->signals))
        svr->status = LK_SLEEPING;
    else
        lkT_enqueue(S, svr);
    lk_unlock(svr->lock);
    if (S->monitor) {
        lk_Service *svrs[2];
        svrs[0] = lk_self(S), svrs[1] = svr;
        lk_emitdata(S->monitor, 0, 0, svrs, sizeof(*svrs));
    }
    if (!S->logger && strcmp(svr->slot.name, "log") == 0)
        S->logger = &svr->slot;
    if (!S->monitor && strcmp(svr->slot.name, "monitor") == 0)
        S->monitor = &svr->slot;
    lk_unlock(S->lock);
    return svr;
}

LK_API lk_Service *lk_self (lk_State *S) {
    lk_Context *ctx = lkS_context(S);
    return ctx ? ctx->current : &S->root;
}

LK_API void lk_preload (lk_State *S, const char *name, lk_ServiceHandler *h) {
    lk_Entry *e;
    lk_lock(S->lock);
    e = lk_getentry(&S->slots, name);
    if (e == NULL) {
        e = lk_setentry(&S->preload, name);
        if (e->value == NULL) {
            e->key = lk_strdup(S, name);
            e->value = (void*)(ptrdiff_t)h;
        }
    }
    lk_unlock(S->lock);
}

LK_API lk_Service *lk_requiref (lk_State *S, const char *name, lk_ServiceHandler *h) {
    lk_Service *svr;
    lk_lock(S->lock);
    svr = lkT_newservice(S, name, h);
    lk_unlock(S->lock);
    return lkT_callhandlerL(S, svr);
}

LK_API lk_Service *lk_require (lk_State *S, const char *name) {
    lk_Service *svr;
    lk_lock(S->lock);
    svr = lkT_newmodule(S, name);
    lk_unlock(S->lock);
    return lkT_callhandlerL(S, svr);
}

LK_API void *lk_data (lk_Service *svr) {
    void *data;
    lk_lock(svr->lock);
    data = svr->data;
    lk_unlock(svr->lock);
    return data;
}

LK_API void lk_setdata (lk_Service *svr, void *data) {
    lk_lock(svr->lock);
    svr->data = data;
    lk_unlock(svr->lock);
}

LK_API lk_SignalHandler *lk_refactor (lk_Service *svr, void **pud) {
    lk_SignalHandler *h;
    lk_lock(svr->lock);
    if (pud) *pud = svr->ud;
    h = svr->refactor;
    lk_unlock(svr->lock);
    return h;
}

LK_API void lk_setrefactor (lk_Service *svr, lk_SignalHandler *h, void *ud) {
    lk_lock(svr->lock);
    svr->refactor = h;
    svr->ud = ud;
    lk_unlock(svr->lock);
}

static void lkT_dispatchL (lk_State *S, lk_Service *svr) {
    lkQ_type(lk_SignalNode) freed_signals;
    lk_SignalNode *node;
    int should_delete;

    /* fetch all signal */
    lk_lock(svr->lock);
    node = lkQ_head(&svr->signals);
    lkQ_init(&svr->signals);
    lk_unlock(svr->lock);

    /* call signal handler */
    lkQ_init(&freed_signals);
    while (node != NULL) {
        int ret = LK_ERR;
        lk_Context ctx;
        lk_SignalNode *next = node->next;
        lk_Slot *slot = node->slot;
        lk_Service *src = node->data.src;
        lk_pushcontext(S, &ctx, svr);
        assert(src != NULL);
        if (src->refactor && slot != S->logger) {
            lk_try(S, &ctx, ret = src->refactor(S, src->ud, slot, &node->data));
        }
        if (ret == LK_ERR && slot->handler) {
            lk_try(S, &ctx, slot->handler(S, slot->ud, slot, &node->data));
        }
        lk_popcontext(S, &ctx);
        if (node->data.copy)
            lk_free(S, node->data.data);
        lkQ_enqueue(&freed_signals, node);
        node = next;
        if (src->status == LK_STOPPING && lk_dec(src->pending) == 0) {
            lk_lock(S->lock);
            if (!svr->inqueue) lkT_enqueue(S, svr);
            lk_unlock(S->lock);
        }
    }

    /* cleanup */
    should_delete = 0;
    lk_lock(S->lock);
    lkQ_merge(&S->freed_signals, &freed_signals);
    lk_lock(svr->lock);
    if (!lkQ_empty(&svr->signals))
        lkT_enqueue(S, svr);
    else if (svr->status != LK_STOPPING)
        svr->status = LK_SLEEPING;
    else if (svr->pending == 0)
        should_delete = 1;
    lk_unlock(svr->lock);
    lk_unlock(S->lock);

    if (should_delete) lkT_delserviceL(S, svr);
}


/* global routines */

static void lkG_delstate (lk_State *S) {
    lk_Entry *e = NULL;
    size_t i;
    while (lk_nextentry(&S->slots, &e)) {
        lk_Slot *slot = (lk_Slot*)e->value;
        if (lkT_isservice(slot))
            lkT_delserviceL(S, (lk_Service*)slot);
    }
    for (i = 0; i < S->preload.size; ++i) {
        const char *key = S->preload.hash[i].key;
        if (key != NULL) lk_free(S, (void*)key);
    }
    for (i = 0; i < S->config.size; ++i) {
        const char *key = S->config.hash[i].key;
        if (key != NULL) lk_free(S, (void*)key);
    }
    lk_freetable(&S->preload);
    lk_freetable(&S->slots);
    lk_freebuffer(&S->path);
    for (i = 0; i < S->nthreads; ++i)
        lk_freethread(S->threads[i]);
    lk_freeevent(S->event);
    lk_freetls(S->tls_index);
    lk_freelock(S->lock);
    free(S);
}

LK_API lk_State *lk_newstate (const char *name) {
    lk_State *S = (lk_State*)malloc(sizeof(lk_State));
    if (S == NULL) return NULL;
    memset(S, 0, sizeof(*S));
    name = name ? name : "root";
    if (!lk_inittls(&S->tls_index)) goto err_tls;
    if (!lk_initevent(&S->event))   goto err_event;
    if (!lk_initlock(&S->lock))     goto err_lock;
    if (!lkT_initservice(S, &S->root, name)) goto err_root;
    S->root.status = LK_SLEEPING;
    S->status = LK_INITIALING;
    S->nservices = 1;
    lk_initbuffer(S, &S->path);
    lk_addstring(&S->path, LK_PATH);
    lk_inittable(S, &S->preload);
    lk_inittable(S, &S->slots);
    lkQ_init(&S->freed_signals);
    lkQ_init(&S->active_services);
    lk_setentry(&S->slots, S->root.slot.name)->value = &S->root.slot;
    return S;

err_root:
err_lock:
     lk_freeevent(S->event); 
err_event:
    lk_freetls(S->tls_index);
err_tls:
    free(S);
    return NULL;
}

LK_API void lk_close (lk_State *S) {
    lk_Context *ctx = lkS_context(S);
    lk_Service *svr;
    if (ctx == NULL) {
        if (S->status == LK_STOPPING) {
            lkG_delstate(S);
            return;
        }
    }
    else if ((svr = ctx->current) != NULL) {
        lk_lock(S->lock);
        lk_lock(svr->lock);
        if (svr->status == LK_SLEEPING)
            lkT_enqueue(S, svr);
        svr->status = LK_STOPPING;
        lk_unlock(svr->lock);
        lk_unlock(S->lock);
    }
    lk_lock(S->lock);
    lk_signal(S->event);
    lk_unlock(S->lock);
}

LK_API void lk_setthreads (lk_State *S, int threads) {
    lk_lock(S->lock);
    if (S->status == LK_INITIALING)
        S->nthreads = threads;
    lk_unlock(S->lock);
}

LK_API void lk_setpath (lk_State *S, const char *path) {
    lk_lock(S->lock);
    if (S->status == LK_INITIALING) {
        lk_addchar(&S->path, ';');
        lk_addstring(&S->path, path);
    }
    lk_unlock(S->lock);
}

LK_API int lk_start (lk_State *S) {
    size_t i, count = 0;
    lk_lock(S->lock);
    if (S->nthreads <= 0)
        S->nthreads = lkP_getcpucount();
    S->status = LK_WORKING;
    for (i = 0; i < S->nthreads; ++i) {
        if (!lk_initthread(&S->threads[i], lkP_worker, S))
            break;
    }
    count = S->nthreads = i;
    lk_unlock(S->lock);
    return count;
}

LK_API int lk_pcall (lk_State *S, lk_Handler *h, void *ud) {
    int ret = LK_OK;
    lk_Context ctx;
    lk_pushcontext(S, &ctx, NULL);
    lk_try(S, &ctx, ret = h(S, ud));
    lk_popcontext(S, &ctx);
    return ctx.status == LK_ERR ? LK_ERR : ret;
}

LK_API int lk_discard (lk_State *S) {
    lk_Context *ctx = lkS_context(S);
    lkQ_type(lk_Cleanup) cleanups;
    if (ctx == NULL) {
        fprintf(stderr, "unproected error\n");
        abort();
    }
    lkQ_init(&cleanups);
    while (ctx->cleanups != NULL) {
        lk_Cleanup *next = ctx->cleanups->next;
        ctx->cleanups->h(S, ctx->cleanups->ud);
        lkQ_enqueue(&cleanups, ctx->cleanups);
        ctx->cleanups = next;
    }
    if (!lkQ_empty(&cleanups)) {
        lk_lock(S->lock);
        lkQ_merge(&S->freed_cleanups, &cleanups);
        lk_unlock(S->lock);
    }
    lk_throw(S, ctx);
    return LK_ERR;
}

LK_API int lk_addcleanup (lk_State *S, lk_Handler *h, void *ud) {
    lk_Context *ctx = lkS_context(S);
    lk_Cleanup *cleanup;
    if (ctx == NULL)
        return LK_ERR;
    lk_lock(S->lock);
    lkQ_dequeue(&S->freed_cleanups, cleanup);
    if (cleanup == NULL)
        cleanup = (lk_Cleanup*)lk_malloc(S, sizeof(lk_Cleanup));
    lk_unlock(S->lock);
    cleanup->h = h;
    cleanup->ud = ud;
    cleanup->next = ctx->cleanups;
    ctx->cleanups = cleanup;
    return LK_OK;
}

LK_API char *lk_getconfig (lk_State *S, const char *key) {
    lk_Entry *e;
    char *value = NULL;
    lk_lock(S->lock);
    e = lk_getentry(&S->config, key);
    if (e) value = lk_strdup(S, (char*)e->value);
    lk_unlock(S->lock);
    return value;
}

LK_API void lk_setconfig (lk_State *S, const char *key, const char *value) {
    lk_Entry *e;
    size_t valuesize = strlen(value);
    lk_lock(S->lock);
    e = lk_setentry(&S->config, key);
    if (e->value && strlen((char*)e->value) >= valuesize)
        memcpy(e->value, value, valuesize+1);
    else {
        size_t keysize = strlen(key);
        lk_Buffer B;
        lk_initbuffer(S, &B);
        lk_addlstring(&B, key, keysize);
        lk_addchar(&B, '\0');
        lk_addlstring(&B, value, valuesize);
        lk_addchar(&B, '\0');
        lk_free(S, (void*)e->key);
        e->key = lk_buffresult(&B);
        e->value = (void*)&e->key[keysize + 1];
    }
    lk_unlock(S->lock);
}

LK_API void lk_log(lk_State *S, const char *fmt, ...) {
    va_list l;
    va_start(l, fmt);
    lk_vlog(S, fmt, l);
    va_end(l);
}

LK_API void lk_vlog(lk_State *S, const char *fmt, va_list l) {
    lk_Slot *logger = S->logger;
    if (logger) {
        lk_Signal sig = LK_SIGNAL;
        lk_Buffer B;
        lk_initbuffer(S, &B);
        lk_addvfstring(&B, fmt, l);
        sig.copy = 1;
        sig.size = lk_buffsize(&B);
        sig.data = lk_buffer(&B);
        lk_emit(logger, &sig);
        lk_freebuffer(&B);
    }
}


LK_NS_END

#endif

/* win32cc: flags+='-Wextra -s -O3 -mdll -DLOKI_IMPLEMENTATION -std=c90 -pedantic -xc'
 * win32cc: output='loki.dll'
 * unixcc: flags+='-Wextra -s -O3 -fPIC -shared -DLOKI_IMPLEMENTATION -xc'
 * unixcc: output='loki.so' */

