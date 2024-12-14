#pragma once

#include "config/Export.h"

#include <vector>
#include <cstdlib>

namespace dag
{
    class MetaCoroutine;
    class MetaField;
    class MetaOperation;

    class DAG_API MetaClass
    {
    public:
        typedef std::vector<MetaField *> MetaFieldArray;
        typedef std::vector<MetaOperation *> MetaOperationArray;
        typedef std::vector<MetaCoroutine *> MetaCoroutineArray;
    public:
        void addField( MetaField * field )
        {
            _fields.push_back(field);
        }

        MetaField * fieldForIndex(size_t i)
        {
            return _fields[i];
        }

        void addOperation( MetaOperation * op )
        {
            _ops.push_back(op);
        }

        MetaOperation * opForIndex(size_t i)
        {
            return _ops[i];
        }

        void addCoroutine( MetaCoroutine * coroutine )
        {
            _coroutines.push_back(coroutine);
        }

        MetaCoroutine * coroutineForIndex(size_t i)
        {
            return _coroutines[i];
        }

        //! Expects the arguments to be on the stack.
        void invoke( size_t index );
    private:
        MetaFieldArray _fields;
        MetaOperationArray _ops;
        MetaCoroutineArray _coroutines;
    };
}
