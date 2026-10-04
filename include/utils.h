#pragma once

#include <common.h>

#include <stddef.h>
#include <stdint.h>

#include <utils_lock.h>

#if defined(_MSC_VER)
// Microsoft

#include <windows.h>

#define LIKELY(exp) exp
#define UNLIKELY(exp) exp

#define DECLARE_RUNTIME_THREAD_FUNC(f) unsigned int f(void* ctx)

typedef uintptr_t server_thread_t;
typedef unsigned int (*runtime_thread_func)(void* arg);

#elif defined(__GNUC__)
// GCC

#define LIKELY(exp) __builtin_expect(exp, true)
#define UNLIKELY(exp) __builtin_expect(exp, false)

#define DECLARE_RUNTIME_THREAD_FUNC(f) void* f(void* ctx)

typedef pthread_t server_thread_t;
typedef void* (*runtime_thread_func)(void* arg);

#endif

#define ENSURE_SIZE(ptr, new_size, capac, elem_size) \
	if (UNLIKELY(new_size >= capac)) \
	{ \
		grow_ptr((char**) &ptr, &capac, elem_size); \
	}

#define ENSURE_SIZE_FAR(ptr, new_size, capac, elem_size) \
	if (UNLIKELY(new_size >= capac)) \
	{ \
		grow_ptr_far((char**) &ptr, &capac, elem_size, new_size); \
	}

void server_init_utils();
void server_sync_window();
void server_deinit_utils();

size_t get_power_two_size(size_t old_size, size_t size);

void grow_ptr(char** ptr, size_t* capacity_ptr, size_t elem_size);
void grow_ptr_far(char** ptr, size_t* capacity_ptr, size_t elem_size, size_t new_size);

u32 get_elapsed_ms();
u64 get_elapsed_ns();
void recomp_sleep(u32 ms);
int getpagesize();

char* vmem_reserve(size_t size);
void vmem_release(char* addr, size_t size);

void thread_start(runtime_thread_func f, server_thread_t* handle);
void thread_exit();
void thread_join(server_thread_t* handle);

void rwlock_init(server_rwlock_t* rwlock);
void rwlock_lock_read(server_rwlock_t* rwlock);
void rwlock_unlock_read(server_rwlock_t* rwlock);
void rwlock_lock_write(server_rwlock_t* rwlock);
void rwlock_unlock_write(server_rwlock_t* rwlock);
void rwlock_destroy(server_rwlock_t* rwlock);

void sitl_init(u16 port);
int sitl_udp_recv(char* data, size_t buffer_size);
void sitl_tcp_init(u16 port);
void sitl_send_json(char* data, size_t data_size);
int sitl_tcp_read(char* out, size_t out_size);
void sitl_tcp_write(char* data, size_t data_size);