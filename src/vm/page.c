#include "vm/page.h"
#include "threads/thread.h"
#include "vm/frame.h"
#include <string.h>
#include "threads/vaddr.h"

static hash_hash_func spt_hash_func;
static hash_less_func spt_less_func;
static void page_destutcor(struct hash_elem *elem, void *aux);
extern struct lock file_lock;

void init_spt(struct hash *spt)
{
  hash_init(spt, spt_hash_func, spt_less_func, NULL);
}

void destroy_spt(struct hash *spt)
{
  hash_destroy(spt, page_destutcor);
}

void init_spte(struct hash *spt, void *upage, void *kpage)
{
  struct spte *e;
  e = (struct spte *)malloc(sizeof *e);

  e->upage = upage;
  e->kpage = kpage;

  e->status = PAGE_FRAME;

  hash_insert(spt, &e->hash_elem);
}

void init_zero_spte(struct hash *spt, void *upage)
{
  struct spte *e;
  e = (struct spte *)malloc(sizeof *e);

  e->upage = upage;
  e->kpage = NULL;

  e->status = PAGE_ZERO;

  e->file = NULL;
  e->writable = true;

  hash_insert(spt, &e->hash_elem);
}

void init_frame_spte(struct hash *spt, void *upage, void *kpage)
{
  struct spte *e;
  e = (struct spte *)malloc(sizeof *e);

  e->upage = upage;
  e->kpage = kpage;

  e->status = PAGE_FRAME;

  e->file = NULL;
  e->writable = true;

  hash_insert(spt, &e->hash_elem);
}

struct supplemental_page_entry *
create_supplemental_page_entry(struct list *supplemental_table, struct file *file,
                               off_t offset, uint32_t read_size, uint32_t zero_size, bool writable)
{
  struct supplemental_page_entry *entry = malloc(sizeof(struct supplemental_page_entry));
  if (!entry)
    return NULL;

  // Inicializa la entrada con los parámetros proporcionados.
  entry->associated_file = file;
  entry->file_offset = offset;
  entry->bytes_to_read = read_size;
  entry->bytes_to_zero = zero_size;
  entry->is_writable = writable;
  entry->current_status = PAGE_FILE;
  entry->physical_page = NULL;

  // Inserta la entrada en la tabla suplementaria.
  list_push_back(supplemental_table, &entry->elem);
  return entry;
}

bool load_virtual_page(struct hash *spt, void *upage)
{
  struct supplemental_page_entry *entry = find_supplemental_page_entry(supplemental_table, virtual_address);
  if (!entry)
    return false;

  void *physical_memory_page = falloc_get_page(PAL_USER);
  if (!physical_memory_page)
    return false;

  if (!file_read_at(entry->associated_file, physical_memory_page, entry->bytes_to_read, entry->file_offset))
  {
    falloc_free_page(physical_memory_page);
    return false;
  }
  memset(physical_memory_page + entry->bytes_to_read, 0, entry->bytes_to_zero);

  if (!pagedir_set_page(thread_current()->pagedir, virtual_address, physical_memory_page, entry->is_writable))
  {
    falloc_free_page(physical_memory_page);
    return false;
  }

  entry->physical_page = physical_memory_page;
  entry->current_status = PAGE_FRAME;
  return true;
}

struct spte *
get_spte(struct hash *spt, void *upage)
{
  struct spte e;
  struct hash_elem *elem;

  e.upage = upage;
  elem = hash_find(spt, &e.hash_elem);

  return elem != NULL ? hash_entry(elem, struct spte, hash_elem) : NULL;
}

static unsigned
spt_hash_func(const struct hash_elem *elem, void *aux)
{
  struct spte *p = hash_entry(elem, struct spte, hash_elem);

  return hash_bytes(&p->upage, sizeof(p->kpage));
}

static bool
spt_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
  void *a_upage = hash_entry(a, struct spte, hash_elem)->upage;
  void *b_upage = hash_entry(b, struct spte, hash_elem)->upage;

  return a_upage < b_upage;
}

static void
page_destutcor(struct hash_elem *elem, void *aux)
{
  struct spte *e;

  e = hash_entry(elem, struct spte, hash_elem);

  free(e);
}

void page_delete(struct hash *spt, struct spte *entry)
{
  hash_delete(spt, &entry->hash_elem);
  free(entry);
}