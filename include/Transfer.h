// ReSharper disable CppClangTidyBugproneMacroParentheses
#pragma once

#include "config/Export.h"

#include "SignalPathDef.h"
#include "Port.h"

#include <any>
#include <variant>
#include <type_traits>
#include <atomic>

namespace nbe
{
    class FieldNode
    {
    public:
        virtual ~FieldNode() = default;

        void setInput(int port, const std::any& value)
        {
            _inputs[port] = value;
        }

        std::any input(int port)
        {
            return _inputs[port];
        }

        std::any output(int port) const
        {
            return _outputs[port];
        }
    protected:
        typedef std::vector<std::any> FieldArray;
        FieldArray _inputs;
        FieldArray _outputs;
    };

    class Output : public FieldNode
    {
    public:
        Output(int i)
            :
            _foo(i)
        {
            _outputs.push_back(std::any(1));
        }

        const int& foo() const
        {
            return _foo;
        }
    public:
        int _foo;
    };

    class Input : public FieldNode
    {
    public:
        Input()
            :
            _foo(0)
        {
            _inputs.push_back(std::any(0));
        }

        void setFoo(const int& i)
        {
            _foo = i;
            _inputs[0] = std::any(i);
        }

        int foo() const
        {
            return _foo;
        }
    public:
        int _foo;
    };

    class AbstractAnyNode
    {
    public:
        virtual ~AbstractAnyNode() = default;

        virtual void setInput(int port, std::any value) = 0;

        virtual std::any output(int port) const = 0;
    };

    class FooAbstractAny : public AbstractAnyNode
    {
    public:
        FooAbstractAny(int i)
            :
            _foo(i)
        {
            // Do nothing.
        }

        void setInput(int port, std::any value) override
        {
            switch (port)
            {
            case 0:
                _foo = std::any_cast<int>(value);
                break;
            }
        }

        std::any output(int port) const override
        {
            switch (port)
            {
            case 0:
                return std::any(_foo);
            default:
                return std::any();
            }
        }

        int foo() const
        {
            return _foo;
        }
    private:
        int _foo;
    };

    class TypedSwitchNode
    {
    public:
        virtual ~TypedSwitchNode() = default;

        virtual void setIntInput(int port, int value) = 0;

        virtual int intOutput(int port) const = 0;
    private:
    };

    class FooTypedSwitch : public TypedSwitchNode
    {
    public:
        FooTypedSwitch(int i)
            :
            _foo(i)
        {
            // Do nothing.
        }

        void setIntInput(int port, int value) override
        {
            switch (port)
            {
            case 0:
                _foo = value;
                break;
            }
        }

        int intOutput(int port) const override
        {
            switch (port)
            {
            case 0:
                return _foo;
            default:
                return 0;
            }
        }

        int foo() const
        {
            return _foo;
        }
    private:
        int _foo;
    };

    class VariantNode
    {
    public:
        typedef nbe::Value Field;
    public:
        void setInput(std::int64_t port, const Field& value)
        {
            _inputs[port] = value;
        }

        const Field& input(std::int64_t port) const
        {
            return _inputs[port];
        }

        const Field& output(std::int64_t port) const
        {
            return _outputs[port];
        }
    protected:
        typedef std::vector<Field> FieldArray;
        FieldArray _inputs;
        FieldArray _outputs;
        void addInput(const Field& value)
        {
            _inputs.push_back(value);
        }
        void addOutput(const Field& value)
        {
            _outputs.push_back(value);
        }
    };

    class Baz : public VariantNode
    {
    public:
        Baz(std::int64_t i)
        {
            addOutput(nbe::Value(i));
        }
    private:
    };

    class Qux : public VariantNode
    {
    public:
        Qux()
        {
            addInput(nbe::Value(std::int64_t{ 0 }));
        }
    private:
    };

    class Transfer
    {
    public:
        virtual ~Transfer() = default;
        
        virtual bool valid() const = 0;

        virtual void makeItSo() = 0;
    };

    template<class S, class D, class T>
    class TypedTransferMemFun final : public Transfer
    {
    public:
        typedef const T& (S::* Getter)() const;
        typedef void (D::* Setter)(const T&);
    public:
        TypedTransferMemFun(const S* source, D* dest, Getter getter, Setter setter)
            :
            _source(source),
            _dest(dest),
            _getter(getter),
            _setter(setter)
        {
            // Do nothing.
        }

        bool valid() const override
        {
            return _source != nullptr && _dest != nullptr && _getter != nullptr && _setter != nullptr;
        }

        void makeItSo() override
        {
            // The virtual call should be optimised because the class is final.
            if (valid())
            {
                (_dest->*_setter)((_source->*_getter)());
            }
        }
    private:
        const S* _source;
        D* _dest;
        Getter _getter;
        Setter _setter;
    };

    class AnyTransfer : public Transfer
    {
    public:
        AnyTransfer(const SignalPathDef& signalPath, const FieldNode& source, FieldNode& dest)
            :
            _signalPath(signalPath),
            _source(source),
            _dest(dest)
        {
        }

        bool valid() const override
        {
            return true;
        }

        void makeItSo() override
        {
            _dest.setInput(_signalPath.dest.port, _source.output(_signalPath.source.port));
        }
    private:
        const SignalPathDef& _signalPath;
        const FieldNode& _source;
        FieldNode& _dest;
    };

    class AbstractAnyTransfer : public Transfer
    {
    public:
        AbstractAnyTransfer(const SignalPathDef& signalPath, const AbstractAnyNode& source, AbstractAnyNode& dest)
            :
            _signalPath(signalPath),
            _source(source),
            _dest(dest)
        {
            // Do nothing.
        }

