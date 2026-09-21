/**
 * essented.h - Заголовочный файл гибридной ОС essentedOS
 * 
 * Гибридная архитектура combining:
 * - Mach microkernel (macOS style)
 * - Linux POSIX compatibility layer
 * - Windows NT process management structures
 * 
 * @author essentedOS Development Team
 * @version 1.0
 */

#ifndef ESSENTED_H
#define ESSENTED_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// [КОНСТАНТЫ VGA РЕЖИМА / VGA MODE CONSTANTS]
// ============================================================================

#define VGA_WIDTH 80                    // Ширина экрана в символах / Screen width in characters
#define VGA_HEIGHT 25                   // Высота экрана в символах / Screen height in characters
#define VGA_MEMORY_ADDR 0xB8000         // Адрес видеопамяти / Video memory address
#define VGA_ATTR_DEFAULT 0x07           // Белый текст на черном фоне / White on black

// Строки интерфейса / Interface rows
#define ROW_MENU_BAR 0                  // Верхнее меню macOS / macOS menu bar
#define ROW_TERMINAL_START 2            // Начало терминала Linux / Linux terminal start
#define ROW_TERMINAL_END 22             // Конец терминала Linux / Linux terminal end
#define ROW_DOCK 24                     // Нижний Dock / Bottom Dock

// ============================================================================
// [СТРУКТУРЫ ПРОЦЕССОВ WINDOWS NT STYLE / WINDOWS NT PROCESS STRUCTURES]
// ============================================================================

// Состояния процесса NT / NT Process states
typedef enum {
    PROCESS_STATE_IDLE = 0,             // Бездействует / Idle
    PROCESS_STATE_READY,                // Готов к выполнению / Ready
    PROCESS_STATE_RUNNING,              // Выполняется / Running
    PROCESS_STATE_WAITING,              // Ожидание / Waiting
    PROCESS_STATE_TERMINATED            // Завершен / Terminated
} nt_process_state_t;

// Типы процессов в гибридной системе / Process types in hybrid system
typedef enum {
    PROCESS_TYPE_MACH = 0,              // Mach microkernel task
    PROCESS_TYPE_LINUX_POSIX,           // Linux POSIX process
    PROCESS_TYPE_NT_WIN32               // Windows NT Win32 process
} hybrid_process_type_t;

// Структура процесса Windows NT / Windows NT Process Structure
typedef struct {
    uint32_t pid;                       // Process ID
    uint32_t ppid;                      // Parent Process ID
    char name[64];                      // Имя процесса / Process name
    nt_process_state_t state;           // Состояние / State
    hybrid_process_type_t type;         // Тип процесса / Process type
    uint32_t priority;                  // Приоритет / Priority
    uint64_t create_time;               // Время создания / Creation time
    uint64_t cpu_time;                  // CPU время / CPU time
    void* page_dir;                     // Page directory pointer
    void* stack_base;                   // Base of stack
    void* entry_point;                  // Entry point address
} nt_process_t;

// Таблица процессов / Process table
#define MAX_PROCESSES 256               // Максимум процессов / Maximum processes
extern nt_process_t g_process_table[MAX_PROCESSES];
extern uint32_t g_process_count;

// ============================================================================
// [VFS LINUX STRUCTURES / СТРУКТУРЫ ВИРТУАЛЬНОЙ ФС LINUX]
// ============================================================================

#define VFS_MAX_PATH 256                // Максимальная длина пути / Max path length
#define VFS_MAX_FILENAME 64             // Максимальная длина имени файла / Max filename length

// Типы узлов файловой системы / Filesystem node types
typedef enum {
    VFS_NODE_FILE = 0,                  // Обычный файл / Regular file
    VFS_NODE_DIRECTORY,                 // Директория / Directory
    VFS_NODE_SYMLINK,                   // Символическая ссылка / Symbolic link
    VFS_NODE_DEVICE                     // Устройство / Device
} vfs_node_type_t;

// Узел VFS / VFS Node structure
typedef struct vfs_node {
    char name[VFS_MAX_FILENAME];        // Имя файла / Filename
    vfs_node_type_t type;               // Тип узла / Node type
    uint64_t size;                      // Размер / Size
    uint32_t permissions;               // Права доступа / Permissions
    uint32_t uid;                       // User ID
    uint32_t gid;                       // Group ID
    uint64_t atime;                     // Last access time
    uint64_t mtime;                     // Last modification time
    uint64_t ctime;                     // Last change time
    struct vfs_node* parent;            // Родительская директория / Parent directory
    struct vfs_node* children;          // Дочерние элементы / Children
    struct vfs_node* next;              // Следующий sibling / Next sibling
    void* data;                         // Указатель на данные / Data pointer
} vfs_node_t;

// ============================================================================
// [MACH MICROKERNEL STRUCTURES / СТРУКТУРЫ MACH МИКРОЯДРА]
// ============================================================================

