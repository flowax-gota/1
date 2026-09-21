/**
 * vga.c - VGA драйвер для гибридной ОС essentedOS
 * 
 * Реализует отрисовку интерфейса в текстовом режиме VGA:
 * - Верхнее меню в стиле macOS (строка 0)
 * - POSIX-терминал Linux с поддержкой скроллинга (строки 2-22)
 * - Dock панель в стиле macOS с системными иконками (строка 24)
 * 
 * @author essentedOS Development Team
 * @version 1.0
 */

#include "essented.h"
#include <string.h>

// ============================================================================
// [ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ / GLOBAL VARIABLES]
// ============================================================================

// Указатель на видеопамять / Video memory pointer
static uint16_t* g_vga_memory = (uint16_t*)VGA_MEMORY_ADDR;

// Текущий атрибут цвета / Current color attribute
static uint8_t g_vga_attr = VGA_ATTR_DEFAULT;

// Глобальный терминал / Global terminal
terminal_t g_terminal = {
    .cursor_x = 0,
    .cursor_y = ROW_TERMINAL_START,
    .scroll_offset = 0,
    .input_pos = 0,
    .insert_mode = false,
    .fg_color = 0x07,
    .bg_color = 0x00
};

// ============================================================================
// [ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ / HELPER FUNCTIONS]
// ============================================================================

/**
 * Вычисляет индекс в видеопамяти по координатам
 * Calculates video memory index from coordinates
 * 
 * @param x Координата X (0-79) / X coordinate (0-79)
 * @param y Координата Y (0-24) / Y coordinate (0-24)
 * @return Индекс в видеопамяти / Video memory index
 */
static inline size_t vga_get_index(int x, int y) {
    if (x < 0) x = 0;
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;
    return (size_t)(y * VGA_WIDTH + x);
}

/**
 * Устанавливает цвет атрибута VGA
 * Sets VGA attribute color
 * 
 * @param fg Цвет переднего плана (0-15) / Foreground color (0-15)
 * @param bg Цвет фона (0-15) / Background color (0-15)
 * @param bold Жирный текст / Bold text flag
 */
void vga_set_color(uint8_t fg, uint8_t bg, bool bold) {
    g_vga_attr = ((bg & 0x0F) << 4) | ((fg & 0x0F) | (bold ? 0x08 : 0x00));
}

// ============================================================================
// [ОСНОВНЫЕ ФУНКЦИИ VGA / MAIN VGA FUNCTIONS]
// ============================================================================

/**
 * Инициализация VGA режима
 * Initialize VGA mode
 */
void vga_init(void) {
    // Очищаем экран при инициализации / Clear screen on init
    vga_clear_screen();
    
    // Устанавливаем курсор в начало терминала / Set cursor to terminal start
    g_terminal.cursor_x = 0;
    g_terminal.cursor_y = ROW_TERMINAL_START;
    
    // Отрисовываем интерфейс / Draw interface
    vga_draw_menu_bar();
    vga_draw_dock();
}

/**
 * Очистка всего экрана
 * Clear entire screen
 */
void vga_clear_screen(void) {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        g_vga_memory[i] = (g_vga_attr << 8) | ' ';
    }
}

/**
 * Очистка конкретной строки
 * Clear specific row
 * 
 * @param y Номер строки / Row number
 */
static void vga_clear_row(int y) {
    for (int x = 0; x < VGA_WIDTH; x++) {
        size_t idx = vga_get_index(x, y);
        g_vga_memory[idx] = (g_vga_attr << 8) | ' ';
    }
}

/**
 * Вывод символа в текущую позицию курсора
 * Put character at current cursor position
 * 
 * @param c Символ для вывода / Character to print
 */
