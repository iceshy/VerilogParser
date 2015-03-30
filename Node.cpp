#include "Node.h"
#include "Circuit.h"
#include "ParseError.h"

Node::Node()
{
}

Node::Node(string name, Circuit* circuit)
{
	this->name = name;
	this->circuit = circuit;
}

void Node::setNodeType(string nodeType)
{
	this->type = nodeType;
}

void Node::addOutput(EdgePointer edge)
{
	outputs.push_back(edge);
}

void Node::addInput(EdgePointer edge)
{
	inputs.push_back(edge);
}

void Node::assignTask(const string& typeOfNode, const string& portsDescription)
{
	this->setNodeType(typeOfNode);
	
	regex pattern1("\\.([A-Za-z0-9_]+)\\((.+?)\\),[^]+?\\.([A-Za-z0-9_]+)\\((.+?)\\)(?:,[^]+?\\.([A-Za-z0-9_]+)\\((.+?)\\))?(?:,[^]+?\\.([A-Za-z0-9_]+)\\((.+)\\))?(?:,[^]+?\\.([A-Za-z0-9_]+)\\((.+?)\\))?");
	smatch match;

	string sp = "[\\t\\n\\r ]*";
	string variableName("[a-zA-Z0-9\\[\\]]+");
	regex pattern2(sp + "(" + variableName + ")" + sp + "((?:," + sp + variableName + sp + ")+)");
	regex variableRegex("(" + variableName + ")");
	smatch m, n;

	if (regex_search(portsDescription, match, pattern1)){
		for (int i = 1; i < 10; i += 2){
			const string& Q = match[i].str();
			const string& W = match[i + 1].str();
			if (match[i].length() && match[i + 1].length()){
				if (match[i] == "Q" || match[i] == "Y"){
					addOutput(EdgePointer(circuit->edges.find(match[i + 1])));
					circuit->edges[match[i + 1]].setSourceNode(circuit->nodeIndex[name]);
				}
				else{
					addInput(EdgePointer(circuit->edges.find(match[i + 1])));
					circuit->edges[match[i + 1]].addDestinationNode(circuit->nodeIndex[name]);
				}
			}
		}
	}
	
	else
	if (regex_search(portsDescription, m, pattern2)){
		string gateName = m[1].str();
		if (!circuit->edges.count(gateName)) throw ParseError("wire \"" + gateName + "\" was not declared!");
		addOutput(EdgePointer(circuit->edges.find(gateName)));	
		circuit->edges[gateName].setSourceNode(circuit->nodeIndex[name]);
		string remainingList = m[2].str();
		while (regex_search(remainingList, n, variableRegex))
		{
			if (!circuit->edges.count(n[1].str())) throw ParseError("wire \"" + n[1].str() + "\" was not declared!");
			addInput(EdgePointer(circuit->edges.find(n[1])));
			circuit->edges[n[1].str()].addDestinationNode(circuit->nodeIndex[name]);
			int ind = remainingList.find(",", remainingList.find(",") + 1);
			remainingList = (ind >= 0)? remainingList.substr(ind) : "";
		}
	}
	
}

std::string Node::getName() const
{
	return name;
}

std::string Node::getType() const
{
	return type;
}


bool Node::isInputPort() const
{
	return (type == "INPUT_PORT");
}

bool Node::isOutputPort() const
{
	return (type == "OUTPUT_PORT");
}

Node& Node::inputNode(size_t index)
{
	if (index >= getInputsCount()) throw std::out_of_range("Node index out of range");
	return circuit->node(inputs[index]->getSource());
}

Node& Node::outputNode(size_t index)
{
	if (index >= getOutputsCount()) throw std::out_of_range("Node index out of range");
	size_t count = 0;
	for (int i = 0; i < outputs.size(); i++)
	{
		if (count + outputs[i]->nDestinations() > index)
		{
			return circuit->node(outputs[i]->getDestination(index - count));
		}
		count += outputs[i]->nDestinations();
	}
	throw std::out_of_range("Node index out of range");
}

size_t Node::getInputsCount()
{
	return inputs.size();
}

size_t Node::getOutputsCount()
{
	size_t s = 0;
	for (int i = 0; i < outputs.size(); i++)
		s += outputs[i]->nDestinations();
	return s;
}