// Mach port rights / Права портов Mach
typedef enum {
    MACH_PORT_RIGHT_SEND = 1,           // Право отправки / Send right
    MACH_PORT_RIGHT_RECEIVE = 2,        // Право получения / Receive right
    MACH_PORT_RIGHT_SEND_ONCE = 3,      // Однократная отправка / Send once
    MACH_PORT_RIGHT_DEAD_NAME = 4       // Dead name right
} mach_port_right_t;

// Mach message header / Заголовок сообщения Mach
typedef struct {
    uint32_t msgh_bits;                 // Message bits
    uint32_t msgh_size;                 // Message size
    uint32_t msgh_remote_port;          // Remote port
    uint32_t msgh_local_port;           // Local port
    uint32_t msgh_reserved;             // Reserved
    int32_t msgh_id;                    // Message ID
} mach_msg_header_t;

// Mach task basic info / Базовая информация о задаче Mach
typedef struct {
    uint32_t suspend_count;             // Count suspends
    uint32_t base_priority;             // Base priority
    uint32_t virtual_size;              // Virtual size
    uint32_t resident_size;             // Resident size
    uint32_t user_time;                 // User CPU time
    uint32_t system_time;               // System CPU time
} mach_task_basic_info_t;

// ============================================================================
// [ТЕРМИНАЛ LINUX POSIX / LINUX POSIX TERMINAL]
// ============================================================================

#define TERMINAL_SCROLL_BUFFER_SIZE 1000  // Размер буфера скроллинга / Scroll buffer size
#define TERMINAL_INPUT_BUFFER_SIZE 256    // Размер буфера ввода / Input buffer size

// Структура терминала / Terminal structure
typedef struct {
    uint8_t cursor_x;                   // Позиция курсора X / Cursor X position
    uint8_t cursor_y;                   // Позиция курсора Y / Cursor Y position
    uint8_t scroll_offset;              // Смещение скролла / Scroll offset
    char input_buffer[TERMINAL_INPUT_BUFFER_SIZE];
    uint8_t input_pos;                  // Позиция ввода / Input position
    bool insert_mode;                   // Режим вставки / Insert mode
    uint8_t fg_color;                   // Цвет текста / Foreground color
    uint8_t bg_color;                   // Цвет фона / Background color
} terminal_t;

extern terminal_t g_terminal;

// ============================================================================
// [ФУНКЦИИ ЯДРА / KERNEL FUNCTIONS]
// ============================================================================

// Инициализация ядра / Kernel initialization
void kernel_init(void);

// Главный цикл ядра / Kernel main loop
void kernel_main_loop(void);

// Обработчик прерываний / Interrupt handler
void kernel_interrupt_handler(uint8_t irq);

// ============================================================================
// [ФУНКЦИИ VGA / VGA FUNCTIONS]
// ============================================================================

// Инициализация VGA / VGA initialization
void vga_init(void);

// Очистка экрана / Clear screen
void vga_clear_screen(void);

// Установка цвета пикселя / Set pixel color
void vga_set_pixel(int x, int y, uint8_t color);

// Вывод символа / Put character
void vga_putchar(char c);

// Вывод строки / Put string
void vga_putstr(const char* str);

// Вывод строки в позицию / Put string at position
void vga_putstr_at(int x, int y, const char* str, uint8_t attr);

// Отрисовка Dock панели / Draw Dock panel
void vga_draw_dock(void);

// Отрисовка верхнего меню / Draw menu bar
void vga_draw_menu_bar(void);

// Обновление экрана / Screen update
void vga_update(void);

// ============================================================================
// [ФУНКЦИИ УПРАВЛЕНИЯ ПРОЦЕССАМИ / PROCESS MANAGEMENT FUNCTIONS]
// ============================================================================

// Создание процесса / Create process
uint32_t process_create(const char* name, hybrid_process_type_t type);

// Удаление процесса / Delete process
void process_delete(uint32_t pid);

// Переключение контекста / Context switch
void process_switch(uint32_t pid);

// Планировщик процессов / Process scheduler
void process_schedule(void);

// ============================================================================
// [ФУНКЦИИ VFS / VFS FUNCTIONS]
// ============================================================================

// Создание узла / Create node
vfs_node_t* vfs_create_node(const char* path, vfs_node_type_t type);

// Поиск узла / Find node
vfs_node_t* vfs_find_node(const char* path);

// Удаление узла / Delete node
void vfs_delete_node(vfs_node_t* node);

// Чтение из узла / Read from node
int vfs_read(vfs_node_t* node, void* buffer, uint64_t size, uint64_t offset);

// Запись в узел / Write to node
int vfs_write(vfs_node_t* node, const void* buffer, uint64_t size, uint64_t offset);

#endif // ESSENTED_H
