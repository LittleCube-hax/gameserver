#include <string.h>

#include <heap.h>
#include <utils.h>

size_t get_power_two_size(size_t old_size, size_t size)
{
	while (old_size < size)
	{
		old_size <<= 1;
	}
	
	return old_size;
}

void grow_ptr(char** ptr, size_t* capacity_ptr, size_t elem_size)
{
	char* data = *ptr;
	size_t capacity = *capacity_ptr;
	size_t old_data_size = capacity*elem_size;
	
	char* new_data = HALLOC(old_data_size << 1);
	
	memcpy(new_data, data, old_data_size);
	
	FREE(data);
	
	*ptr = new_data;
	*capacity_ptr = capacity << 1;
}

void grow_ptr_far(char** ptr, size_t* capacity_ptr, size_t elem_size, size_t new_size)
{
	char* data = *ptr;
	size_t capacity = *capacity_ptr;
	size_t old_data_size = capacity*elem_size;
	
	size_t new_capacity = get_power_two_size(capacity, new_size);
	size_t new_data_size = new_capacity*elem_size;
	
	char* new_data = HALLOC(new_data_size);
	
	memcpy(new_data, data, old_data_size);
	
	FREE(data);
	
	*ptr = new_data;
	*capacity_ptr = new_capacity;
}

#if defined(_MSC_VER)
// Microsoft

#include <windows.h>
#include <process.h>
#include <Winbase.h>

#include <dwmapi.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dwmapi.lib")

#pragma comment(lib, "ws2_32.lib")

// windows-only machine-specific global:
LARGE_INTEGER counter_frequency;

void server_init_utils()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	QueryPerformanceFrequency(&counter_frequency);
	
	timeBeginPeriod(1);
}

void server_sync_window()
{
	DwmFlush();
}

void server_deinit_utils()
{
	timeEndPeriod(1);
}

u32 get_elapsed_ms()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	
	u32 time = (u32) (1000*counter.QuadPart/counter_frequency.QuadPart);
	
	return time;
}

u64 get_elapsed_ns()
{
	FILETIME t;
	GetSystemTimePreciseAsFileTime(&t);
	u64 ticks = (((u64) t.dwHighDateTime) << 32) | t.dwLowDateTime;
	
	u64 time = (u64) (100*(ticks - TICKS_FROM_1601_TO_2000));
	
	return time;
}

void server_sleep(u32 ms)
{
	Sleep(ms);
}

int getpagesize()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	
	return si.dwPageSize;
}

char* vmem_reserve(size_t size)
{
	return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void vmem_release(char* addr, size_t size)
{
	VirtualFree(addr, 0, MEM_RELEASE);
}

void thread_start(runtime_thread_func f, server_thread_t* handle, void* arg)
{
	*handle = _beginthreadex(NULL, 0, f, arg, 0, NULL);
}

void thread_exit()
{
	_endthreadex(0);
}

void thread_join(server_thread_t* handle)
{
	WaitForSingleObject((HANDLE) *handle, INFINITE);
	CloseHandle((HANDLE) *handle);
}

void rwlock_init(server_rwlock_t* rwlock)
{
	InitializeSRWLock((PSRWLOCK) rwlock);
}

void rwlock_lock_read(server_rwlock_t* rwlock)
{
	AcquireSRWLockShared((PSRWLOCK) rwlock);
}

void rwlock_unlock_read(server_rwlock_t* rwlock)
{
	ReleaseSRWLockShared((PSRWLOCK) rwlock);
}

void rwlock_lock_write(server_rwlock_t* rwlock)
{
	AcquireSRWLockExclusive((PSRWLOCK) rwlock);
}

void rwlock_unlock_write(server_rwlock_t* rwlock)
{
	ReleaseSRWLockExclusive((PSRWLOCK) rwlock);
}

void rwlock_destroy(server_rwlock_t* rwlock)
{
	
}

#elif defined(__GNUC__)
// GCC

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <arpa/inet.h>

void server_init_utils()
{
	
}

void server_sync_window()
{
	
}

void server_deinit_utils()
{
	
}

u32 get_elapsed_ms()
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	return (now.tv_sec)*1000 + (now.tv_nsec)/1000000;
}

u64 get_elapsed_ns()
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return ((u64) (now.tv_sec - 946684800ULL)*1000000000ULL) + now.tv_nsec;
}

void server_sleep(u32 ms)
{
	struct timespec ms_ts;
	ms_ts.tv_sec = ms/1000;
	ms_ts.tv_nsec = (ms % 1000)*1000000;
	nanosleep(&ms_ts, NULL);
}

char* vmem_reserve(size_t size)
{
	return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void vmem_release(char* addr, size_t size)
{
	munmap(addr, size);
}

void thread_start(runtime_thread_func f, server_thread_t* handle, void* arg)
{
	pthread_create(handle, NULL, (void* (*)(void*)) f, arg);
}

void thread_exit()
{
	
}

void thread_join(server_thread_t* handle)
{
	pthread_join(*handle, NULL);
}

void rwlock_init(server_rwlock_t* rwlock)
{
	pthread_rwlock_init(rwlock, NULL);
}

void rwlock_lock_read(server_rwlock_t* rwlock)
{
	pthread_rwlock_rdlock(rwlock);
}

void rwlock_unlock_read(server_rwlock_t* rwlock)
{
	pthread_rwlock_unlock(rwlock);
}

void rwlock_lock_write(server_rwlock_t* rwlock)
{
	pthread_rwlock_wrlock(rwlock);
}

void rwlock_unlock_write(server_rwlock_t* rwlock)
{
	pthread_rwlock_unlock(rwlock);
}

void rwlock_destroy(server_rwlock_t* rwlock)
{
	pthread_rwlock_destroy(rwlock);
}

#endif