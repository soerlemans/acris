#ifndef CORE_H
#define CORE_H

/*!
 * @file
 *
 * Warning the C core is built without libc.
 */

// Core Includes:
#include "stdacris/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Core:
isize_t read(int_t t_fd, u8_t* t_buf, usize_t t_count);
/* isize_t write(int_t t_fd, const u8_t* t_buf, usize_t t_count); */
/* isize_t read_str(int_t t_fd, char* t_buf, usize_t t_count); */
isize_t write_str(int_t t_fd, const char* t_buf, usize_t t_count);
int_t open(const char* t_path, const int_t t_flags, const umode_t t_mode);
int_t close(int_t t_fd);
int_t getpid();
void exit(int_t t_errc);
int_t getppid();

usize_t strlen(const char* t_str);

int_t main();
void _start();

#ifdef __cplusplus
}
#endif

#endif // CORE_H
