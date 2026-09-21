#include "CircuitSimulator.h"
#include <algorithm>
#include <iostream>

namespace ElectronicSandbox {

CircuitSimulator::CircuitSimulator() {
}

CircuitSimulator::~CircuitSimulator() {
    stop();
    clear();
}

void CircuitSimulator::addComponent(std::shared_ptr<Component> component) {
    if (!component) {
        return;
    }
    
    // Check if already exists
    if (m_componentMap.find(component->getName()) != m_componentMap.end()) {
        std::cerr << "Component with name '" << component->getName() 
                  << "' already exists!" << std::endl;
        return;
    }
    
    m_components.push_back(component);
    m_componentMap[component->getName()] = component;
    
    initializeComponent(component);
    m_needsReorder = true;
    
    // Trigger callbacks
    for (auto& callback : m_onComponentAdded) {
        callback(component);
    }
}

void CircuitSimulator::removeComponent(const std::string& name) {
    auto it = m_componentMap.find(name);
    if (it == m_componentMap.end()) {
        return;
    }
    
    auto component = it->second;
    
    // Remove from vectors
    m_components.erase(
        std::remove(m_components.begin(), m_components.end(), component),
        m_components.end()
    );
    
    m_componentMap.erase(it);
    m_needsReorder = true;
    
    // Trigger callbacks
    for (auto& callback : m_onComponentRemoved) {
        callback(component);
    }
}

std::shared_ptr<Component> CircuitSimulator::getComponent(const std::string& name) {
    auto it = m_componentMap.find(name);
    if (it != m_componentMap.end()) {
        return it->second;
    }
    return nullptr;
}

void CircuitSimulator::addWire(std::shared_ptr<Wire> wire) {
    if (wire) {
        m_wires.push_back(wire);
    }
}

void CircuitSimulator::removeWire(std::shared_ptr<Wire> wire) {
    m_wires.erase(
        std::remove(m_wires.begin(), m_wires.end(), wire),
        m_wires.end()
    );
}

void CircuitSimulator::start() {
    m_running = true;
    m_paused = false;
    computeSimulationOrder();
}

void CircuitSimulator::stop() {
    m_running = false;
    m_paused = false;
}

void CircuitSimulator::pause() {
    if (m_running) {
        m_paused = true;
    }
}

void CircuitSimulator::resume() {
    if (m_running) {
        m_paused = false;
    }
}

void CircuitSimulator::update(float deltaTime) {
    if (!m_running || m_paused) {
        return;
    }
    
    m_currentTime += deltaTime * m_timeScale;
    m_lastUpdateTime = m_currentTime;
    
    // Process pending events
    processEvents();
    
    // Simulate all components in order
    simulateStep();
}

void CircuitSimulator::simulateStep() {
    // Use pre-computed simulation order if available
    const auto& order = m_needsReorder ? m_components : m_simulationOrder;
    
    for (auto& component : order) {
        if (component && component->isActive()) {
            component->simulate();
        }
    }
}

void CircuitSimulator::queueEvent(const SimulationEvent& event) {
    m_eventQueue.push(event);
}

void CircuitSimulator::processEvents() {
    while (!m_eventQueue.empty()) {
        const auto& event = m_eventQueue.top();
        
        // Only process if timestamp has passed
        if (event.timestamp <= m_currentTime) {
            if (event.component && event.component->isActive()) {
                event.component->simulate();
            }
            m_eventQueue.pop();
        } else {
            break;  // Future event, wait for next update
        }
    }
}

void CircuitSimulator::propagateSignals() {
    // Propagate signals through all wires
    for (auto& wire : m_wires) {
        if (wire) {
            auto fromPin = wire->getFromPin();
            if (fromPin) {
                wire->propagateSignal(fromPin.get());
            }
        }
    }
}

std::vector<std::shared_ptr<Component>> CircuitSimulator::findComponents(
    const std::string& pattern) const 
{
    std::vector<std::shared_ptr<Component>> results;
    
    // Simple wildcard matching (* and ?)
    for (const auto& [name, component] : m_componentMap) {
        bool matches = true;
        size_t patternIdx = 0;
        size_t nameIdx = 0;
        size_t starIdx = std::string::npos;
        size_t matchIdx = 0;
        
        while (nameIdx < name.size()) {
            if (patternIdx < pattern.size() && 
                (pattern[patternIdx] == '?' || pattern[patternIdx] == name[nameIdx])) {
                patternIdx++;
                nameIdx++;
            } else if (patternIdx < pattern.size() && pattern[patternIdx] == '*') {
                starIdx = patternIdx;
                matchIdx = nameIdx;
                patternIdx++;
            } else if (starIdx != std::string::npos) {
                patternIdx = starIdx + 1;
                matchIdx++;
                nameIdx = matchIdx;
            } else {
                matches = false;
                break;
            }
        }
        
        while (patternIdx < pattern.size() && pattern[patternIdx] == '*') {
            patternIdx++;
        }
        
        if (matches && patternIdx == pattern.size()) {
            results.push_back(component);
        }
    }
    
    return results;
}

void CircuitSimulator::clear() {
    m_components.clear();
    m_wires.clear();
    m_componentMap.clear();
    m_simulationOrder.clear();
    
    while (!m_eventQueue.empty()) {
        m_eventQueue.pop();
    }
    
    m_needsReorder = true;
}

void CircuitSimulator::onComponentAdded(ComponentCallback callback) {
    m_onComponentAdded.push_back(callback);
}

void CircuitSimulator::onComponentRemoved(ComponentCallback callback) {
    m_onComponentRemoved.push_back(callback);
}

void CircuitSimulator::initializeComponent(std::shared_ptr<Component> component) {
    // Any initialization needed before simulation
    // This is called when a component is added
}

void CircuitSimulator::computeSimulationOrder() {
    // Topological sort to determine optimal simulation order
    // This ensures components are simulated after their dependencies
    
    m_simulationOrder.clear();
    std::unordered_map<std::string, bool> visited;
    std::unordered_map<std::string, bool> recStack;
    
    // DFS-based topological sort
    std::function<void(std::shared_ptr<Component>)> visit = 
        [&](std::shared_ptr<Component> comp) {
            if (!comp) return;
            
            const std::string& name = comp->getName();
            if (visited[name]) return;
            
            visited[name] = true;
            recStack[name] = true;
            
            // Visit all components connected via output pins
            for (const auto& [pinName, pin] : comp->getAllPins()) {
                if (pin->getDirection() == PinDirection::Output) {
                    for (const auto& wire : pin->getConnectedWires()) {
                        auto toPin = wire->getToPin();
                        if (toPin) {
                            auto targetComp = toPin->getParentComponent();
                            if (targetComp) {
                                visit(std::shared_ptr<Component>(targetComp, 
                                    [](Component*) {}));  // Non-owning pointer
                            }
                        }
                    }
                }
            }
            
            recStack[name] = false;
            m_simulationOrder.push_back(comp);
        };
    
    for (auto& component : m_components) {
        visit(component);
    }
    
    m_needsReorder = false;
}

bool CircuitSimulator::hasCombinationalLoop() const {
    std::unordered_map<std::string, bool> visited;
    std::unordered_map<std::string, bool> recStack;
    
    for (const auto& component : m_components) {
        if (dfsDetectLoop(component, visited, recStack)) {
            return true;
        }
    }
    
    return false;
}

bool CircuitSimulator::dfsDetectLoop(
    std::shared_ptr<Component> comp,
    std::unordered_map<std::string, bool>& visited,
    std::unordered_map<std::string, bool>& recStack) const 
{
    if (!comp) return false;
    
    const std::string& name = comp->getName();
    
    if (recStack[name]) {
        return true;  // Loop detected
    }
    
    if (visited[name]) {
        return false;
    }
    
    visited[name] = true;
    recStack[name] = true;
    
    // Check all connected components
    for (const auto& [pinName, pin] : comp->getAllPins()) {
        for (const auto& wire : pin->getConnectedWires()) {
            auto fromPin = wire->getFromPin();
            auto otherPin = (fromPin && fromPin.get() == pin.get()) ? 
                           wire->getToPin() : fromPin;
            
            if (otherPin) {
                auto otherComp = otherPin->getParentComponent();
                if (otherComp && otherComp != comp.get()) {
                    if (dfsDetectLoop(std::shared_ptr<Component>(otherComp, 
                            [](Component*) {}), visited, recStack)) {
                        return true;
                    }
                }
            }
        }
    }
    
    recStack[name] = false;
    return false;
}

} // namespace ElectronicSandbox
