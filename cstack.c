#include "cstack.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(VAR) (void)(VAR)

#define MAX_STACKS 1000 // Максимальное число стеков, хранимых за раз

/* Структура элемента стека */ 
struct node
{
    const struct node* prev;
    unsigned int size;
    char data[0];
};
typedef struct node* stack_t; // Указатель на node (вершина стека)

/* Запись стека в глобальной таблице стеков */
struct stack_entry
{
    int reserved;
    int size;
    stack_t stack;
};
typedef struct stack_entry stack_entry_t; // Указатель на запись

/* Глобальная таблица стеков */
struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};
struct stack_entries_table g_table = {0u, NULL}; // Инициализация

/* Создаёт новый стек и возвращает его хэндлер (индекс в таблице) */
hstack_t stack_new(void)
{
    // Если таблица ещё не была выделена, выделяем её
    if (g_table.size == 0) {
        stack_entry_t *tmp = calloc(MAX_STACKS, sizeof(stack_entry_t));
        if (!tmp)
            return -1;
        g_table.entries = tmp;
        g_table.size = MAX_STACKS;
    }
    // Ищем первую свободную ячейку
    for (unsigned int i = 0; i < g_table.size; ++i) {
        if (g_table.entries[i].reserved == 0) {
            g_table.entries[i].reserved = 1;
            g_table.entries[i].size = 0;
            g_table.entries[i].stack = NULL;
            return (hstack_t)i;
        }
    }
    return -1; // Нет свободных стеков
}

/* Проверяет корректность хэндлера (0 - норм, 1 - не норм) */
int stack_valid_handler(const hstack_t hstack)
{
    if (g_table.entries == NULL)
        return 1;
    if (hstack < 0 || (unsigned int)hstack >= g_table.size)
        return 1;
    if (g_table.entries[hstack].reserved == 0)
        return 1;
    return 0;
}

/* Удаляет стек полностью */
void stack_free(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
        return;

    stack_t cur_stack = g_table.entries[hstack].stack;

    while (cur_stack) {
        stack_t prev = (stack_t)cur_stack->prev;
        free(cur_stack);
        cur_stack = prev;
    }

    g_table.entries[hstack].reserved = 0;
    g_table.entries[hstack].size = 0;
    g_table.entries[hstack].stack = NULL;
}

/* Возвращает количество элементов в стеке */
unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
        return 0; 

    return (unsigned)g_table.entries[hstack].size;
}

/* Добавляет данные в стек из буфера*/
void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (stack_valid_handler(hstack))
        return;

    if (!data_in || size == 0)
        return;

    stack_t new_node = malloc(sizeof(struct node) + size);
    if (!new_node)
        return;

    new_node->size = size;
    memcpy(new_node->data, data_in, size);
    new_node->prev = g_table.entries[hstack].stack;
    g_table.entries[hstack].stack = new_node;
    g_table.entries[hstack].size++;
}

/* Удаляет элемент из стека и переносит его в буфер. Возвращает количество записанных байт в data_out (0 - ошибка или мело места в буфере) */
unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (stack_valid_handler(hstack))
        return 0u;

    if (!data_out || size == 0u)
        return 0u;
    
    stack_t cur_stack = g_table.entries[(unsigned int)hstack].stack;
    if (cur_stack == NULL)
        return 0u;

    unsigned int node_size = cur_stack->size;

    if (size < node_size)
        return 0u;

    stack_t prev = (stack_t)cur_stack->prev;
    memcpy(data_out, cur_stack->data, node_size);
    free((void*)cur_stack);
    g_table.entries[hstack].stack = prev;
    g_table.entries[hstack].size--;
    return (unsigned)node_size;
}