void vga_putchar(char c) {
    // Обработка специальных символов / Handle special characters
    switch (c) {
        case '\n':
            // Переход на новую строку / New line
            g_terminal.cursor_x = 0;
            g_terminal.cursor_y++;
            break;
            
        case '\r':
            // Возврат каретки / Carriage return
            g_terminal.cursor_x = 0;
            break;
            
        case '\t':
            // Табуляция / Tabulation
            g_terminal.cursor_x = (g_terminal.cursor_x + 8) & ~7;
            break;
            
        case '\b':
            // Backspace
            if (g_terminal.cursor_x > 0) {
                g_terminal.cursor_x--;
                size_t idx = vga_get_index(g_terminal.cursor_x, g_terminal.cursor_y);
                g_vga_memory[idx] = (g_vga_attr << 8) | ' ';
            }
            break;
            
        default:
            // Обычный символ / Regular character
            if (g_terminal.cursor_x < VGA_WIDTH) {
                size_t idx = vga_get_index(g_terminal.cursor_x, g_terminal.cursor_y);
                g_vga_memory[idx] = (g_vga_attr << 8) | (uint8_t)c;
                g_terminal.cursor_x++;
            }
            break;
    }
    
    // Проверка выхода за границы экрана / Check screen bounds
    if (g_terminal.cursor_x >= VGA_WIDTH) {
        g_terminal.cursor_x = 0;
        g_terminal.cursor_y++;
    }
    
    // Скроллинг терминала / Terminal scrolling
    if (g_terminal.cursor_y > ROW_TERMINAL_END) {
        // Сдвиг содержимого вверх / Scroll content up
        for (int y = ROW_TERMINAL_START; y < ROW_TERMINAL_END; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                size_t src_idx = vga_get_index(x, y + 1);
                size_t dst_idx = vga_get_index(x, y);
                g_vga_memory[dst_idx] = g_vga_memory[src_idx];
            }
        }
        
        // Очистка последней строки / Clear last row
        vga_clear_row(ROW_TERMINAL_END);
        g_terminal.cursor_y = ROW_TERMINAL_END;
        
        // Увеличение смещения скролла / Increase scroll offset
        if (g_terminal.scroll_offset < TERMINAL_SCROLL_BUFFER_SIZE) {
            g_terminal.scroll_offset++;
        }
    }
}

/**
 * Вывод строки
 * Put string
 * 
 * @param str Строка для вывода / String to print
 */
void vga_putstr(const char* str) {
    if (!str) return;
    while (*str) {
        vga_putchar(*str++);
    }
}

/**
 * Вывод строки в конкретную позицию
 * Put string at specific position
 * 
 * @param x Координата X / X coordinate
 * @param y Координата Y / Y coordinate
 * @param str Строка / String
 * @param attr Атрибут цвета / Color attribute
 */
void vga_putstr_at(int x, int y, const char* str, uint8_t attr) {
    if (!str || x < 0 || y < 0 || y >= VGA_HEIGHT) return;
    
    for (int i = 0; str[i] && x + i < VGA_WIDTH; i++) {
        size_t idx = vga_get_index(x + i, y);
        g_vga_memory[idx] = ((uint16_t)attr << 8) | (uint8_t)str[i];
    }
}

/**
 * Установка пикселя (в текстовом режиме - символа)
 * Set pixel (in text mode - character)
 * 
 * @param x Координата X / X coordinate
 * @param y Координата Y / Y coordinate
 * @param color Цвет / Color
 */
void vga_set_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    size_t idx = vga_get_index(x, y);
    g_vga_memory[idx] = ((uint16_t)color << 8) | (uint8_t)0xDB; // Full block character
}

/**
 * Обновление экрана (синхронизация)
 * Screen update (synchronization)
 */
void vga_update(void) {
    // В реальном оборудовании здесь была бы синхронизация
    // In real hardware this would synchronize with vertical blank
    // Для эмуляции просто оставляем как есть / For emulation, leave as is
}

// ============================================================================
// [ОТРИСОВКА DOCK ПАНЕЛИ / DOCK PANEL DRAWING]
// ============================================================================

/**
 * Отрисовка Dock панели в стиле macOS на строке 24
 * Draw macOS-style Dock panel on row 24
 * 
 * Использует псевдографику и Unicode-символы для создания
 * реалистичного интерфейса с системными иконками.
 * 
 * Uses pseudographics and Unicode symbols to create
 * realistic interface with system icons.
 */
