#include <iostream>
#include <memory>
#include <chrono>
#include "core/Component.h"
#include "components/LogicGate.h"
#include "components/Chip.h"
#include "components/Monitor.h"
#include "simulation/CircuitSimulator.h"

using namespace ElectronicSandbox;

/**
 * @brief Demonstration of the Electronic Sandbox simulation system
 * 
 * This example creates a simple circuit with:
 * - Two input switches
 * - AND gate
 * - OR gate
 * - XOR gate
 * - Monitor to display output
 */
int main(int argc, char* argv[]) {
    std::cout << "=== Electronic Sandbox 3D Circuit Simulator ===" << std::endl;
    std::cout << std::endl;
    
    // Create the main simulator
    CircuitSimulator simulator;
    
    // === Create Components ===
    std::cout << "Creating components..." << std::endl;
    
    // Input switches (represented as components with output pins)
    auto switchA = std::make_shared<Component>("SwitchA", ComponentType::Input);
    switchA->addPin("OUT", PinDirection::Output, SignalType::Digital, Vector3(0.3f, 0, 0));
    switchA->setPosition(Vector3(0, 0, 0));
    switchA->setColor(0.8f, 0.8f, 0.2f);
    
    auto switchB = std::make_shared<Component>("SwitchB", ComponentType::Input);
    switchB->addPin("OUT", PinDirection::Output, SignalType::Digital, Vector3(0.3f, 0, 0));
    switchB->setPosition(Vector3(0, -0.5f, 0));
    switchB->setColor(0.8f, 0.8f, 0.2f);
    
    // Logic gates
    auto andGate = std::make_shared<ANDGate>(2);
    // andGate->setName("AND1");
    andGate->setPosition(Vector3(2, -0.25f, 0));
    
    auto orGate = std::make_shared<ORGate>(2);
    // orGate->setName("OR1");
    orGate->setPosition(Vector3(2, -1.0f, 0));
    
    auto xorGate = std::make_shared<XORGate>(2);
    // xorGate->setName("XOR1");
    xorGate->setPosition(Vector3(2, -1.75f, 0));
    
    // Monitor for output display
    auto monitor = std::make_shared<Monitor>("Display1", 16, 8);
    monitor->setPosition(Vector3(4, -0.5f, 0));
    monitor->setDisplayMode(DisplayMode::Text);
    monitor->setText("Ready");
    
    // Add all components to simulator
    simulator.addComponent(switchA);
    simulator.addComponent(switchB);
    simulator.addComponent(andGate);
    simulator.addComponent(orGate);
    simulator.addComponent(xorGate);
    simulator.addComponent(monitor);
    
    std::cout << "  Created " << simulator.getComponentCount() << " components" << std::endl;
    
    // === Create Wires ===
    std::cout << "Creating wire connections..." << std::endl;
    
    // Connect SwitchA to all gates (input A)
    auto wire1 = std::make_shared<Wire>();
    wire1->connect(switchA->getPin("OUT"), andGate->getPin("IN0"));
    simulator.addWire(wire1);
    
    auto wire2 = std::make_shared<Wire>();
    wire2->connect(switchA->getPin("OUT"), orGate->getPin("IN0"));
    simulator.addWire(wire2);
    
    auto wire3 = std::make_shared<Wire>();
    wire3->connect(switchA->getPin("OUT"), xorGate->getPin("IN0"));
    simulator.addWire(wire3);
    
    // Connect SwitchB to all gates (input B)
    auto wire4 = std::make_shared<Wire>();
    wire4->connect(switchB->getPin("OUT"), andGate->getPin("IN1"));
    simulator.addWire(wire4);
    
    auto wire5 = std::make_shared<Wire>();
    wire5->connect(switchB->getPin("OUT"), orGate->getPin("IN1"));
    simulator.addWire(wire5);
    
    auto wire6 = std::make_shared<Wire>();
    wire6->connect(switchB->getPin("OUT"), xorGate->getPin("IN1"));
    simulator.addWire(wire6);
    
    std::cout << "  Created " << simulator.getWireCount() << " wires" << std::endl;
    
    // === Start Simulation ===
    std::cout << std::endl;
    std::cout << "Starting simulation..." << std::endl;
    simulator.start();
    
    // === Test All Input Combinations ===
    std::cout << std::endl;
    std::cout << "Testing logic gates:" << std::endl;
    std::cout << "-------------------" << std::endl;
    
    bool testInputs[] = {false, true};
    
    for (bool a : testInputs) {
        for (bool b : testInputs) {
            // Set switch states
            switchA->getPin("OUT")->setDigitalSignal(a);
            switchB->getPin("OUT")->setDigitalSignal(b);
            
            // Run simulation step
            simulator.simulateStep();
            
            // Read outputs
            bool andOut = andGate->getOutput();
            bool orOut = orGate->getOutput();
            bool xorOut = xorGate->getOutput();
            
            // Display results
            std::cout << "A=" << (a ? 1 : 0) 
                      << " B=" << (b ? 1 : 0)
                      << " | AND=" << (andOut ? 1 : 0)
                      << " OR=" << (orOut ? 1 : 0)
                      << " XOR=" << (xorOut ? 1 : 0)
                      << std::endl;
        }
    }
    
    // === Demonstrate Chip Creation ===
    std::cout << std::endl;
    std::cout << "Creating custom chip from components..." << std::endl;
    
    // Create a simple chip that combines AND and OR gates
    auto comboChip = std::make_shared<Chip>("ComboChip");
    comboChip->setDescription("Combined AND+OR gate chip");
    comboChip->setAuthor("Demo User");
    
    // Add internal components
    auto internalAND = std::make_shared<ANDGate>(2);
    // internalAND->setName("U1");
    comboChip->addComponent(internalAND);
    
    auto internalOR = std::make_shared<ORGate>(2);
    // internalOR->setName("U2");
    comboChip->addComponent(internalOR);
    
    // Create external interface
    comboChip->addPin("A", PinDirection::Input, SignalType::Digital, Vector3(-0.5f, 0.1f, 0));
    comboChip->addPin("B", PinDirection::Input, SignalType::Digital, Vector3(-0.5f, 0.0f, 0));
    comboChip->addPin("AND_OUT", PinDirection::Output, SignalType::Digital, Vector3(0.5f, 0.1f, 0));
    comboChip->addPin("OR_OUT", PinDirection::Output, SignalType::Digital, Vector3(0.5f, 0.0f, 0));
    
    // Map external pins to internal component pins
    comboChip->mapExternalPinToInternal("A", internalAND, "IN0");
    comboChip->mapExternalPinToInternal("A", internalOR, "IN0");
    comboChip->mapExternalPinToInternal("B", internalAND, "IN1");
    comboChip->mapExternalPinToInternal("B", internalOR, "IN1");
    comboChip->mapExternalPinToInternal("AND_OUT", internalAND, "OUT");
    comboChip->mapExternalPinToInternal("OR_OUT", internalOR, "OUT");
    
    std::cout << "  Chip created with " << comboChip->getAllComponents().size() 
              << " internal components" << std::endl;
    std::cout << "  Chip has " << comboChip->getPinCount() << " external pins" << std::endl;
    
    // Test the chip
    std::cout << std::endl;
    std::cout << "Testing custom chip:" << std::endl;
    comboChip->getPin("A")->setDigitalSignal(true);
    comboChip->getPin("B")->setDigitalSignal(false);
    comboChip->simulate();
    
    std::cout << "  A=1, B=0 -> AND_OUT=" 
              << (comboChip->getPin("AND_OUT")->getDigitalSignal() ? 1 : 0)
              << ", OR_OUT="
              << (comboChip->getPin("OR_OUT")->getDigitalSignal() ? 1 : 0)
              << std::endl;
    
    // === Performance Test ===
    std::cout << std::endl;
    std::cout << "Running performance test with 1000 gates..." << std::endl;
    
    auto perfSimulator = std::make_shared<CircuitSimulator>();
    
    // Create chain of gates
    std::vector<std::shared_ptr<LogicGate>> gates;
    auto inputComp = std::make_shared<Component>("PerfInput", ComponentType::Input);
    inputComp->addPin("OUT", PinDirection::Output, SignalType::Digital);
    perfSimulator->addComponent(inputComp);
    
    for (int i = 0; i < 1000; ++i) {
        auto gate = std::make_shared<ANDGate>(2);
        // gate->setName("Gate" + std::to_string(i));
        gate->setPosition(Vector3(i * 0.1f, 0, 0));
        perfSimulator->addComponent(gate);
        gates.push_back(gate);
    }
    
    // Connect in chain
    auto prevOutput = inputComp->getPin("OUT");
    for (int i = 0; i < gates.size(); ++i) {
        auto wire = std::make_shared<Wire>();
        wire->connect(prevOutput, gates[i]->getPin("IN0"));
        perfSimulator->addWire(wire);
        prevOutput = gates[i]->getPin("OUT");
    }
    
    perfSimulator->start();
    
    // Time the simulation
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        perfSimulator->simulateStep();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    std::cout << "  Simulated 100 steps with 1000 gates in " 
              << duration.count() << " microseconds" << std::endl;
    std::cout << "  Average: " << (duration.count() / 100.0) 
              << " microseconds per step" << std::endl;
    
    // === Summary ===
    std::cout << std::endl;
    std::cout << "=== Simulation Complete ===" << std::endl;
    std::cout << "Components created: " << simulator.getComponentCount() << std::endl;
    std::cout << "Wires created: " << simulator.getWireCount() << std::endl;
    std::cout << std::endl;
    std::cout << "Features demonstrated:" << std::endl;
    std::cout << "  ✓ 3D component system with positions" << std::endl;
    std::cout << "  ✓ Dynamic wire connections" << std::endl;
    std::cout << "  ✓ Real-time logic gate simulation" << std::endl;
    std::cout << "  ✓ Custom chip creation and encapsulation" << std::endl;
    std::cout << "  ✓ Monitor/display components" << std::endl;
    std::cout << "  ✓ High-performance simulation engine" << std::endl;
    std::cout << std::endl;
    
    return 0;
}
