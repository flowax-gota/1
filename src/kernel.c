/**
 * kernel.c - Ядро гибридной ОС essentedOS
 * 
 * Главное ядро объединяет:
 * - Mach microkernel (macOS style)
 * - Linux POSIX compatibility layer  
 * - Windows NT process management
 * 
 * @author essentedOS Development Team
 * @version 1.0
 */

#include "essented.h"
#include <string.h>
#include <stdbool.h>

// ============================================================================
// [ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ЯДРА / KERNEL GLOBAL VARIABLES]
// ============================================================================

// Таблица процессов / Process table
nt_process_t g_process_table[MAX_PROCESSES];
uint32_t g_process_count = 0;

// Флаг работы ядра / Kernel running flag
static bool g_kernel_running = false;

// Счетчик тиков / Tick counter
static uint64_t g_tick_count = 0;

// ============================================================================
// [ИНИЦИАЛИЗАЦИЯ КОМПОНЕНТОВ / COMPONENT INITIALIZATION]
// ============================================================================

/**
 * Инициализация таблицы процессов
 * Initialize process table
 */
static void init_process_table(void) {
    // Очищаем таблицу процессов / Clear process table
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        g_process_table[i].pid = 0;
        g_process_table[i].ppid = 0;
        g_process_table[i].name[0] = '\0';
        g_process_table[i].state = PROCESS_STATE_IDLE;
        g_process_table[i].type = PROCESS_TYPE_MACH;
        g_process_table[i].priority = 0;
        g_process_table[i].create_time = 0;
        g_process_table[i].cpu_time = 0;
        g_process_table[i].page_dir = NULL;
        g_process_table[i].stack_base = NULL;
        g_process_table[i].entry_point = NULL;
    }
    g_process_count = 0;
}

/**
 * Создание системного процесса idle
 * Create system idle process
 * 
 * @return PID созданного процесса / Created process PID
 */
static uint32_t create_idle_process(void) {
    return process_create("System Idle", PROCESS_TYPE_MACH);
}

/**
 * Создание процесса init (POSIX)
 * Create init process (POSIX)
 * 
 * @return PID созданного процесса / Created process PID
 */
static uint32_t create_init_process(void) {
    uint32_t pid = process_create("init", PROCESS_TYPE_LINUX_POSIX);
    if (pid > 0) {
        g_process_table[pid - 1].ppid = 0;  // Init не имеет родителя / Init has no parent
        g_process_table[pid - 1].priority = 100;  // Высокий приоритет / High priority
    }
    return pid;
}

/**
 * Создание процесса System (NT)
 * Create System process (NT)
 * 
 * @return PID созданного процесса / Created process PID
 */
static uint32_t create_system_process(void) {
    uint32_t pid = process_create("System", PROCESS_TYPE_NT_WIN32);
    if (pid > 0) {
        g_process_table[pid - 1].ppid = 0;
        g_process_table[pid - 1].priority = 31;  // Максимальный приоритет NT / Max NT priority
    }
    return pid;
}

/**
 * Инициализация VFS (Virtual File System)
 * Initialize Virtual File System
 */
static void init_vfs(void) {
    // Создаем корневую директорию / Create root directory
    vfs_node_t* root = vfs_create_node("/", VFS_NODE_DIRECTORY);
    if (root) {
        root->permissions = 0755;
        root->uid = 0;
        root->gid = 0;
    }
    
    // Создаем стандартные директории Linux
    // Create standard Linux directories
    vfs_create_node("/bin", VFS_NODE_DIRECTORY);
    vfs_create_node("/etc", VFS_NODE_DIRECTORY);
    vfs_create_node("/home", VFS_NODE_DIRECTORY);
    vfs_create_node("/tmp", VFS_NODE_DIRECTORY);
    vfs_create_node("/var", VFS_NODE_DIRECTORY);
    vfs_create_node("/dev", VFS_NODE_DEVICE);
    
    // Создаем директорию Programs (Windows style)
    // Create Programs directory (Windows style)
    vfs_node_t* programs = vfs_create_node("/Programs", VFS_NODE_DIRECTORY);
    if (programs) {
        programs->permissions = 0755;
    }
}

/**
 * Инициализация Mach IPC (Inter-Process Communication)
 * Initialize Mach IPC
 */
static void init_mach_ipc(void) {
    // В реальной реализации здесь создаются порты Mach
    // In real implementation, Mach ports would be created here
    // Для демонстрации просто выводим сообщение
    // For demonstration, just output message
}

