#ifndef CIRCUIT_SIMULATOR_H
#define CIRCUIT_SIMULATOR_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <functional>
#include "Component.h"
#include "Wire.h"
#include "LogicGate.h"
#include "Chip.h"
#include "Monitor.h"

namespace ElectronicSandbox {

// Simulation event for efficient propagation
struct SimulationEvent {
    std::shared_ptr<Component> component;
    double timestamp;
    int priority;  // Lower = higher priority
    
    bool operator>(const SimulationEvent& other) const {
        return timestamp > other.timestamp;
    }
};

/**
 * @brief Main circuit simulation engine
 * 
 * Handles real-time simulation of electronic circuits with:
 * - Event-driven signal propagation
 * - Support for digital and analog signals
 * - Hierarchical chip simulation
 * - Multi-threading support (future)
 */
class CircuitSimulator {
public:
    CircuitSimulator();
    ~CircuitSimulator();

    // Component management
    void addComponent(std::shared_ptr<Component> component);
    void removeComponent(const std::string& name);
    std::shared_ptr<Component> getComponent(const std::string& name);
    
    const std::vector<std::shared_ptr<Component>>& getAllComponents() const {
        return m_components;
    }
    
    // Wire management
    void addWire(std::shared_ptr<Wire> wire);
    void removeWire(std::shared_ptr<Wire> wire);
    
    const std::vector<std::shared_ptr<Wire>>& getAllWires() const {
        return m_wires;
    }
    
    // Simulation control
    void start();
    void stop();
    void pause();
    void resume();
    
    bool isRunning() const { return m_running; }
    bool isPaused() const { return m_paused; }
    
    // Simulation step
    void update(float deltaTime);
    void simulateStep();
    
    // Event-driven simulation
    void queueEvent(const SimulationEvent& event);
    void processEvents();
    
    // Signal propagation
    void propagateSignals();
    
    // Get all components of a specific type
    template<typename T>
    std::vector<std::shared_ptr<T>> getComponentsByType() const {
        std::vector<std::shared_ptr<T>> result;
        for (const auto& comp : m_components) {
            auto typed = std::dynamic_pointer_cast<T>(comp);
            if (typed) {
                result.push_back(typed);
            }
        }
        return result;
    }
    
    // Find component by name (supports wildcards)
    std::vector<std::shared_ptr<Component>> findComponents(const std::string& pattern) const;
    
    // Clear all components and wires
    void clear();
    
    // Performance metrics
    int getComponentCount() const { return m_components.size(); }
    int getWireCount() const { return m_wires.size(); }
    double getLastUpdateTime() const { return m_lastUpdateTime; }
    
    // Callbacks
    using ComponentCallback = std::function<void(std::shared_ptr<Component>)>;
    void onComponentAdded(ComponentCallback callback);
    void onComponentRemoved(ComponentCallback callback);

private:
    // Initialize component for simulation
    void initializeComponent(std::shared_ptr<Component> component);
    
    // Topological sort for simulation order
    void computeSimulationOrder();
    
    // Detect combinational loops
    bool hasCombinationalLoop() const;
    
    // Depth-first search for loop detection
    bool dfsDetectLoop(std::shared_ptr<Component> comp, 
                       std::unordered_map<std::string, bool>& visited,
                       std::unordered_map<std::string, bool>& recStack) const;

private:
    // All components in the circuit
    std::vector<std::shared_ptr<Component>> m_components;
    
    // All wires in the circuit
    std::vector<std::shared_ptr<Wire>> m_wires;
    
    // Component lookup by name
    std::unordered_map<std::string, std::shared_ptr<Component>> m_componentMap;
    
    // Simulation state
    bool m_running = false;
    bool m_paused = false;
    
    // Event queue for time-based simulation
    std::priority_queue<SimulationEvent, std::vector<SimulationEvent>, 
                        std::greater<SimulationEvent>> m_eventQueue;
    
    // Simulation order (pre-computed for efficiency)
    std::vector<std::shared_ptr<Component>> m_simulationOrder;
    
    // Timing
    double m_currentTime = 0.0;
    double m_lastUpdateTime = 0.0;
    float m_timeScale = 1.0f;
    
    // Callbacks
    std::vector<ComponentCallback> m_onComponentAdded;
    std::vector<ComponentCallback> m_onComponentRemoved;
    
    // Dirty flag system
    bool m_needsReorder = true;
};

} // namespace ElectronicSandbox

#endif // CIRCUIT_SIMULATOR_H
