#include <linux/slab.h>

struct page
{
    char data[PAGE_SIZE];
    struct page *next;
};

struct sequence
{
    struct page *head;
    struct page *tail;
    struct page *current;
};

void sequence_init(struct sequence *seq)
{
    seq->head = NULL;
    seq->tail = NULL;
    seq->current = NULL;
}

void sequence_destroy(struct sequence *seq)
{
    struct page *curr = seq->head;
    while (curr)
    {
        struct page *temp = curr;
        curr = curr->next;
        kfree(temp);
    }
    seq->head = NULL;
    seq->tail = NULL;
    seq->current = NULL;
}

int sequence_write(struct sequence *seq, const void *data, size_t length)
{
    if (length > PAGE_SIZE)
        return -1;

    struct page *new_page = kmalloc(sizeof(struct page), GFP_KERNEL);
    if (!new_page)
        return -1;

    memcpy(new_page->data, data, length);
    new_page->next = NULL;

    if (seq->head == NULL)
    {
        seq->head = new_page;
        seq->tail = new_page;
        seq->current = new_page;
    }
    else
    {
        seq->tail->next = new_page;
        seq->tail = new_page;
    }

    return 0;
}

int sequence_read(struct sequence *seq, void *buffer, size_t length)
{
    if (length > PAGE_SIZE)
        return -1;

    if (seq->current == NULL)
        seq->current = seq->head;

    if (copy_to_user(buffer, seq->current->data, length))
        return -EFAULT;

    seq->current = seq->current->next;
    if (seq->current == NULL)
        seq->current = seq->head;

    return 0;
}