        bool valid() const override
        {
            return true;
        }

        void makeItSo() override
        {
            _dest.setInput(_signalPath.dest.port, _source.output(_signalPath.source.port));
        }
    private:
        const SignalPathDef& _signalPath;
        const AbstractAnyNode& _source;
        AbstractAnyNode& _dest;
    };

    class TypedSwitchTransfer : public Transfer
    {
    public:
        TypedSwitchTransfer(const SignalPathDef& signalPath, const TypedSwitchNode& source, TypedSwitchNode& dest)
            :
            _signalPath(signalPath),
            _source(source),
            _dest(dest)
        {
            // Do nothing.
        }

        bool valid() const override
        {
            return true;
        }

        void makeItSo() override
        {
            _dest.setIntInput(_signalPath.dest.port, _source.intOutput(_signalPath.source.port));
        }
    private:
        const SignalPathDef& _signalPath;
        const TypedSwitchNode& _source;
        TypedSwitchNode& _dest;
    };

    template<class T>
    class TypedPointerTransfer final : public Transfer
    {
    public:
        TypedPointerTransfer(const T* source, T* dest)
            :
            _source(source),
            _dest(dest)
        {
            // Do nothing.
        }

        bool valid() const override
        {
            return _source != nullptr && _dest != nullptr;
        }

        void makeItSo() override
        {
            *_dest = *_source;
        }
    private:
        const T* _source;
        T* _dest;
    };

    class VariantTransfer : public Transfer
    {
    public:
        VariantTransfer(const SignalPathDef* signalPath, const VariantNode* source, VariantNode* dest)
            :
            _signalPath(signalPath),
            _source(source),
            _dest(dest)
        {
            // Do nothing.
        }

        bool valid() const override
        {
            return _signalPath != nullptr && _source != nullptr && _dest != nullptr;
        }

        void makeItSo() override
        {
            _dest->setInput(_signalPath->dest.port, _source->output(_signalPath->source.port));
        }
    private:
        const SignalPathDef* _signalPath;
        const VariantNode* _source;
        VariantNode* _dest;

    };

    class VariantPortTransfer : public Transfer
    {
    public:
        VariantPortTransfer(const nbe::VariantPort* source, nbe::VariantPort* dest)
        :
        _source(source),
        _dest(dest)
        {
            // Do nothing.
        }

        explicit VariantPortTransfer(const nbe::VariantPort* source)
        :
        _source(source)
        {
            // Do nothing.
        }

        void setDest(nbe::VariantPort* dest)
        {
            _dest = dest;
        }

        [[nodiscard]]bool valid() const override
        {
            return _source != nullptr && _dest!=nullptr && _source->isConnectedTo(_dest);
        }

        void makeItSo() override
        {
            _dest->setValue(_source->value());
        }
    private:
        const nbe::VariantPort* _source;
        nbe::VariantPort* _dest;
    };
    
    class PortTransfer : public Transfer
    {
    public:
        PortTransfer(const nbe::ValuePort* src, nbe::ValuePort* dst)
            :
            _src(src),
            _dst(dst)
        {
            // Do nothing.
        }

        explicit PortTransfer(const nbe::ValuePort* src)
        :
        _src(src),
        _dst(nullptr)
        {
            // Do nothing.
        }

        void setDest(nbe::ValuePort* dst)
        {
            _dst = dst;
        }

        [[nodiscard]]bool valid() const override
        {
            return _src != nullptr && _dst != nullptr && _src->isConnectedTo(_dst);
        }

        void makeItSo() override
        {
            _dst->setValue(_src->value());
        }
    private:
        const nbe::ValuePort* _src;
        nbe::ValuePort* _dst;
    };

    template<typename T>
    using EnableIfFundamental = std::enable_if<std::is_fundamental_v<T>>;

    template<typename T>
    class TypedPort;

    template<typename T>
    class TypedPortTransfer : public Transfer
    {
    public:
        TypedPortTransfer(const nbe::TypedPort<T>* src, nbe::TypedPort<T>* dst)
            :
            _src(src),
            _dst(dst)
        {
            // Do nothing.
        }

        bool valid() const override
        {
            return _src != nullptr && _dst != nullptr && _src->isConnectedTo(_dst);
        }

        void makeItSo() override
        {
            _dst->setValue(_src->value());
        }
    private:
        const nbe::TypedPort<T>* _src;
        nbe::TypedPort<T>* _dst;
    };
    
    template<typename T, typename = Value::EnableIfSupported<T>>
    class TypedTransfer : public Transfer
    {
    public:
        TypedTransfer(const T* source)
            :
            _source(source),
            _dest(nullptr)
        {
            // Do nothing.
        }

        void setDest(T* dest)
        {
            _dest = dest;
        }

        bool valid() const override
        {
            return _source != nullptr && _dest != nullptr;
        }

        void makeItSo() override
        {
            *_dest = *_source;
        }

        [[nodiscard]]bool hasSource(const T* source) const
        {
            return _source == source;
        }

        [[nodiscard]]bool hasDest(T* dest) const
        {
            return _dest == dest;
        }
    private:
        const T* _source;
        T* _dest;
    };
    
#define TRANSFER_DEFINE(Name, Source, Dest, Setter, Getter) \
    template<typename Source, typename Dest> \
    class Name : public nbe::Transfer \
    {\
    public:\
        Name(const Source& source, Dest& dest)\
        :\
        _source(source),\
        _dest(dest)\
        {\
        }\
        \
        bool valid() const override\
        {\
			return true;\
		}\
        void makeItSo() override\
        {\
            _dest.Setter(_source.Getter());\
        }\
    private:\
        const Source& _source;\
        Dest& _dest;\
    }
}
