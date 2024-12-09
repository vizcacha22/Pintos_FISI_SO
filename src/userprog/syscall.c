#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#define USERPROG
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/input.h"
#include "userprog/pagedir.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include <string.h>


static void syscall_handler (struct intr_frame *);

void
syscall_init (void){
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED){
  int syscall_number;
  user_to_kernel(&syscall_number, (void *) f->esp, sizeof(int));
  switch (syscall_number) {
      case SYS_WRITE: {
          int fd;
          const void *buffer;
          unsigned size;
          user_to_kernel(&fd, (void *) (f->esp + 4), sizeof(int));
          user_to_kernel(&buffer, (void *) (f->esp + 8), sizeof(void *));
          user_to_kernel(&size, (void *) (f->esp + 12), sizeof(unsigned));
          if (!validation_user_pointer(buffer)) {
              exit(-1);
          }
          f->eax = write(fd, buffer, size);
          break;
      }
      default:
          exit(-1); 
  }
}

bool validation_user_pointer(const void *ptr){
    return (ptr != NULL && is_user_vaddr(ptr) && 
            pagedir_get_page(thread_current()->pagedir, ptr) != NULL);
}

bool validate_user_range(const void *user_buffer, size_t size) {
    const uint8_t *start = (const uint8_t *)user_buffer;
    const uint8_t *end = start + size;
    while (start < end) {
        if (!is_user_vaddr(start) || pagedir_get_page(thread_current()->pagedir, start) == NULL) {
            return false;
        }
        start = pg_round_up(start + 1);
    }
    return true;
}

bool user_to_kernel(void *kernel_buffer, const void *user_buffer, size_t size) {
    if (!validate_user_range(user_buffer, size)) {
        return false; 
    }
    memcpy(kernel_buffer, user_buffer, size);
    return true;
}

bool validate_user_range(const void *user_buffer, size_t size) {
    const uint8_t *start = (const uint8_t *)user_buffer;
    const uint8_t *end = start + size;
    while (start < end) {
        if (!is_user_vaddr(start) || pagedir_get_page(thread_current()->pagedir, start) == NULL) {
            return false;
        }
        start = pg_round_up(start + 1);
    }
    return true;
}