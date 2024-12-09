#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#define USERPROG
#include "threads/thread.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

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

bool user_to_kernel(void *kernel_buffer, const void *user_buffer, size_t size) {
    if (kernel_buffer == NULL || user_buffer == NULL) {
        return false; 
    }

    const uint8_t *ubuf = user_buffer;
    uint8_t *kbuf = kernel_buffer;

    for (size_t i = 0; i < size; i++) {
        if (!is_user_vaddr(ubuf + i)) {
            return false; 
        }

        void *page = pagedir_get_page(thread_current()->pagedir, ubuf + i);
        if (page == NULL) {
            return false; 
        }

        kbuf[i] = *((uint8_t *)page);
    }
    return true;
}

bool kernel_to_user(void *user_buffer, const void *kernel_buffer, size_t size){
 if (kernel_buffer == NULL || user_buffer == NULL) {
        return false; 
    }

    uint8_t *ubuf = user_buffer;
    const uint8_t *kbuf = kernel_buffer;

    for (size_t i = 0; i < size; i++) {
        if (!is_user_vaddr(ubuf + i)) {
            return false; 
        }

        void *page = pagedir_get_page(thread_current()->pagedir, ubuf + i);
        if (page == NULL) {
            return false; 
        }

        *((uint8_t *)page) = kbuf[i];
    }
    return true;
}