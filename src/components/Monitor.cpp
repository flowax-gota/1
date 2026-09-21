#include "Monitor.h"
#include <sstream>
#include <cmath>
#include <algorithm>

namespace ElectronicSandbox {

Monitor::Monitor(const std::string& name, int width, int height)
    : Component(name, ComponentType::Monitor)
    , m_width(width)
    , m_height(height)
{
    // Initialize graphics buffer
    m_graphicsBuffer.resize(width * height, false);
    
    // Add input pins for data
    addPin("DATA", PinDirection::Input, SignalType::Digital, Vector3(-0.4f, 0.0f, 0.0f));
    addPin("CLK", PinDirection::Input, SignalType::Digital, Vector3(-0.4f, -0.1f, 0.0f));
    addPin("RS", PinDirection::Input, SignalType::Digital, Vector3(-0.4f, -0.2f, 0.0f));  // Register Select
    addPin("RW", PinDirection::Input, SignalType::Digital, Vector3(-0.4f, -0.3f, 0.0f));  // Read/Write
    addPin("E", PinDirection::Input, SignalType::Digital, Vector3(-0.4f, -0.4f, 0.0f));   // Enable
    
    setColor(0.1f, 0.1f, 0.15f);  // Dark color for monitor body
}

Monitor::~Monitor() {
}

void Monitor::setSize(int width, int height) {
    m_width = std::max(8, width);
    m_height = std::max(4, height);
    m_graphicsBuffer.resize(m_width * m_height, false);
}

void Monitor::setText(const std::string& text) {
    m_displayText = text;
    updateDisplay();
}

void Monitor::appendText(const std::string& text) {
    m_displayText += text;
    
    // Handle auto-scroll
    if (m_autoScroll) {
        size_t maxLines = m_height;
        size_t lineLength = m_width;
        
        // Count lines
        size_t lineCount = 1;
        for (char c : m_displayText) {
            if (c == '\n') lineCount++;
        }
        
        // Scroll if needed
        if (lineCount > maxLines) {
            size_t pos = 0;
            size_t linesToRemove = lineCount - maxLines;
            
            for (size_t i = 0; i < linesToRemove && pos < m_displayText.size(); ++i) {
                pos = m_displayText.find('\n', pos);
                if (pos != std::string::npos) pos++;
            }
            
            if (pos > 0) {
                m_displayText = m_displayText.substr(pos);
            }
        }
    }
    
    updateDisplay();
}

void Monitor::clearText() {
    m_displayText.clear();
    m_cursorX = 0;
    m_cursorY = 0;
    updateDisplay();
}

void Monitor::setValue(int value, int base) {
    m_numericValue = value;
    m_numericBase = base;
    updateDisplay();
}

void Monitor::setValue(float value, int decimals) {
    m_floatValue = value;
    m_decimalPlaces = decimals;
    updateDisplay();
}

void Monitor::setPixel(int x, int y, bool on) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_graphicsBuffer[y * m_width + x] = on;
    }
}

bool Monitor::getPixel(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return m_graphicsBuffer[y * m_width + x];
    }
    return false;
}

void Monitor::clearGraphics() {
    std::fill(m_graphicsBuffer.begin(), m_graphicsBuffer.end(), false);
}

void Monitor::drawLine(int x0, int y0, int x1, int y1) {
    // Bresenham's line algorithm
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        setPixel(x0, y0, true);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Monitor::drawRect(int x, int y, int w, int h, bool filled) {
    if (filled) {
        for (int iy = y; iy < y + h && iy < m_height; ++iy) {
            for (int ix = x; ix < x + w && ix < m_width; ++ix) {
                setPixel(ix, iy, true);
            }
        }
    } else {
        drawLine(x, y, x + w, y);           // Top
        drawLine(x, y + h, x + w, y + h);   // Bottom
        drawLine(x, y, x, y + h);           // Left
        drawLine(x + w, y, x + w, y + h);   // Right
    }
}

void Monitor::setBacklightColor(float r, float g, float b) {
    m_backlightR = r;
    m_backlightG = g;
    m_backlightB = b;
}

void Monitor::setBacklightBrightness(float brightness) {
    m_brightness = std::max(0.0f, std::min(1.0f, brightness));
}

void Monitor::simulate() {
    if (!m_active) {
        return;
    }
    
    // Read input pins and update display accordingly
    auto dataPin = getPin("DATA");
    auto clkPin = getPin("CLK");
    auto rsPin = getPin("RS");
    
    // Simple simulation: read binary value from DATA pin
    if (dataPin) {
        bool data = dataPin->getDigitalSignal();
        
        if (m_mode == DisplayMode::Binary) {
            // Display signal state
            clearGraphics();
            if (data) {
                drawRect(2, 2, m_width - 4, m_height - 4, true);
            }
        } else if (m_mode == DisplayMode::Numeric) {
            setValue(data ? 1 : 0, 10);
        }
    }
    
    updateDisplay();
}

void Monitor::updateDisplay() {
    // This would update the visual representation
    // In a real implementation, this would trigger rendering updates
}

std::string Monitor::formatNumericValue() const {
    std::stringstream ss;
    
    switch (m_numericBase) {
        case 2:  // Binary
            ss << "0b";
            for (int i = 31; i >= 0; --i) {
                if (m_numericValue & (1 << i)) {
                    ss << '1';
                } else if (i < 31) {  // Skip leading zeros after 0b
                    static bool started = false;
                    if (started || i < 8) {
                        ss << '0';
                        started = true;
                    }
                }
            }
            break;
        case 16:  // Hex
            ss << "0x" << std::hex << m_numericValue;
            break;
        default:  // Decimal
            ss << m_numericValue;
            break;
    }
    
    return ss.str();
}

} // namespace ElectronicSandbox
