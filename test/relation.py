import sys_sage
import unittest
import sys
import os

#class stream_suppressor:
#    def __init__(self, stream):
#        self.stream = stream
#
#    def __enter__(self):
#        self.original_stream = getattr(sys, self.stream)
#        self.devnull = open(os.devnull, "w")
#        setattr(sys, self.stream, self.devnull)
#
#    def __exit__(self, *_):
#        setattr(sys, self.stream, self.original_stream)
#        self.devnull.close()

class TestRelations(unittest.TestCase):
    def test_interconnection(self):
        foo = sys_sage.Component()
        bar = sys_sage.Component()
        v = [foo, bar]
        r = sys_sage.Relation(v)
        self.assertEqual(r.GetComponent(0), foo)
        self.assertEqual(r.GetComponent(1), bar)
        #with stream_suppressor("stderr"):
        #    self.assertEqual(r.GetComponent(2), None)
        self.assertEqual(r.GetComponent(2), None)
        self.assertEqual(r.components, v)
        self.assertEqual(foo.GetRelations(sys_sage.RELATION_TYPE_RELATION), [r])
        self.assertEqual(bar.GetRelations(sys_sage.RELATION_TYPE_RELATION), [r])

    def test_deletion(self):
        foo = sys_sage.Component()
        bar = sys_sage.Component()
        v = [foo, bar]
        r = sys_sage.Relation(v)
        r.Delete()
        
        self.assertEqual(len(foo.GetRelations(sys_sage.RELATION_TYPE_RELATION)), 0)
        self.assertEqual(len(bar.GetRelations(sys_sage.RELATION_TYPE_RELATION)), 0)

    def test_getters_and_setters(self):
        r = sys_sage.Relation([])

        self.assertEqual(r.type, sys_sage.RELATION_TYPE_RELATION)

        r.id = 2
        self.assertEqual(r.id, 2)
        self.assertTrue(r.ordered)

    def test_adding_and_updating_components(self):
        r = sys_sage.Relation([])
        self.assertEqual(len(r.components), 0)

        foo = sys_sage.Component()
        r.AddComponent(foo)
        self.assertTrue(r.ContainsComponent(foo))

        bar = sys_sage.Component()
        #with stream_suppressor("stderr"):
        #    self.assertEqual(r.UpdateComponent(bar, bar), 1)
        self.assertEqual(r.UpdateComponent(bar, bar), 1)

        r.UpdateComponent(foo, bar)
        self.assertEqual(r.components, [bar])

        foobar = sys_sage.Component()
        #with stream_suppressor("stderr"):
        #    self.assertEqual(r.UpdateComponent(1, foobar), 1)
        self.assertEqual(r.UpdateComponent(1, foobar), 1)

        r.UpdateComponent(0, foobar)
        self.assertEqual(r.components, [foobar])

    def test_attributes(self):
        r = sys_sage.Relation([])

        r["foo"] = 1
        r["bar"] = 2.0
        r["foobar"] = "test"
        self.assertEqual(r["foo"], 1)
        self.assertEqual(r["bar"], 2.0)
        self.assertEqual(r["foobar"], "test")
        with self.assertRaises(AttributeError):
            r["fail"]
        with self.assertRaises(TypeError):
            r[1] = 2

    def test_inheritance_data_path(self):
        foo = sys_sage.Component()
        bar = sys_sage.Component()

        r = sys_sage.DataPath(foo, bar, sys_sage.DATAPATH_ORIENTATION_ORIENTED, sys_sage.DATAPATH_TYPE_ANY)
        # test if inherited class can access members of the base class
        self.assertEqual(r.type, sys_sage.RELATION_TYPE_DATAPATH)

    def test_inheritance_quantum_gate(self):
        foo = sys_sage.Qubit()
        bar = sys_sage.Qubit()
        v = [foo, bar]
        r = sys_sage.QuantumGate(len(v), v)

        self.assertEqual(r.type, sys_sage.RELATION_TYPE_QUANTUMGATE)

        r.SetGateProperties("cx", 1.0, "[1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]")
        self.assertEqual(r.gate_type, sys_sage.QUANTUMGATE_TYPE_CNOT)
        self.assertEqual(r.fidelity, 1.0)
        self.assertEqual(r.unitary, "[1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]")

    def test_inheritance_coupling_map(self):
        foo = sys_sage.Qubit()
        bar = sys_sage.Qubit()
        r = sys_sage.CouplingMap(foo, bar)

        self.assertEqual(r.type, sys_sage.RELATION_TYPE_COUPLINGMAP)

        r.fidelity = 1.0
        self.assertEqual(r.fidelity, 1.0)

if __name__ == "__main__":
    unittest.main()
