//
// Created by Tony Horrobin on 14/11/2026.
//

#pragma once

#include "NodeEditorCommand.h"

#include "core/Node.h"
#include "core/Port.h"
#include "core/SignalPath.h"
#include "core/GraphNode.h"
#include "NodeEditorLive.h"
#include "test/TestUtils.h"
#include <gtest/gtest.h>

struct ExistingNodeEditorLive
{
    using PropertyMap = dagbase::VectorMap<std::string, dagbase::Variant>;

    void configure(dagbase::ConfigurationElement& config)
    {
        dagbase::ConfigurationElement::readConfig(config, "class", &className);
        dagbase::ConfigurationElement::readConfig(config, "count", &count);

        if (auto element=config.findElement("path"); element)
        {
            element->eachChild([this](dagbase::ConfigurationElement& child)
            {
                auto entry = child.value().cast(dagbase::Variant::TYPE_UINT);
                if (entry.has_value())
                {
                    path.emplace_back(entry.asUint32(~0U));
                }
                return true;
            });
            ASSERT_EQ(element->numChildren(), path.size()) << "Expected all children of path element to be added";
        }

        if (auto element = config.findElement("from"); element)
        {
            element->eachChild([this](dagbase::ConfigurationElement& child)
            {
                from.emplace(child.name(), child.value());
                return true;
            });
        }
        if (auto element = config.findElement("to"); element)
        {
            element->eachChild([this](dagbase::ConfigurationElement& child)
            {
                to.emplace(child.name(), child.value());
                return true;
            });
        }
        if (auto element = config.findElement("graph"); element)
        {
            element->eachChild([this](dagbase::ConfigurationElement& child)
            {
                nestedGraph.emplace(child.name(), child.value());
                return true;
            });
        }
        if (auto element = config.findElement("properties"); element)
        {
            element->eachChild([this](dagbase::ConfigurationElement& child)
            {
                if (child.numChildren()==0)
                    properties.emplace(child.name(), child.value());
                else if (child.name() == "value")
                {
                    dagbase::Variant value;
                    dagbase::ConfigurationElement::readConfig(*child.parent(), "value", &value);
                    properties.emplace(child.name(), value);
                }
                return true;
            });
        }
    }

    template<typename Obj>
    static bool areAllTrue(const PropertyMap& properties, const Obj* obj)
    {
        bool allTrue = true;
        for (const auto& p : properties)
        {
            auto actual = obj->find(p.first);
            auto casted = p.second.cast(actual.index());
            if (actual.has_value() != casted.has_value() ||  (actual.has_value() && actual != casted))
            {
                allTrue = false;
            }
        }

        return allTrue;
    }

    void makeItSo(dag::NodeEditorLive& sut) const
    {
        auto graph = sut.graph(path);
        ASSERT_NE(nullptr, graph);
        std::uint32_t actualCount{0};
        if (graph)
        {
            if (className == "SignalPath")
            {
                // Dispatch a search on SignalPaths in the specified Graph
                graph->eachSignalPath([this, &actualCount](const dagbase::SignalPath* signalPath)
                {
                    bool allTrue = true;
                    bool checked = false;
                    if (!from.empty())
                    {
                        checked = true;
                        allTrue = allTrue && areAllTrue(from, signalPath->source());
                    }
                    if (!to.empty())
                    {
                        checked = true;
                        allTrue = allTrue && areAllTrue(to, signalPath->dest());
                    }

                    if (!properties.empty())
                    {
                        checked = true;
                        allTrue = allTrue && areAllTrue(properties, signalPath);
                    }

                    if (checked && allTrue)
                    {
                        ++actualCount;
                    }
                    return true;
                });
            }
            else if (className == "Port")
            {
                graph->eachPort([this, &actualCount](const dagbase::Port* port)
                {
                    bool allTrue = true;
                    bool checked = false;

                    if (!properties.empty())
                    {
                        checked = true;
                        allTrue = allTrue && areAllTrue(properties, port);
                    }

                    if (checked && allTrue)
                    {
                        ++actualCount;
                    }

                    return true;
                });
            }
            else if (className == "Node")
            {
                graph->eachNode([this, &actualCount](const dagbase::Node* node)
                {
                    bool allTrue = true;
                    bool checked = false;

                    if (const auto* graphNode = dynamic_cast<const dagbase::GraphNode*>(node); graphNode && !nestedGraph.empty())
                    {
                        checked = true;
                        allTrue = allTrue && areAllTrue(to, graphNode->graph());
                    }

                    if (!properties.empty())
                    {
                        checked = true;
                        allTrue = allTrue && areAllTrue(properties, node);
                    }

                    if (checked && allTrue)
                    {
                        ++actualCount;
                    }

                    return true;
                });
            }

            std::ostringstream propertyStr;
            for (const auto& prop : properties)
            {
                propertyStr << prop.first << " = " << prop.second << '\n';
            }
            ASSERT_EQ(count, actualCount) << "Expected class " << className << " to have properties:\n" << propertyStr.str();

        }
        else
        {
            FAIL() << "Expected a valid Graph";
        }
    }

    std::string className;
    dag::NodeEditorLive::GraphChildPath path;

    PropertyMap from;
    PropertyMap to;
    PropertyMap nestedGraph;
    PropertyMap properties;
    std::uint32_t count{1};
};

using NodeEditorLiveAssertion = Assertion<dag::NodeEditorLive>;

class VerificationNodeEditorLive : public dag::NodeEditorCommand
{
public:
    VerificationNodeEditorLive(NodeEditorCommand* decoratee);

    void setCaseName(const std::string& name)
    {
        _caseName = name;
    }

    void configure(dagbase::ConfigurationElement& config) override;

    dagbase::Status makeItSo(const std::pmr::vector<dagbase::Variant>& args, std::pmr::vector<dagbase::Variant>* results) override;

    void verify();
private:
    std::string _caseName;
    std::string _str;
public:
    dagbase::Status status{dagbase::Status::STATUS_OK};
    dagbase::Status actualStatus{dagbase::Status::STATUS_OK};
    using ExistingArray = std::vector<ExistingNodeEditorLive>;
    ExistingArray existing;
    using AssertionArray = std::vector<NodeEditorLiveAssertion>;
    AssertionArray assertions;
};