void vga_draw_dock(void) {
    // ========================================================================
    // [КОНФИГУРАЦИЯ DOCK / DOCK CONFIGURATION]
    // ========================================================================
    
    const int dock_row = ROW_DOCK;              // Строка Dock / Dock row
    const int dock_start_x = 5;                 // Начальная позиция X / Start X position
    const int dock_end_x = VGA_WIDTH - 5;       // Конечная позиция X / End X position
    const uint8_t dock_bg_attr = 0x70;          // Белый текст на сером фоне / White on gray
    const uint8_t dock_border_attr = 0x0F;      // Яркий белый / Bright white
    const uint8_t icon_attr = 0x0E;             // Желтый для иконок / Yellow for icons
    
    // ========================================================================
    // [СТРУКТУРА ИКОНОК / ICON STRUCTURE]
    // ========================================================================
    
    // Определяем системные иконки и их названия
    // Define system icons and their names
    
    typedef struct {
        const char* icon;           // Unicode/ASCII символ иконки / Icon symbol
        const char* name;           // Название приложения / Application name
        int width;                  // Ширина элемента / Element width
    } dock_icon_t;
    
    // Массив иконок Dock (5 системных приложений)
    // Dock icons array (5 system applications)
    static const dock_icon_t icons[] = {
        {"[M]", "Мой ПК", 12},      // 🖥️ Этот Компьютер / This PC
        {"[F]", "Проводник", 14},   // 📁 Finder/Explorer
        {"[S]", "Настройки", 14},   // ⚙️ System Preferences
        {"[A]", "Диспетчер", 14},   // 📊 Activity Monitor
        {"[B]", "Корзина", 12}      // 🗑️ Recycle Bin
    };
    
    const int icon_count = 5;       // Количество иконок / Number of icons
    const char separator = '|';     // Разделитель между иконками / Separator between icons
    
    // ========================================================================
    // [РАСЧЕТ ПОЗИЦИОНИРОВАНИЯ / POSITIONING CALCULATION]
    // ========================================================================
    
    // Вычисляем общую ширину контента Dock
    // Calculate total width of Dock content
    
    int total_content_width = 0;
    for (int i = 0; i < icon_count; i++) {
        total_content_width += icons[i].width;
    }
    // Добавляем разделители (их на 1 меньше чем иконок)
    // Add separators (one less than icons)
    total_content_width += (icon_count - 1) * 3; // " | " = 3 символа
    
    // Центрируем контент внутри Dock
    // Center content inside Dock
    int dock_width = dock_end_x - dock_start_x + 1;
    int start_content_x = dock_start_x + (dock_width - total_content_width) / 2;
    
    // ========================================================================
    // [ОТРИСОВКА РАМКИ DOCK / DRAW DOCK BORDER]
    // ========================================================================
    
    // Рисуем верхнюю границу (если нужно) / Draw top border (if needed)
    // Для простоты рисуем только боковые рамки
    // For simplicity, draw only side borders
    
    // Левая граница / Left border - используем символ ╋
    size_t left_idx = vga_get_index(dock_start_x, dock_row);
    g_vga_memory[left_idx] = (dock_border_attr << 8) | (uint8_t)0x0B; // ┫
    
    // Правая граница / Right border - используем символ ╋
    size_t right_idx = vga_get_index(dock_end_x, dock_row);
    g_vga_memory[right_idx] = (dock_border_attr << 8) | (uint8_t)0x0B; // ┫
    
    // Альтернативный вариант с углами:
    // Alternative version with corners:
    // g_vga_memory[left_idx] = (dock_border_attr << 8) | (uint8_t)0xDA; // ┌
    // g_vga_memory[right_idx] = (dock_border_attr << 8) | (uint8_t)0xBF; // ┐
    
    // ========================================================================
    // [ОТРИСОВКА ФОНА DOCK / DRAW DOCK BACKGROUND]
    // ========================================================================
    
    // Заполняем фон Dock панели серым цветом
    // Fill Dock panel background with gray color
    for (int x = dock_start_x + 1; x < dock_end_x; x++) {
        size_t idx = vga_get_index(x, dock_row);
        g_vga_memory[idx] = (dock_bg_attr << 8) | ' ';
    }
    
    // ========================================================================
    // [ОТРИСОВКА ИКОНОК И ТЕКСТА / DRAW ICONS AND TEXT]
    // ========================================================================
    
    int current_x = start_content_x;
    
    for (int i = 0; i < icon_count; i++) {
        // --- Отрисовка иконки / Draw icon ---
        // Рисуем символ иконки в квадратных скобках
        // Draw icon symbol in square brackets
        
        // Позиция иконки / Icon position
        int icon_x = current_x;
        
        // Формируем строку иконки: [X] имя
        // Build icon string: [X] name
        char icon_str[20];
        
        // Копируем символ иконки (например, [M])
        // Copy icon symbol (e.g., [M])
        int pos = 0;
        icon_str[pos++] = ' ';  // Пробел перед иконкой / Space before icon
        
        // Копируем символы иконки
        for (int j = 0; icons[i].icon[j] && j < 3; j++) {
            icon_str[pos++] = icons[i].icon[j];
        }
        
        icon_str[pos++] = ' ';  // Пробел после иконки / Space after icon
        
        // Копируем название приложения
        // Copy application name
        for (int j = 0; icons[i].name[j] && pos < 18; j++) {
            icon_str[pos++] = icons[i].name[j];
        }
        
        icon_str[pos] = '\0';
        
        // Выводим иконку с именем желтым цветом
        // Output icon with name in yellow
        vga_putstr_at(icon_x, dock_row, icon_str, icon_attr);
        
        // Перемещаем курсор на ширину элемента
        // Move cursor by element width
        current_x += icons[i].width;
        
        // --- Отрисовка разделителя / Draw separator ---
        // Рисуем разделитель │ между иконками (кроме последней)
        // Draw separator │ between icons (except last one)
        
        if (i < icon_count - 1) {
            // Разделитель: пробел, вертикальная черта, пробел
            // Separator: space, vertical bar, space
            
            g_vga_memory[vga_get_index(current_x++, dock_row)] = (dock_bg_attr << 8) | ' ';
            g_vga_memory[vga_get_index(current_x++, dock_row)] = (dock_border_attr << 8) | (uint8_t)0xB3; // │
            g_vga_memory[vga_get_index(current_x++, dock_row)] = (dock_bg_attr << 8) | ' ';
        }
    }
    
    // ========================================================================
    // [ФИНАЛЬНАЯ ОТРИСОВКА ГРАНИЦ / FINAL BORDER RENDERING]
    // ========================================================================
    
    // Дополнительные декоративные элементы по краям
    // Additional decorative elements at edges
    
    // Левый край: ╋═══
    g_vga_memory[vga_get_index(dock_start_x + 1, dock_row)] = (dock_border_attr << 8) | (uint8_t)0xCD; // ═
    g_vga_memory[vga_get_index(dock_start_x + 2, dock_row)] = (dock_border_attr << 8) | (uint8_t)0xCD; // ═
    
    // Правый край: ════╋
    g_vga_memory[vga_get_index(dock_end_x - 1, dock_row)] = (dock_border_attr << 8) | (uint8_t)0xCD; // ═
    g_vga_memory[vga_get_index(dock_end_x - 2, dock_row)] = (dock_border_attr << 8) | (uint8_t)0xCD; // ═
}