// ============================================================================
// [ФУНКЦИИ УПРАВЛЕНИЯ ПРОЦЕССАМИ / PROCESS MANAGEMENT FUNCTIONS]
// ============================================================================

/**
 * Создание нового процесса
 * Create new process
 * 
 * @param name Имя процесса / Process name
 * @param type Тип процесса / Process type
 * @return PID процесса или 0 при ошибке / Process PID or 0 on error
 */
uint32_t process_create(const char* name, hybrid_process_type_t type) {
    if (!name || g_process_count >= MAX_PROCESSES) {
        return 0;
    }
    
    // Находим свободный слот / Find free slot
    uint32_t pid = 0;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (g_process_table[i].state == PROCESS_STATE_IDLE) {
            pid = i + 1;  // PID начинается с 1 / PID starts from 1
            break;
        }
    }
    
    if (pid == 0) {
        return 0;  // Нет свободных слотов / No free slots
    }
    
    // Инициализируем процесс / Initialize process
    nt_process_t* proc = &g_process_table[pid - 1];
    
    // Копируем имя / Copy name
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->name[sizeof(proc->name) - 1] = '\0';
    
    // Устанавливаем параметры / Set parameters
    proc->pid = pid;
    proc->ppid = 1;  // По умолчанию родитель - init / Default parent is init
    proc->type = type;
    proc->state = PROCESS_STATE_READY;
    proc->priority = (type == PROCESS_TYPE_NT_WIN32) ? 8 : 10;
    proc->create_time = g_tick_count;
    proc->cpu_time = 0;
    proc->page_dir = NULL;
    proc->stack_base = NULL;
    proc->entry_point = NULL;
    
    g_process_count++;
    
    return pid;
}

/**
 * Удаление процесса
 * Delete process
 * 
 * @param pid PID процесса / Process PID
 */
void process_delete(uint32_t pid) {
    if (pid == 0 || pid > MAX_PROCESSES) {
        return;
    }
    
    nt_process_t* proc = &g_process_table[pid - 1];
    
    if (proc->state != PROCESS_STATE_IDLE) {
        // Освобождаем ресурсы / Free resources
        proc->page_dir = NULL;
        proc->stack_base = NULL;
        proc->entry_point = NULL;
        
        // Помечаем как завершенный / Mark as terminated
        proc->state = PROCESS_STATE_TERMINATED;
        
        // Очищаем структуру / Clear structure
        proc->pid = 0;
        proc->ppid = 0;
        proc->name[0] = '\0';
        proc->type = PROCESS_TYPE_MACH;
        proc->priority = 0;
        proc->create_time = 0;
        proc->cpu_time = 0;
        
        g_process_count--;
    }
}

/**
 * Переключение контекста процесса
 * Context switch
 * 
 * @param pid PID процесса / Process PID
 */
void process_switch(uint32_t pid) {
    if (pid == 0 || pid > MAX_PROCESSES) {
        return;
    }
    
    nt_process_t* proc = &g_process_table[pid - 1];
    
    if (proc->state == PROCESS_STATE_READY) {
        // Сохраняем контекст текущего процесса
        // Save current process context (реализация зависит от архитектуры)
        // Implementation depends on architecture
        
        // Загружаем контекст нового процесса
        // Load new process context
        
        proc->state = PROCESS_STATE_RUNNING;
    }
}

/**
 * Планировщик процессов (Round Robin)
 * Process scheduler (Round Robin)
 */
void process_schedule(void) {
    static uint32_t last_pid = 0;
    
    // Простой round-robin планировщик
    // Simple round-robin scheduler
    
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        uint32_t pid = ((last_pid + i) % MAX_PROCESSES) + 1;
        nt_process_t* proc = &g_process_table[pid - 1];
        
        if (proc->state == PROCESS_STATE_READY) {
            process_switch(pid);
            last_pid = pid;
            return;
        }
    }
    
    // Если нет готовых процессов, запускаем idle
    // If no ready processes, run idle
    process_switch(1);  // PID 1 - System Idle
}

// ============================================================================
// [ФУНКЦИИ VFS / VFS FUNCTIONS]
// ============================================================================

/**
 * Создание узла VFS
 * Create VFS node
 * 
 * @param path Путь к узлу / Path to node
 * @param type Тип узла / Node type
 * @return Указатель на узел или NULL / Pointer to node or NULL
 */
