#include "Pin.h"
#include "Component.h"
#include "Wire.h"

namespace ElectronicSandbox {

Pin::Pin(const std::string& name, PinDirection direction, SignalType type)
    : m_name(name)
    , m_direction(direction)
    , m_signalType(type)
    , m_digitalValue(false)
    , m_analogValue(0.0f)
    , m_parentComponent(nullptr)
{
}

Pin::~Pin() {
    disconnectAll();
}

const Vector3& Pin::getWorldPosition() const {
    return m_worldPosition;
}

void Pin::setDigitalSignal(bool value) {
    if (m_direction == PinDirection::Input) {
        // Input pins can receive signals from connected wires
        m_digitalValue = value;
    } else {
        // Output pins drive the signal
        m_digitalValue = value;
        
        // Propagate to connected wires
        for (auto& wire : m_connectedWires) {
            wire->propagateSignal(this);
        }
    }
}

void Pin::setAnalogSignal(float value) {
    if (m_direction == PinDirection::Input) {
        m_analogValue = value;
    } else {
        m_analogValue = value;
        
        // Propagate to connected wires
        for (auto& wire : m_connectedWires) {
            wire->propagateAnalogSignal(this, value);
        }
    }
}

bool Pin::connectTo(std::shared_ptr<Wire> wire) {
    // Check if already connected
    for (const auto& existingWire : m_connectedWires) {
        if (existingWire == wire) {
            return false;
        }
    }
    
    m_connectedWires.push_back(wire);
    return true;
}

bool Pin::disconnectFrom(std::shared_ptr<Wire> wire) {
    for (auto it = m_connectedWires.begin(); it != m_connectedWires.end(); ++it) {
        if (*it == wire) {
            m_connectedWires.erase(it);
            return true;
        }
    }
    return false;
}

void Pin::disconnectAll() {
    m_connectedWires.clear();
}

void Pin::setParentComponent(Component* component) {
    m_parentComponent = component;
    updateWorldPosition();
}

void Pin::updateWorldPosition() {
    if (m_parentComponent) {
        m_worldPosition = m_parentComponent->getPosition() + m_position;
    } else {
        m_worldPosition = m_position;
    }
}

} // namespace ElectronicSandbox
