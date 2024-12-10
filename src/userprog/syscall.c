#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#ifndef USERPROG
#define USERPROG
#endif
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include <string.h>  
#include <stdlib.h>

#define MAX_ARGS 3
#define MAX_FD_COUNT 128
#define STDOUT_FILENO 1

static void syscall_handler(struct intr_frame *);
static int sys_write(int fd, const void *buffer, unsigned size);
static void sys_exit(int status);

static struct lock filesys_lock;

void syscall_init(void){
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

static bool validate_user_addr(const void *addr){
  if (addr == NULL)
    return false;
  if (!is_user_vaddr(addr))
    return false;
  if (pagedir_get_page(thread_current()->pagedir, addr) == NULL)
    return false;

  return true;
}

static bool  validate_user_buffer(const void *buffer, unsigned size){
  unsigned i;
  char *local_buffer = (char *)buffer;
  
  for (i = 0; i < size; i++) {
    if (!validate_user_addr(local_buffer + i))
      return false;
  }
  return true;
}

static bool get_syscall_args(void *esp, uint32_t *args, int count){
  int i;
  for (i = 0; i < count; i++) {
    void *ptr = esp + sizeof(uint32_t) * (i + 1);
    if (!validate_user_addr(ptr))
      return false;
    args[i] = *(uint32_t *)ptr;
  }
  return true;
}

static void syscall_handler(struct intr_frame *f){
  uint32_t args[MAX_ARGS];
  if (!validate_user_addr(f->esp))
    sys_exit(-1);
  int syscall_number = *(int *)f->esp;
  switch (syscall_number) 
  {
    case SYS_EXIT:
      if (!get_syscall_args(f->esp, args, 1))
        sys_exit(-1);
      sys_exit((int)args[0]);
      break;
    case SYS_WRITE:
      if (!get_syscall_args(f->esp, args, 3))
        sys_exit(-1);
      f->eax = sys_write((int)args[0], (const void *)args[1], (unsigned)args[2]);
      break;
    default:
      printf("Unimplemented system call!\n");
      thread_exit();
  }
}

static void sys_exit(int status){
  struct thread *cur = thread_current();
  printf("%s: exit(%d)\n", cur->name, status);
  thread_exit();
}

static int sys_write(int fd, const void *buffer, unsigned size){
  if (!validate_user_buffer(buffer, size)) {
    sys_exit(-1);
  }
  if (fd == 0) { 
    return -1;
  }
  if (fd < 0 || fd >= MAX_FD_COUNT || thread_current()->fd_table[fd] == NULL) {
    return -1; 
  }
  if (fd == STDOUT_FILENO) {
    putbuf(buffer, size);
    return size;
  }
  struct file *file = thread_current()->fd_table[fd];
  lock_acquire(&filesys_lock);
  int bytes_written = file_write(file, buffer, size);
  lock_release(&filesys_lock);
  return bytes_written;
}

static bool validate_string(const char *str){
  if (!validate_user_addr(str))
    return false;
  while (true) {
    if (!validate_user_addr(str))
      return false;
    if (*str == '\0')
      break;
    str++;
  }
  return true;
}

static char *copy_in_string(const char *ustr){
  if (!validate_string(ustr))
    sys_exit(-1);
  size_t len = strlen(ustr) + 1;
  char *kstr = malloc(len);
  if (kstr == NULL)
    return NULL;
  strlcpy(kstr, ustr, len);
  return kstr;
}

static bool validate_user_page(const void *addr){
  void *page_start = pg_round_down(addr);
  if (!validate_user_addr(page_start))
    return false;
  return true;
}