vfs_node_t* vfs_create_node(const char* path, vfs_node_type_t type) {
    if (!path) {
        return NULL;
    }
    
    // В упрощенной реализации просто выделяем память
    // In simplified implementation, just allocate memory
    // В реальном ядре здесь была бы сложная логика
    // In real kernel there would be complex logic
    
    static vfs_node_t g_vfs_nodes[100];
    static uint32_t g_vfs_node_count = 0;
    
    if (g_vfs_node_count >= 100) {
        return NULL;
    }
    
    vfs_node_t* node = &g_vfs_nodes[g_vfs_node_count++];
    
    // Извлекаем имя из пути / Extract name from path
    const char* name_start = strrchr(path, '/');
    if (name_start) {
        name_start++;
    } else {
        name_start = path;
    }
    
    strncpy(node->name, name_start, VFS_MAX_FILENAME - 1);
    node->name[VFS_MAX_FILENAME - 1] = '\0';
    
    node->type = type;
    node->size = 0;
    node->permissions = 0644;
    node->uid = 0;
    node->gid = 0;
    node->atime = g_tick_count;
    node->mtime = g_tick_count;
    node->ctime = g_tick_count;
    node->parent = NULL;
    node->children = NULL;
    node->next = NULL;
    node->data = NULL;
    
    return node;
}

/**
 * Поиск узла VFS
 * Find VFS node
 * 
 * @param path Путь к узлу / Path to node
 * @return Указатель на узел или NULL / Pointer to node or NULL
 */
vfs_node_t* vfs_find_node(const char* path) {
    // Упрощенная реализация / Simplified implementation
    // В реальном ядре здесь был бы поиск по дереву
    // In real kernel there would be tree search
    
    if (!path || strcmp(path, "/") == 0) {
        // Возвращаем корень / Return root
        static vfs_node_t root = {
            .name = "/",
            .type = VFS_NODE_DIRECTORY,
            .permissions = 0755
        };
        return &root;
    }
    
    return NULL;  // Не найдено / Not found
}

/**
 * Удаление узла VFS
 * Delete VFS node
 * 
 * @param node Указатель на узел / Pointer to node
 */
void vfs_delete_node(vfs_node_t* node) {
    if (!node) {
        return;
    }
    
    // Рекурсивное удаление дочерних элементов
    // Recursive deletion of children
    vfs_node_t* child = node->children;
    while (child) {
        vfs_node_t* next = child->next;
        vfs_delete_node(child);
        child = next;
    }
    
    // Освобождение данных / Free data
    if (node->data) {
        // free(node->data);  // В реальной реализации
        node->data = NULL;
    }
}

/**
 * Чтение из узла VFS
 * Read from VFS node
 * 
 * @param node Узел для чтения / Node to read from
 * @param buffer Буфер для данных / Buffer for data
 * @param size Размер данных / Data size
 * @param offset Смещение / Offset
 * @return Количество прочитанных байт / Number of bytes read
 */
int vfs_read(vfs_node_t* node, void* buffer, uint64_t size, uint64_t offset) {
    if (!node || !buffer || node->type == VFS_NODE_DIRECTORY) {
        return -1;
    }
    
    // В реальной реализации здесь было бы чтение данных
    // In real implementation there would be data reading
    return 0;
}

/**
 * Запись в узел VFS
 * Write to VFS node
 * 
 * @param node Узел для записи / Node to write to
 * @param buffer Данные для записи / Data to write
 * @param size Размер данных / Data size
 * @param offset Смещение / Offset
 * @return Количество записанных байт / Number of bytes written
 */
int vfs_write(vfs_node_t* node, const void* buffer, uint64_t size, uint64_t offset) {
    if (!node || !buffer || node->type == VFS_NODE_DIRECTORY) {
        return -1;
    }
    
    // В реальной реализации здесь была бы запись данных
    // In real implementation there would be data writing
    return 0;
}

// ============================================================================
// [ОБРАБОТЧИКИ ПРЕРЫВАНИЙ / INTERRUPT HANDLERS]
// ============================================================================

/**
 * Обработчик прерываний ядра
 * Kernel interrupt handler
 * 
 * @param irq Номер прерывания / IRQ number
 */