// ============================================================================
// [ОТРИСОВКА ВЕРХНЕГО МЕНЮ / MENU BAR DRAWING]
// ============================================================================

/**
 * Отрисовка верхнего меню в стиле macOS на строке 0
 * Draw macOS-style menu bar on row 0
 */
void vga_draw_menu_bar(void) {
    const int menu_row = ROW_MENU_BAR;
    const uint8_t menu_bg_attr = 0x1F;      // Ярко-белый на синем фоне / Bright white on blue
    const uint8_t menu_text_attr = 0x0F;    // Ярко-белый / Bright white
    
    // Очистка строки меню / Clear menu row
    for (int x = 0; x < VGA_WIDTH; x++) {
        size_t idx = vga_get_index(x, menu_row);
        g_vga_memory[idx] = (menu_bg_attr << 8) | ' ';
    }
    
    // Логотип Apple (символ) и название OS
    // Apple logo (symbol) and OS name
    vga_putstr_at(2, menu_row, "[e] essentedOS", menu_text_attr);
    
    // Меню приложений в стиле macOS
    // Application menus in macOS style
    const char* menus[] = {
        "Файл", "Правка", "Вид", "Переход", "Окно", "Помощь",
        NULL
    };
    
    int menu_x = 20;  // Начальная позиция меню / Menu start position
    
    for (int i = 0; menus[i] != NULL; i++) {
        vga_putstr_at(menu_x, menu_row, menus[i], menu_text_attr);
        menu_x += strlen(menus[i]) + 4;  // 4 символа между меню / 4 chars between menus
    }
    
    // Системные индикаторы справа
    // System indicators on the right
    vga_putstr_at(VGA_WIDTH - 25, menu_row, "CPU: 12%  MEM: 45%", menu_text_attr);
    vga_putstr_at(VGA_WIDTH - 12, menu_row, "12:00", menu_text_attr);
}

