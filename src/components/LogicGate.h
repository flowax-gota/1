#ifndef LOGIC_GATE_H
#define LOGIC_GATE_H

#include "Component.h"
#include <functional>

namespace ElectronicSandbox {

// Logic gate types
enum class LogicGateType {
    AND,
    OR,
    NOT,
    NAND,
    NOR,
    XOR,
    XNOR,
    BUFFER
};

/**
 * @brief Base class for logic gates
 * 
 * Implements basic digital logic operations with configurable inputs.
 */
class LogicGate : public Component {
public:
    LogicGate(LogicGateType gateType, int inputCount = 2);
    virtual ~LogicGate();

    // Get gate type
    LogicGateType getGateType() const { return m_gateType; }
    
    // Set number of inputs (for gates that support it)
    void setInputCount(int count);
    int getInputCount() const { return m_inputCount; }
    
    // Simulation
    void simulate() override;
    
    // Get output state
    bool getOutput() const;

protected:
    // Initialize pins based on gate type
    void initializePins();
    
    // Compute output based on inputs
    bool computeOutput();
    
    // Logic functions
    bool andLogic(const std::vector<bool>& inputs);
    bool orLogic(const std::vector<bool>& inputs);
    bool notLogic(const std::vector<bool>& inputs);
    bool nandLogic(const std::vector<bool>& inputs);
    bool norLogic(const std::vector<bool>& inputs);
    bool xorLogic(const std::vector<bool>& inputs);
    bool xnorLogic(const std::vector<bool>& inputs);
    bool bufferLogic(const std::vector<bool>& inputs);

protected:
    LogicGateType m_gateType;
    int m_inputCount;
    std::shared_ptr<Pin> m_outputPin;
    
    // Propagation delay in milliseconds
    int m_propagationDelay = 0;
};

/**
 * @brief Specialized AND gate
 */
class ANDGate : public LogicGate {
public:
    ANDGate(int inputs = 2) : LogicGate(LogicGateType::AND, inputs) {}
};

/**
 * @brief Specialized OR gate
 */
class ORGate : public LogicGate {
public:
    ORGate(int inputs = 2) : LogicGate(LogicGateType::OR, inputs) {}
};

/**
 * @brief Specialized NOT gate (inverter)
 */
class NOTGate : public LogicGate {
public:
    NOTGate() : LogicGate(LogicGateType::NOT, 1) {}
};

/**
 * @brief Specialized XOR gate
 */
class XORGate : public LogicGate {
public:
    XORGate(int inputs = 2) : LogicGate(LogicGateType::XOR, inputs) {}
};

} // namespace ElectronicSandbox

#endif // LOGIC_GATE_H
