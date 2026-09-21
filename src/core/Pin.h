#ifndef PIN_H
#define PIN_H

#include <string>
#include <vector>
#include <memory>
#include "Vector3.h"

namespace ElectronicSandbox {

// Signal types supported by the system
enum class SignalType {
    Digital,    // Binary 0/1
    Analog,     // Continuous voltage
    Data        // Complex data packets
};

// Pin direction
enum class PinDirection {
    Input,
    Output,
    Bidirectional
};

// Forward declaration
class Component;
class Wire;

/**
 * @brief Represents a connection point on a component
 * 
 * Pins are the interface between components and wires.
 * They handle signal input/output and connection management.
 */
class Pin {
public:
    Pin(const std::string& name, PinDirection direction, SignalType type = SignalType::Digital);
    ~Pin();

    // Getters
    const std::string& getName() const { return m_name; }
    PinDirection getDirection() const { return m_direction; }
    SignalType getSignalType() const { return m_signalType; }
    Component* getParentComponent() const { return m_parentComponent; }
    const Vector3& getPosition() const { return m_position; }
    const Vector3& getWorldPosition() const;
    
    // Signal handling
    bool getDigitalSignal() const { return m_digitalValue; }
    float getAnalogSignal() const { return m_analogValue; }
    
    // Set signal value (for output pins)
    void setDigitalSignal(bool value);
    void setAnalogSignal(float value);
    
    // Connection management
    bool connectTo(std::shared_ptr<Wire> wire);
    bool disconnectFrom(std::shared_ptr<Wire> wire);
    void disconnectAll();
    
    const std::vector<std::shared_ptr<Wire>>& getConnectedWires() const { return m_connectedWires; }
    bool hasConnections() const { return !m_connectedWires.empty(); }
    
    // Parent component binding
    void setParentComponent(Component* component);
    
    // Update world position based on parent transform
    void updateWorldPosition();

private:
    std::string m_name;
    PinDirection m_direction;
    SignalType m_signalType;
    
    // Signal values
    bool m_digitalValue = false;
    float m_analogValue = 0.0f;
    
    // Position in local and world space
    Vector3 m_position;
    Vector3 m_worldPosition;
    
    // Parent component
    Component* m_parentComponent = nullptr;
    
    // Connected wires
    std::vector<std::shared_ptr<Wire>> m_connectedWires;
};

} // namespace ElectronicSandbox

#endif // PIN_H
