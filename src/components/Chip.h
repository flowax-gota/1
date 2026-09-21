#ifndef CHIP_H
#define CHIP_H

#include "Component.h"
#include <unordered_map>
#include <unordered_set>

namespace ElectronicSandbox {

/**
 * @brief Represents a custom integrated circuit (IC) chip
 * 
 * Chips allow players to encapsulate complex circuits into reusable components.
 * This supports hierarchical design and circuit abstraction.
 */
class Chip : public Component {
public:
    Chip(const std::string& name);
    virtual ~Chip();

    // Circuit management
    void addComponent(std::shared_ptr<Component> component);
    void removeComponent(const std::string& componentName);
    std::shared_ptr<Component> getComponent(const std::string& name);
    
    const std::vector<std::shared_ptr<Component>>& getAllComponents() const { 
        return m_internalComponents; 
    }
    
    // Wire management inside chip
    void addInternalWire(std::shared_ptr<Wire> wire);
    void removeInternalWire(const std::string& wireId);
    
    // Interface definition (external pins mapped to internal components)
    bool mapExternalPinToInternal(const std::string& externalPin, 
                                   std::shared_ptr<Component> internalComponent,
                                   const std::string& internalPin);
    void unmapExternalPin(const std::string& externalPin);
    
    // Create chip from existing components (for "packaging" feature)
    static std::shared_ptr<Chip> createFromComponents(
        const std::string& chipName,
        const std::vector<std::shared_ptr<Component>>& components,
        const std::vector<std::shared_ptr<Wire>>& wires,
        const std::vector<std::string>& inputPins,   // External interface
        const std::vector<std::string>& outputPins   // External interface
    );
    
    // Simulation
    void simulate() override;
    void update(float deltaTime) override;
    
    // Get internal component by path (e.g., "U1/AND2/OUT")
    std::shared_ptr<Component> getComponentByPath(const std::string& path);
    
    // Serialization support
    std::string serialize() const;
    static std::shared_ptr<Chip> deserialize(const std::string& data);
    
    // Metadata
    void setAuthor(const std::string& author) { m_author = author; }
    void setDescription(const std::string& desc) { m_description = desc; }
    const std::string& getAuthor() const { return m_author; }
    const std::string& getDescription() const { return m_description; }

private:
    // Initialize external interface pins
    void initializeInterface(const std::vector<std::string>& inputs,
                            const std::vector<std::string>& outputs);
    
    // Forward signals between external and internal pins
    void propagateInputSignals();
    void propagateOutputSignals();
    
    // Generate unique ID for a component
    std::string generateComponentId(ComponentType type);

private:
    // Internal circuit
    std::vector<std::shared_ptr<Component>> m_internalComponents;
    std::vector<std::shared_ptr<Wire>> m_internalWires;
    
    // Mapping: external pin name -> (internal component, internal pin name)
    struct PinMapping {
        std::weak_ptr<Component> component;
        std::string internalPinName;
        bool isInput;  // true if external pin is input, false if output
    };
    
    std::map<std::string, PinMapping> m_pinMappings;
    
    // Component naming counter
    int m_componentCounter = 0;
    
    // Metadata
    std::string m_author;
    std::string m_description;
};

} // namespace ElectronicSandbox

#endif // CHIP_H
