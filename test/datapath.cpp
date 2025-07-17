#include <boost/ut.hpp>

#include "sys-sage.hpp"

using namespace boost::ut;
using namespace sys_sage;

static suite<"data-path"> _ = []
{
    "Constructors"_test = []
    {
        "Constructor #1"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathType::Physical == dp.GetDataPathType());
        };
        "Constructor #2"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathType::None == dp.GetDataPathType());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
        "Constructor #3"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathType::Physical == dp.GetDataPathType());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
    };

    "Unidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};

        // note: defining the constants in the enums.hpp header file the way they
        // are now with const-expression within individual namespaces can lead to
        // subtle bugs.
        //
        // For intance, the `GetAllDataPaths` function that returns a vector has
        // two arguments, which are both default initialized. If I were to call the
        // function like `GetAllDataPaths(DataPathDirection::Incoming)`, because
        // I thought that the first argument, which is of type DataPathType::type,
        // would be default initialized with `DataPathType::Any`. This isn't the
        // case, so I need to explicitely state that.
        //
        // The problem is however, that the compiler doesn't warn me, because it
        // assumes that the single argument that I have given is meant for the
        // first parameter. Since both parameters are actually of the same type
        // under the hood (because `DataPathType::type == int32_t == DataPathDirection::type`),
        // the compiler will silently "cast" them and thus accept one type as another.
        //
        // The solution would be to use scoped enums. This gives us the benefit of
        // namespaces, because each enum is only defined within its own class,
        // and the compiler won't allow implicit conversions between enums of
        // different scopes.

        expect(that % a.GetAllDataPaths(DataPathType::Any, DataPathDirection::Incoming).empty());
        expect(that % std::vector{&dp} == a.GetAllDataPaths(DataPathType::Any, DataPathDirection::Outgoing));

        expect(that % b.GetAllDataPaths(DataPathType::Any, DataPathDirection::Outgoing).empty());
        expect(that % std::vector{&dp} == b.GetAllDataPaths(DataPathType::Any, DataPathDirection::Incoming));
    };

    "Bidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, DataPathOrientation::Bidirectional, DataPathType::Physical};

        expect(that % std::vector{&dp} == a.GetAllDataPaths(DataPathType::Any, DataPathDirection::Incoming));
        expect(that % std::vector{&dp} == a.GetAllDataPaths(DataPathType::Any, DataPathDirection::Outgoing));

        expect(that % std::vector{&dp} == b.GetAllDataPaths(DataPathType::Any, DataPathDirection::Incoming));
        expect(that % std::vector{&dp} == b.GetAllDataPaths(DataPathType::Any, DataPathDirection::Outgoing));
    };

    "Get data path by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, DataPathOrientation::Oriented, DataPathType::Logical};
        DataPath dp2{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp3{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp4{&b, &a, DataPathOrientation::Oriented, DataPathType::Physical};

        expect(that % &dp1 == a.GetDataPathByType(DataPathType::Logical, DataPathDirection::Outgoing));
        expect(that % &dp2 == a.GetDataPathByType(DataPathType::Physical, DataPathDirection::Outgoing));
        expect(that % nullptr == a.GetDataPathByType(DataPathType::L3CAT, DataPathDirection::Outgoing));
        expect(that % &dp4 == a.GetDataPathByType(DataPathType::Physical, DataPathDirection::Incoming));
        expect(that % &dp4 == b.GetDataPathByType(DataPathType::Physical, DataPathDirection::Outgoing));
        // using a bit-wise OR-operation does only work correctly, if every
        // constant is a power of 2. Otherwise, we can not guarantee that the
        // bits of different constants don't overlap. In this case,
        // DataPathDirection::Incoming = 3 and DataPathDirection::Outgoing == 2,
        // so OR-ing them would give again 3, which is only DataPathDirection::Incoming.
        // Also, I'm not sure if the implementation of GetAllDataPaths in Component.cpp
        // would be able to recongnize multiple directions, because it only checks
        // for equality.
        //
        //a.GetAllDataPaths(&v, DataPathType::Physical, DataPathDirection::Incoming | DataPathDirection::Outgoing);
        expect(that % &dp2 == b.GetDataPathByType(DataPathType::Physical, DataPathDirection::Any));
    };

    "Get all data paths by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, DataPathOrientation::Oriented, DataPathType::Logical};
        DataPath dp2{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp3{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp4{&b, &a, DataPathOrientation::Oriented, DataPathType::Physical};

        //boost::ut::log << "a: " << &a << '\n';
        //boost::ut::log << "b: " << &b << '\n';

        //boost::ut::log << "dp4 source: " << dp4.GetSource() << '\n';
        //boost::ut::log << "dp4 taget: " << dp4.GetTarget() << '\n';

        "Get all incoming logical data paths"_test = [&]()
        {
            expect(that % dp4.GetSource() == &b);
            expect(that % dp4.GetTarget() == &a);

            expect(that % dp4.GetDataPathType() == DataPathType::Physical);

            std::vector<DataPath *> vec;
            a.GetAllDataPaths(&vec, DataPathType::Any, DataPathDirection::Any);
            expect(that % vec.size() == static_cast<size_t>(4));

            boost::ut::log << "dp1: " << &dp1 << '\n';
            boost::ut::log << "dp2: " << &dp2 << '\n';
            boost::ut::log << "dp3: " << &dp3 << '\n';
            boost::ut::log << "dp4: " << &dp4 << '\n';

            for (DataPath * dp : vec)
              if (dp->GetDataPathType() == DataPathType::Logical && dp->GetComponent(0) == &a)
                boost::ut::log << dp << '\n';
//--------------------------------------------------------------------------------- 

            std::vector<DataPath *> v;
            a.GetAllDataPaths(&v, DataPathType::Logical, DataPathDirection::Incoming);
            expect((that % v.empty()) >> fatal);
        };

        "Get all incoming physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.GetAllDataPaths(&v, DataPathType::Physical, DataPathDirection::Incoming);
            expect(std::vector{&dp4} == v);
        };

        "Get all outgoing logical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.GetAllDataPaths(&v, DataPathType::Logical, DataPathDirection::Outgoing);
            expect(that % std::vector{&dp1} == v);
        };

        "Get all outgoing physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.GetAllDataPaths(&v, DataPathType::Physical, DataPathDirection::Outgoing);
            expect(that % std::vector{&dp2, &dp3} == v);
        };

        "Get all physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;

            // using a bit-wise OR-operation does only work correctly, if every
            // constant is a power of 2. Otherwise, we can not guarantee that the
            // bits of different constants don't overlap. In this case,
            // DataPathDirection::Incoming = 3 and DataPathDirection::Outgoing == 2,
            // so OR-ing them would give again 3, which is only DataPathDirection::Incoming.
            // Also, I'm not sure if the implementation of GetAllDataPaths in Component.cpp
            // would be able to recongnize multiple directions, because it only checks
            // for equality.
            //
            //a.GetAllDataPaths(&v, DataPathType::Physical, DataPathDirection::Incoming | DataPathDirection::Outgoing);
            a.GetAllDataPaths(&v, DataPathType::Physical, DataPathDirection::Any);
            expect(that % std::vector{&dp2, &dp3, &dp4} == v);
        };
    };
};
