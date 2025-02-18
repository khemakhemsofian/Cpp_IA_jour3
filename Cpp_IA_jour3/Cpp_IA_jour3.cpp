
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

using namespace std;

enum class NodeState { SUCCESS, FAILURE, RUNNING };

class BTNode {
public:
    virtual ~BTNode() = default;
    virtual NodeState execute() = 0;
};


class Blackboard {
private:
    std::unordered_map<std::string, int> data;
public:
    void SetValue(const std::string& key, int value) {
        data[key] = value;
    }
    int GetValue(const std::string& key) {
        return data[key];
    }
};

class SequenceNode : public BTNode {
private:
    std::vector<std::unique_ptr<BTNode>> children;
public:
    void AddChild(std::unique_ptr<BTNode> child) {
        children.push_back(std::move(child));
    }
    NodeState execute() override {
        for (auto& child : children) {
            if (child->execute() == NodeState::FAILURE) {
                return NodeState::FAILURE;
            }
        }
        return NodeState::SUCCESS;
    }
};

class SelectorNode : public BTNode {
private:
    std::vector<std::unique_ptr<BTNode>> children;
public:
    void AddChild(std::unique_ptr<BTNode> child) {
        children.push_back(std::move(child));
    }
    NodeState execute() override {
        for (auto& child : children) {
            if (child->execute() == NodeState::SUCCESS) {
                return NodeState::SUCCESS;
            }
        }
        return NodeState::FAILURE;
    }
};

class ConditionNode : public BTNode {
private:
    Blackboard& blackboard;
    std::string key;
    int expectedValue;
public:
    ConditionNode(Blackboard& bb, const std::string& key, int value) : blackboard(bb), key(key), expectedValue(value) {}
    NodeState execute() override {
        return (blackboard.GetValue(key) == expectedValue) ? NodeState::SUCCESS : NodeState::FAILURE;
    }
};

class ActionNode : public BTNode {
private:
    std::string actionName;
public:
    ActionNode(std::string name) : actionName(name) {}
    NodeState execute() override {
        std::cout << "Action: " << actionName << std::endl;
        return NodeState::SUCCESS;
    }
};

class PrintMessageNode : public BTNode {
private:
    std::string message;
public:
    PrintMessageNode(std::string name) : message(name) {}
    NodeState execute() override {
        std::cout << "Message: " << message << std::endl;
        return NodeState::SUCCESS;
    }
};

class InvertNode : public BTNode {
private:
    std::unique_ptr<BTNode> child;
public:
    InvertNode(std::unique_ptr<BTNode> _child) : child(std::move(_child)) {};
    NodeState execute() override {
        if (child->execute() == NodeState::SUCCESS) {
            return NodeState::FAILURE;
        }
        return NodeState::SUCCESS;
    }
};




int main() {
    Blackboard blackboard;
    blackboard.SetValue("PlayerDetected", 1);

    auto root = std::make_unique<SelectorNode>();
    auto sequence = std::make_unique<SequenceNode>();

    sequence->AddChild(std::make_unique<ConditionNode>(blackboard, "PlayerDetected", 1));
    sequence->AddChild(std::make_unique<ActionNode>("Attaquer"));

    auto invert = std::make_unique<InvertNode>(std::move(sequence));
    root->AddChild(std::move(invert));

    sequence = std::make_unique<SequenceNode>();
    sequence->AddChild(std::make_unique<ActionNode>("Patrouiller"));
    sequence->AddChild(std::make_unique<PrintMessageNode>("Coucou"));

    root->AddChild(std::move(sequence));

    NodeState result = root->execute();
    std::cout << "Root execution result: " << (result == NodeState::SUCCESS ? "SUCCESS" : "FAILURE") << std::endl;

    // Modifier les valeurs du tableau noir et ré-exécuter
    blackboard.SetValue("PlayerDetected", 0);
    result = root->execute();
    std::cout << "Root execution result after changing blackboard: " << (result == NodeState::SUCCESS ? "SUCCESS" : "FAILURE") << std::endl;

    return 0;
}