void kernel_interrupt_handler(uint8_t irq) {
    switch (irq) {
        case 0:  // Timer interrupt
            g_tick_count++;
            
            // Вызываем планировщик каждые N тиков
            // Call scheduler every N ticks
            if (g_tick_count % 10 == 0) {
                process_schedule();
            }
            break;
            
        case 1:  // Keyboard interrupt
            // Обработка ввода с клавиатуры
            // Keyboard input handling
            break;
            
        default:
            // Другие прерывания / Other interrupts
            break;
    }
}

// ============================================================================
// [ИНИЦИАЛИЗАЦИЯ И ГЛАВНЫЙ ЦИКЛ ЯДРА / KERNEL INIT AND MAIN LOOP]
// ============================================================================

/**
 * Инициализация ядра
 * Kernel initialization
 */
void kernel_init(void) {
    // Инициализация VGA / Initialize VGA
    vga_init();
    
    // Вывод приветственного сообщения
    // Output welcome message
    vga_putstr_at(10, ROW_TERMINAL_START + 1, 
        "========================================", 0x0F);
    vga_putstr_at(15, ROW_TERMINAL_START + 2,
        "essentedOS - Hybrid Operating System", 0x0A);
    vga_putstr_at(10, ROW_TERMINAL_START + 3,
        "========================================", 0x0F);
    vga_putstr_at(5, ROW_TERMINAL_START + 5,
        "[Mach Microkernel] + [Linux POSIX] + [Windows NT]", 0x07);
    vga_putstr_at(5, ROW_TERMINAL_START + 6,
        "Version 1.0 | Build 2024", 0x07);
    vga_putstr_at(5, ROW_TERMINAL_START + 8,
        "Initializing core components...", 0x07);
    
    // Инициализация таблицы процессов
    // Initialize process table
    init_process_table();
    vga_putstr_at(5, ROW_TERMINAL_START + 9,
        "[OK] Process table initialized", 0x0A);
    
    // Создание системных процессов
    // Create system processes
    create_idle_process();
    vga_putstr_at(5, ROW_TERMINAL_START + 10,
        "[OK] System Idle process created (PID: 1)", 0x0A);
    
    create_init_process();
    vga_putstr_at(5, ROW_TERMINAL_START + 11,
        "[OK] Init process created (PID: 2)", 0x0A);
    
    create_system_process();
    vga_putstr_at(5, ROW_TERMINAL_START + 12,
        "[OK] System NT process created (PID: 3)", 0x0A);
    
    // Инициализация VFS
    // Initialize VFS
    init_vfs();
    vga_putstr_at(5, ROW_TERMINAL_START + 13,
        "[OK] Virtual File System initialized", 0x0A);
    
    // Инициализация Mach IPC
    // Initialize Mach IPC
    init_mach_ipc();
    vga_putstr_at(5, ROW_TERMINAL_START + 14,
        "[OK] Mach IPC initialized", 0x0A);
    
    vga_putstr_at(5, ROW_TERMINAL_START + 16,
        "System ready. Press any key...", 0x0E);
    
    // Устанавливаем флаг работы
    // Set running flag
    g_kernel_running = true;
}

/**
 * Главный цикл ядра
 * Kernel main loop
 */
void kernel_main_loop(void) {
    while (g_kernel_running) {
        // Обработка прерываний / Handle interrupts
        // В реальном ядре здесь был бы цикл ожидания прерываний
        // In real kernel there would be interrupt wait loop
        
        // Обновление системной информации / Update system info
        static uint64_t update_counter = 0;
        update_counter++;
        
        if (update_counter % 100 == 0) {
            // Обновление времени в меню
            // Update time in menu
            // (В реальной реализации)
        }
        
        // Планирование процессов / Schedule processes
        process_schedule();
        
        // Ожидание следующего тика / Wait for next tick
        // (В реальном ядре используется HLT инструкция)
    }
}

// ============================================================================
// [ТОЧКА ВХОДА / ENTRY POINT]
// ============================================================================

/**
 * Точка входа ядра
 * Kernel entry point
 */
void _start(void) {
    // Ассемблерная часть инициализации стека уже выполнена
    // Assembly stack initialization already done
    
    // Инициализация ядра / Initialize kernel
    kernel_init();
    
    // Главный цикл / Main loop
    kernel_main_loop();
    
    // Бесконечный цикл на случай выхода из main
    // Infinite loop in case of exit from main
    while (1) {
        // Halt instruction would be here
        // asm volatile("hlt");
    }
}
