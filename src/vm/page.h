#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "filesys/file.h"
#include "filesys/off_t.h"

#define PAGE_ZERO 0
#define PAGE_FRAME 1
#define PAGE_FILE 2
#define PAGE_SWAP 3
// File: vm/page.h
struct supplemental_page_entry
{
  struct file *associated_file;    // Archivo asociado.
  off_t file_offset;               // Offset en el archivo.
  uint32_t bytes_to_read;          // Bytes que deben leerse del archivo.
  uint32_t bytes_to_zero;          // Bytes que deben inicializarse a cero.
  bool is_writable;                // Indica si la página es escribible.
  void *physical_page;             // Dirección física de la página cargada (si aplica).
  enum page_status current_status; // Estado actual de la página.
};

void init_spt(struct hash *);
void destroy_spt(struct hash *);
void init_spte(struct hash *, void *, void *);
void init_zero_spte(struct hash *, void *);
void init_frame_spte(struct hash *, void *, void *);
struct spte *init_file_spte(struct hash *, void *, struct file *, off_t, uint32_t, uint32_t, bool);
bool load_page(struct hash *, void *);
struct spte *get_spte(struct hash *, void *);
void page_delete(struct hash *spt, struct spte *entry);

#endif
