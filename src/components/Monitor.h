#ifndef MONITOR_H
#define MONITOR_H

#include "Component.h"
#include <string>
#include <vector>

namespace ElectronicSandbox {

// Display modes for monitors
enum class DisplayMode {
    Text,       // Simple text display
    Numeric,    // Numeric value (decimal/hex/binary)
    Graph,      // Waveform/graph display
    Binary,     // LED-style binary display
    Custom      // User-defined pattern
};

/**
 * @brief Interactive display/monitor component
 * 
 * Monitors can display text, numbers, waveforms, or custom graphics.
 * They connect to circuits to visualize signals and data.
 */
class Monitor : public Component {
public:
    Monitor(const std::string& name, int width = 32, int height = 16);
    virtual ~Monitor();

    // Display configuration
    void setDisplayMode(DisplayMode mode) { m_mode = mode; }
    DisplayMode getDisplayMode() const { return m_mode; }
    
    void setSize(int width, int height);
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Text display
    void setText(const std::string& text);
    void appendText(const std::string& text);
    void clearText();
    const std::string& getText() const { return m_displayText; }
    
    // Numeric display
    void setValue(int value, int base = 10);  // base: 2=binary, 10=decimal, 16=hex
    void setValue(float value, int decimals = 2);
    int getNumericValue() const { return m_numericValue; }
    
    // Graphics display
    void setPixel(int x, int y, bool on);
    bool getPixel(int x, int y) const;
    void clearGraphics();
    void drawLine(int x0, int y0, int x1, int y1);
    void drawRect(int x, int y, int w, int h, bool filled = false);
    
    // Backlight/color
    void setBacklightColor(float r, float g, float b);
    void setBacklightBrightness(float brightness);
    
    // Simulation
    void simulate() override;
    
    // Get display buffer for rendering
    const std::vector<bool>& getGraphicsBuffer() const { return m_graphicsBuffer; }
    
    // Auto-scroll for text
    void setAutoScroll(bool enabled) { m_autoScroll = enabled; }
    bool getAutoScroll() const { return m_autoScroll; }

private:
    // Update internal display state
    void updateDisplay();
    
    // Convert numeric value to string based on mode
    std::string formatNumericValue() const;

private:
    DisplayMode m_mode = DisplayMode::Text;
    
    // Dimensions
    int m_width = 32;
    int m_height = 16;
    
    // Text buffer
    std::string m_displayText;
    int m_cursorX = 0;
    int m_cursorY = 0;
    bool m_autoScroll = true;
    
    // Numeric value
    int m_numericValue = 0;
    float m_floatValue = 0.0f;
    int m_numericBase = 10;
    int m_decimalPlaces = 2;
    
    // Graphics buffer (1 bit per pixel)
    std::vector<bool> m_graphicsBuffer;
    
    // Visual properties
    float m_backlightR = 0.0f;
    float m_backlightG = 0.5f;
    float m_backlightB = 0.0f;
    float m_brightness = 1.0f;
};

} // namespace ElectronicSandbox

#endif // MONITOR_H
