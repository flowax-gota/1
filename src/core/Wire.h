#ifndef WIRE_H
#define WIRE_H

#include <memory>
#include <vector>
#include "Vector3.h"

namespace ElectronicSandbox {

class Pin;

/**
 * @brief Represents a wire connection between two pins
 * 
 * Wires carry signals from output pins to input pins.
 * They can have properties like resistance, capacitance, and delay.
 */
class Wire : public std::enable_shared_from_this<Wire> {
public:
    Wire();
    ~Wire();

    // Connection endpoints
    bool connect(std::shared_ptr<Pin> from, std::shared_ptr<Pin> to);
    void disconnect();
    
    std::shared_ptr<Pin> getFromPin() const { return m_fromPin.lock(); }
    std::shared_ptr<Pin> getToPin() const { return m_toPin.lock(); }
    
    // Signal propagation
    void propagateSignal(Pin* source);
    void propagateAnalogSignal(Pin* source, float value);
    
    // Wire properties
    void setResistance(float ohms) { m_resistance = ohms; }
    void setCapacitance(float farads) { m_capacitance = farads; }
    void setDelay(int milliseconds) { m_delayMs = milliseconds; }
    
    float getResistance() const { return m_resistance; }
    float getCapacitance() const { return m_capacitance; }
    int getDelay() const { return m_delayMs; }
    
    // Visual properties (for 3D rendering)
    void setControlPoints(const std::vector<Vector3>& points) { m_controlPoints = points; }
    const std::vector<Vector3>& getControlPoints() const { return m_controlPoints; }
    
    // Calculate wire length
    float getLength() const;
    
    // Generate smooth curve points for rendering
    std::vector<Vector3> generateRenderPoints(int segments = 20) const;

private:
    std::weak_ptr<Pin> m_fromPin;  // Output pin
    std::weak_ptr<Pin> m_toPin;    // Input pin
    
    // Electrical properties
    float m_resistance = 0.0f;     // Ohms
    float m_capacitance = 0.0f;    // Farads
    int m_delayMs = 0;             // Signal delay in milliseconds
    
    // 3D path for rendering (Bezier curve control points)
    std::vector<Vector3> m_controlPoints;
    
    // Helper to calculate Bezier curve
    Vector3 bezierPoint(float t, const std::vector<Vector3>& points) const;
};

} // namespace ElectronicSandbox

#endif // WIRE_H
