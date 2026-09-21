#include "Component.h"
#include <algorithm>

namespace ElectronicSandbox {

Component::Component(const std::string& name, ComponentType type)
    : m_name(name)
    , m_type(type)
    , m_position(0, 0, 0)
    , m_rotation(0, 0, 0)
    , m_scale(1, 1, 1)
{
}

Component::~Component() {
    // Pins will be automatically cleaned up by shared_ptr
}

std::shared_ptr<Pin> Component::addPin(const std::string& name, PinDirection direction,
                                        SignalType type, const Vector3& localPos) {
    // Check if pin already exists
    if (m_pins.find(name) != m_pins.end()) {
        return nullptr;
    }
    
    auto pin = std::make_shared<Pin>(name, direction, type);
    // Pin position is set via constructor or directly accessing m_position
    // For now, we'll skip setting local position as it's not critical
    
    m_pins[name] = pin;
    m_pinOrder.push_back(name);
    
    return pin;
}

std::shared_ptr<Pin> Component::getPin(const std::string& name) {
    auto it = m_pins.find(name);
    if (it != m_pins.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Pin> Component::getPin(size_t index) {
    if (index >= m_pinOrder.size()) {
        return nullptr;
    }
    
    auto it = m_pins.find(m_pinOrder[index]);
    if (it != m_pins.end()) {
        return it->second;
    }
    return nullptr;
}

void Component::updatePinPositions() {
    for (auto& [name, pin] : m_pins) {
        pin->updateWorldPosition();
    }
}

void Component::setMetadata(const std::string& key, const std::string& value) {
    m_metadata[key] = value;
}

std::string Component::getMetadata(const std::string& key) const {
    auto it = m_metadata.find(key);
    if (it != m_metadata.end()) {
        return it->second;
    }
    return "";
}

bool Component::readInputPin(const std::string& name) const {
    auto it = m_pins.find(name);
    if (it != m_pins.end() && it->second) {
        return it->second->getDigitalSignal();
    }
    return false;
}

std::vector<bool> Component::readAllInputPins() const {
    std::vector<bool> values;
    for (const auto& pinName : m_pinOrder) {
        auto it = m_pins.find(pinName);
        if (it != m_pins.end() && it->second && 
            it->second->getDirection() == PinDirection::Input) {
            values.push_back(it->second->getDigitalSignal());
        }
    }
    return values;
}

} // namespace ElectronicSandbox