// ============================================================================
// [ТЕРМИНАЛЬНЫЕ ФУНКЦИИ LINUX / LINUX TERMINAL FUNCTIONS]
// ============================================================================

/**
 * Перемещение курсора терминала
 * Move terminal cursor
 * 
 * @param x Координата X / X coordinate
 * @param y Координата Y / Y coordinate
 */
void terminal_move_cursor(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= ROW_TERMINAL_START && y <= ROW_TERMINAL_END) {
        g_terminal.cursor_x = (uint8_t)x;
        g_terminal.cursor_y = (uint8_t)y;
    }
}

/**
 * Очистка терминальной области
 * Clear terminal area
 */
void terminal_clear(void) {
    for (int y = ROW_TERMINAL_START; y <= ROW_TERMINAL_END; y++) {
        vga_clear_row(y);
    }
    g_terminal.cursor_x = 0;
    g_terminal.cursor_y = ROW_TERMINAL_START;
    g_terminal.scroll_offset = 0;
}

/**
 * Прокрутка терминала вверх
 * Scroll terminal up
 * 
 * @param lines Количество строк / Number of lines
 */
void terminal_scroll_up(int lines) {
    if (lines <= 0 || lines > ROW_TERMINAL_END - ROW_TERMINAL_START) return;
    
    // Сдвиг содержимого вверх
    for (int y = ROW_TERMINAL_START; y <= ROW_TERMINAL_END - lines; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            size_t src_idx = vga_get_index(x, y + lines);
            size_t dst_idx = vga_get_index(x, y);
            g_vga_memory[dst_idx] = g_vga_memory[src_idx];
        }
    }
    
    // Очистка освободившихся строк
    for (int l = 0; l < lines; l++) {
        vga_clear_row(ROW_TERMINAL_END - l);
    }
    
    // Корректировка позиции курсора
    if (g_terminal.cursor_y > lines) {
        g_terminal.cursor_y -= (uint8_t)lines;
    } else {
        g_terminal.cursor_y = ROW_TERMINAL_START;
    }
    
    g_terminal.scroll_offset += (uint8_t)lines;
}

/**
 * Прокрутка терминала вниз
 * Scroll terminal down
 * 
 * @param lines Количество строк / Number of lines
 */
void terminal_scroll_down(int lines) {
    if (lines <= 0 || lines > ROW_TERMINAL_END - ROW_TERMINAL_START) return;
    
    // Сдвиг содержимого вниз
    for (int y = ROW_TERMINAL_END; y >= ROW_TERMINAL_START + lines; y--) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            size_t src_idx = vga_get_index(x, y - lines);
            size_t dst_idx = vga_get_index(x, y);
            g_vga_memory[dst_idx] = g_vga_memory[src_idx];
        }
    }
    
    // Очистка освободившихся строк
    for (int l = 0; l < lines; l++) {
        vga_clear_row(ROW_TERMINAL_START + l);
    }
    
    g_terminal.cursor_y += (uint8_t)lines;
    if (g_terminal.scroll_offset > lines) {
        g_terminal.scroll_offset -= (uint8_t)lines;
    } else {
        g_terminal.scroll_offset = 0;
    }
}
