/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Node.h"
#include "Agent.h"
#include "Condition.h"

namespace AI {

sa::IdGenerator<Id> Node::sIDs;

Node::Node(const ArgumentsType&) :
    id_(sIDs.Next())
{ }

Node::~Node() = default;

bool Node::AddNode(std::shared_ptr<Node>)
{
    return false;
}

void Node::SetCondition(std::shared_ptr<Condition> condition)
{
    condition_ = condition;
}

Node::Status Node::Execute(Agent& agent, uint32_t)
{
    if (condition_ && !condition_->Evaluate(agent, *this))
        return ReturnStatus(agent, Status::CanNotExecute);
    return ReturnStatus(agent, Status::Finished);
}

Node::Status Node::ReturnStatus(Agent& agent, Node::Status value)
{
    agent.context_.Set<NodeStatusType>(id_, value);
    return value;
}

void ForEachChildNode(const Node& parent, const std::function<Iteration(const Node& parent, const Node& child)>& callback)
{
    parent.VisitChildren([&](const Node& node)
    {
        callback(parent, node);
        ForEachChildNode(node, callback);
        return Iteration::Continue;
    });
}

}
