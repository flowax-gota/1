#include "Chip.h"
#include "../core/Component.h"
#include "LogicGate.h"
#include <sstream>
#include <algorithm>

namespace ElectronicSandbox {

Chip::Chip(const std::string& name)
    : Component(name, ComponentType::Chip)
    , m_author("Unknown")
    , m_description("")
{
    setColor(0.3f, 0.3f, 0.6f);  // Blue-gray color for chips
}

Chip::~Chip() {
    m_internalComponents.clear();
    m_internalWires.clear();
    m_pinMappings.clear();
}

void Chip::addComponent(std::shared_ptr<Component> component) {
    if (!component) {
        return;
    }
    
    // Check if already exists
    for (const auto& existing : m_internalComponents) {
        if (existing == component) {
            return;
        }
    }
    
    m_internalComponents.push_back(component);
}

void Chip::removeComponent(const std::string& componentName) {
    m_internalComponents.erase(
        std::remove_if(m_internalComponents.begin(), m_internalComponents.end(),
            [&componentName](const std::shared_ptr<Component>& comp) {
                return comp && comp->getName() == componentName;
            }),
        m_internalComponents.end()
    );
}

std::shared_ptr<Component> Chip::getComponent(const std::string& name) {
    for (const auto& comp : m_internalComponents) {
        if (comp && comp->getName() == name) {
            return comp;
        }
    }
    return nullptr;
}

void Chip::addInternalWire(std::shared_ptr<Wire> wire) {
    if (wire) {
        m_internalWires.push_back(wire);
    }
}

void Chip::removeInternalWire(const std::string& wireId) {
    // Implementation would need wire ID tracking
    m_internalWires.clear();  // Simplified
}

bool Chip::mapExternalPinToInternal(const std::string& externalPin,
                                     std::shared_ptr<Component> internalComponent,
                                     const std::string& internalPin) {
    if (!internalComponent) {
        return false;
    }
    
    auto internalPinPtr = internalComponent->getPin(internalPin);
    if (!internalPinPtr) {
        return false;
    }
    
    PinMapping mapping;
    mapping.component = internalComponent;
    mapping.internalPinName = internalPin;
    mapping.isInput = (internalPinPtr->getDirection() == PinDirection::Input);
    
    m_pinMappings[externalPin] = mapping;
    
    return true;
}

void Chip::unmapExternalPin(const std::string& externalPin) {
    m_pinMappings.erase(externalPin);
}

std::shared_ptr<Chip> Chip::createFromComponents(
    const std::string& chipName,
    const std::vector<std::shared_ptr<Component>>& components,
    const std::vector<std::shared_ptr<Wire>>& wires,
    const std::vector<std::string>& inputPins,
    const std::vector<std::string>& outputPins)
{
    auto chip = std::make_shared<Chip>(chipName);
    
    // Add all components to the chip
    for (const auto& comp : components) {
        chip->addComponent(comp);
    }
    
    // Add all internal wires
    for (const auto& wire : wires) {
        chip->addInternalWire(wire);
    }
    
    // Initialize external interface
    chip->initializeInterface(inputPins, outputPins);
    
    return chip;
}

void Chip::simulate() {
    if (!m_active) {
        return;
    }
    
    // Propagate input signals from external pins to internal components
    propagateInputSignals();
    
    // Simulate all internal components
    for (auto& component : m_internalComponents) {
        if (component) {
            component->simulate();
        }
    }
    
    // Propagate output signals from internal components to external pins
    propagateOutputSignals();
}

void Chip::update(float deltaTime) {
    // Update all internal components
    for (auto& component : m_internalComponents) {
        if (component) {
            component->update(deltaTime);
        }
    }
}

std::shared_ptr<Component> Chip::getComponentByPath(const std::string& path) {
    // Simple path: just component name
    // Complex path: "U1/U2/Component" (nested chips)
    
    size_t slashPos = path.find('/');
    if (slashPos == std::string::npos) {
        return getComponent(path);
    }
    
    std::string firstLevel = path.substr(0, slashPos);
    std::string remaining = path.substr(slashPos + 1);
    
    auto component = getComponent(firstLevel);
    if (!component || component->getType() != ComponentType::Chip) {
        return nullptr;
    }
    
    // Cast to Chip and recurse
    auto chip = std::dynamic_pointer_cast<Chip>(component);
    if (chip) {
        return chip->getComponentByPath(remaining);
    }
    
    return nullptr;
}

std::string Chip::serialize() const {
    std::stringstream ss;
    
    // Simple serialization format (would be JSON/XML in production)
    ss << "CHIP:" << m_name << "\n";
    ss << "AUTHOR:" << m_author << "\n";
    ss << "DESC:" << m_description << "\n";
    ss << "COMPONENTS:" << m_internalComponents.size() << "\n";
    ss << "PINS:" << m_pins.size() << "\n";
    
    // In production: serialize each component and wire
    
    return ss.str();
}

std::shared_ptr<Chip> Chip::deserialize(const std::string& data) {
    // Placeholder for deserialization
    // In production: parse the serialized format and reconstruct the chip
    return nullptr;
}

void Chip::initializeInterface(const std::vector<std::string>& inputs,
                               const std::vector<std::string>& outputs) {
    // Create external input pins
    for (size_t i = 0; i < inputs.size(); ++i) {
        Vector3 pos(-0.5f, 0.2f - (i * 0.1f), 0.0f);
        addPin(inputs[i], PinDirection::Input, SignalType::Digital, pos);
    }
    
    // Create external output pins
    for (size_t i = 0; i < outputs.size(); ++i) {
        Vector3 pos(0.5f, 0.2f - (i * 0.1f), 0.0f);
        addPin(outputs[i], PinDirection::Output, SignalType::Digital, pos);
    }
}

void Chip::propagateInputSignals() {
    // Copy signals from external input pins to mapped internal pins
    for (const auto& [extPinName, mapping] : m_pinMappings) {
        if (mapping.isInput) {
            auto extPin = getPin(extPinName);
            if (!extPin) continue;
            
            auto internalComp = mapping.component.lock();
            if (!internalComp) continue;
            
            auto internalPin = internalComp->getPin(mapping.internalPinName);
            if (internalPin) {
                internalPin->setDigitalSignal(extPin->getDigitalSignal());
            }
        }
    }
}

void Chip::propagateOutputSignals() {
    // Copy signals from mapped internal pins to external output pins
    for (const auto& [extPinName, mapping] : m_pinMappings) {
        if (!mapping.isInput) {
            auto extPin = getPin(extPinName);
            if (!extPin) continue;
            
            auto internalComp = mapping.component.lock();
            if (!internalComp) continue;
            
            auto internalPin = internalComp->getPin(mapping.internalPinName);
            if (internalPin) {
                extPin->setDigitalSignal(internalPin->getDigitalSignal());
            }
        }
    }
}

std::string Chip::generateComponentId(ComponentType type) {
    std::string prefix;
    switch (type) {
        case ComponentType::LogicGate: prefix = "U"; break;
        case ComponentType::Input: prefix = "I"; break;
        case ComponentType::Output: prefix = "O"; break;
        case ComponentType::Chip: prefix = "C"; break;
        default: prefix = "X"; break;
    }
    
    return prefix + std::to_string(++m_componentCounter);
}

} // namespace ElectronicSandbox
