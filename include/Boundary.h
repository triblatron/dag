//
// Created by tony on 13/02/24.
//

#pragma  once

#include "config/Export.h"

#include "core/Node.h"

namespace dag
{
    class DAG_API Boundary final : public dagbase::Node
    {
    public:
        explicit Boundary(dagbase::KeyGenerator& keyGen, std::string name, dagbase::NodeCategory::Category category=dagbase::NodeCategory::CAT_NONE);

        Boundary(const Boundary& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

        Boundary(Boundary&& other) noexcept = default;

        explicit Boundary(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

        ~Boundary() override = default;

        Boundary& operator=(const Boundary& other);

        Boundary& operator=(Boundary&& other) = default;

        [[nodiscard]]const char* className() const override
        {
            return "Boundary";
        }

        Boundary* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Boundary(*this,facility,copyOp,keyGen);
        }

        Boundary* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const Node& other, dagbase::ComparisonFlags flags) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        void debug(dagbase::DebugPrinter& printer) const override;
    private:
    };
}
