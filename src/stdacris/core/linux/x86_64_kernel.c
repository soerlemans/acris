#include "kernel.h"

#if !(defined(linux) || defined(__linux) || defined(__linux__) \
      || defined(__gnu_linux__))
#error "Syscalls in core are written for Linux."
#endif

#if !defined(__x86_64__)
#error "Syscalls in core are written for x86_64."
#endif

// Syscall numbers:
static const u64_t SYS_read = 0;
static const u64_t SYS_write = 1;
static const u64_t SYS_open = 2;
static const u64_t SYS_close = 3;
static const u64_t SYS_getpid = 39;
static const u64_t SYS_exit = 60;
static const u64_t SYS_getppid = 110;

isize_t read(int_t t_fd, u8_t* t_buf, usize_t t_count)
{
  isize_t err = 0;

  // clang-format off
  __asm__ volatile(
		"syscall\n"
		: "=a"(err)
		: "0"(SYS_read), "D"((u64_t)t_fd), "S"(t_buf), "d"(t_count)
		: "rcx", "r11", "memory" // clobbered registers.
  );
  // clang-format on

  return err;
}

isize_t write(int_t t_fd, const u8_t* t_buf, usize_t t_count)
{
  isize_t err = 0;

  // clang-format off
  __asm__ volatile(
		"syscall\n"
		: "=a"(err)
		: "0"(SYS_write), "D"((u64_t)t_fd), "S"(t_buf), "d"(t_count)
		: "rcx", "r11", "memory" // clobbered registers.
  );
  // clang-format on

  return err;
}

isize_t read_str(int_t t_fd, char* t_buf, usize_t t_count)
{
  return read(t_fd, (unsigned char*)t_buf, t_count);
}

isize_t write_str(int_t t_fd, const char* t_buf, usize_t t_count)
{
  return write(t_fd, (const unsigned char*)t_buf, t_count);
}

int_t open(const char* t_path, int_t t_flags, umode_t t_mode)
{
  int_t err = 0;

  // clang-format off
  __asm__ volatile(
		"syscall\n"
		: "=a"(err)
		: "0"(SYS_open), "D"(t_path), "S"(t_flags), "d"(t_mode)
		: "rcx", "r11", "memory" // clobbered registers.
  );
  // clang-format on

  return err;
}

int_t close(int_t t_fd)
{
  int_t err = 0;

  // clang-format off
  __asm__ volatile(
		"syscall\n"
		: "=a"(err)
		: "0"(SYS_close), "d"(t_fd)
		: "rcx", "r11", "memory" // clobbered registers.
  );
  // clang-format on

  return err;
}

int_t getpid()
{
  int_t pid = 0;

  // clang-format off
  __asm__ volatile(
    "syscall\n"
    : "=a"(pid)
    : "0"(SYS_getpid)
		: "rcx", "r11", "memory" // clobbered registers.
  );
  // clang-format on

  return pid;
}

void __attribute__((noreturn)) exit(int_t t_errc)
{
  // clang-format off
  __asm__ volatile(
    "syscall\n"
    :
    : "a"(SYS_exit), "D"(t_errc)
		: "rcx", "r11", "memory" // clobbered registers.
  );
  // clang-format on

  __builtin_unreachable();
}

int_t getppid()
{
  int_t pid = 0;

  // clang-format off
  __asm__ volatile(
    "syscall\n"
    : "=a"(pid)
    : "0"(SYS_getppid)
		: "rcx", "r11", "memory" // clobbered registers.
  );
  // clang-format on

  return pid;
}

usize_t strlen(const char* t_str)
{
  const char* ptr = t_str;
  while(*ptr != '\0') {
    ptr++;
  }

  return (usize_t)(ptr - t_str);
}

// Program entry without stdlib.
void _start()
{
  // Perform pre main setup.
  /* long* stack_ptr; */
  /* __asm__("mov %%rsp, %0" : "=r"(stack_ptr)); */

  /* // 2. Extract argc and argv */
  /* int argc = (int)stack_ptr[0]; */
  /* char** argv = (char**)&stack_ptr[1]; */

  /* if(argc > 1) { */
  /*   const char* print_stuff = "yes yes!"; */
  /*   write_str(1, print_stuff, strlen(print_stuff)); */
  /* } */

  const int_t errc = main();

  exit(errc);

  __builtin_unreachable();
}
