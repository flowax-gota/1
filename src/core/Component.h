#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include "Vector3.h"
#include "Pin.h"

namespace ElectronicSandbox {

// Component types for categorization
enum class ComponentType {
    LogicGate,
    Input,
    Output,
    Chip,
    Monitor,
    Power,
    Custom
};

// Base class for all electronic components
class Component : public std::enable_shared_from_this<Component> {
public:
    Component(const std::string& name, ComponentType type);
    virtual ~Component();

    // Identity
    const std::string& getName() const { return m_name; }
    ComponentType getType() const { return m_type; }
    
    // 3D Transform
    void setPosition(const Vector3& pos) { m_position = pos; updatePinPositions(); }
    void setRotation(const Vector3& rot) { m_rotation = rot; updatePinPositions(); }
    void setScale(const Vector3& scale) { m_scale = scale; updatePinPositions(); }
    
    const Vector3& getPosition() const { return m_position; }
    const Vector3& getRotation() const { return m_rotation; }
    const Vector3& getScale() const { return m_scale; }
    
    // Pin management
    std::shared_ptr<Pin> addPin(const std::string& name, PinDirection direction, 
                                SignalType type = SignalType::Digital,
                                const Vector3& localPos = Vector3());
    
    std::shared_ptr<Pin> getPin(const std::string& name);
    std::shared_ptr<Pin> getPin(size_t index);
    size_t getPinCount() const { return m_pins.size(); }
    
    const std::map<std::string, std::shared_ptr<Pin>>& getAllPins() const { return m_pins; }
    
    // Simulation
    virtual void update(float deltaTime) {}
    virtual void simulate() {}
    
    // State
    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }
    
    // Metadata for UI/serialization
    void setMetadata(const std::string& key, const std::string& value);
    std::string getMetadata(const std::string& key) const;
    
    // Visual properties (for rendering)
    void setColor(float r, float g, float b) { 
        m_colorR = r; m_colorG = g; m_colorB = b; 
    }
    float getColorR() const { return m_colorR; }
    float getColorG() const { return m_colorG; }
    float getColorB() const { return m_colorB; }

protected:
    // Update all pin world positions
    void updatePinPositions();
    
    // Helper to calculate output based on inputs
    bool readInputPin(const std::string& name) const;
    std::vector<bool> readAllInputPins() const;

protected:
    std::string m_name;
    ComponentType m_type;
    
    // Transform
    Vector3 m_position;
    Vector3 m_rotation;  // Euler angles in degrees
    Vector3 m_scale = Vector3(1, 1, 1);
    
    // Pins
    std::map<std::string, std::shared_ptr<Pin>> m_pins;
    std::vector<std::string> m_pinOrder;  // Maintain insertion order
    
    // State
    bool m_active = true;
    
    // Visual
    float m_colorR = 0.5f;
    float m_colorG = 0.5f;
    float m_colorB = 0.5f;
    
    // Metadata
    std::map<std::string, std::string> m_metadata;
};

} // namespace ElectronicSandbox

#endif // COMPONENT_H
