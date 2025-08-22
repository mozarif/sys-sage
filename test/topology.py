import unittest
import sys_sage as syge 

class TestComponents(unittest.TestCase):
    def test_node(self):
        n = syge.Node(42)
        self.assertEqual(n.id, 42)
        self.assertEqual(n.type, syge.COMPONENT_NODE)
        self.assertEqual(n.GetComponentTypeStr(), "Node")
        self.assertEqual(n.parent, None)
        
        root = syge.Node(1)
        n.parent = root
        self.assertEqual(n.parent, root)
        
    def test_topologogy(self):
        t = syge.Topology()
        self.assertEqual(t.id, 0)
        self.assertEqual(t.type, syge.COMPONENT_TOPOLOGY)
        self.assertEqual(t.GetComponentTypeStr(), "Topology")
        
    def test_thread(self):
        root = syge.Node(0)
        t = syge.Thread(root, 42, "foo")
        self.assertEqual(root, t.parent)
        self.assertEqual(t.id, 42)
        self.assertEqual(t.type, syge.COMPONENT_THREAD)
        self.assertEqual(t.GetComponentTypeStr(), "HW_Thread")
        self.assertEqual(t.name, "foo")
    
    def test_core(self):
        root = syge.Node(0)
        c = syge.Core(root, 42, "foo")
        self.assertEqual(root, c.parent)
        self.assertEqual(c.id, 42)
        self.assertEqual(c.type, syge.COMPONENT_CORE)
        self.assertEqual(c.GetComponentTypeStr(), "Core")
        self.assertEqual(c.name, "foo")
        
    def test_cache(self):
        root = syge.Node(0)
        c = syge.Cache(root, 42, "3", 32, 2, 16)
        self.assertEqual(root, c.parent)
        self.assertEqual(c.id, 42)
        self.assertEqual(c.type, syge.COMPONENT_CACHE)
        self.assertEqual(c.GetComponentTypeStr(), "Cache")
        self.assertEqual(c.name, "Cache")
        self.assertEqual(c.cacheName, "3")
        self.assertEqual(c.cacheLevel, 3)
        self.assertEqual(c.cacheSize, 32)
        self.assertEqual(c.cacheAssociativity, 2)
        self.assertEqual(c.cacheLineSize, 16)
        
    def test_subdivision(self):
        root = syge.Node(0)
        s = syge.Subdivision(root, 42, "foo")
        self.assertEqual(root, s.parent)
        self.assertEqual(s.id, 42)
        self.assertEqual(s.type, syge.COMPONENT_SUBDIVISION)
        self.assertEqual(s.GetComponentTypeStr(), "Subdivision")
        self.assertEqual(s.name, "foo")
        
        s.subdivisionType = 3   
        self.assertEqual(s.subdivisionType, 3)
        
    def test_numa(self):
        root = syge.Node(0)
        n = syge.Numa(root, 42, 64)
        self.assertEqual(root, n.parent)
        self.assertEqual(n.id, 42)
        self.assertEqual(n.type, syge.COMPONENT_NUMA)
        self.assertEqual(n.GetComponentTypeStr(), "NUMA")
        self.assertEqual(n.name, "Numa")
        
        n.subdivisionType = 3   
        self.assertEqual(n.subdivisionType, 3)
    
    def test_chip(self):
        root = syge.Node(0)
        c = syge.Chip(root, 42, "foo", 5)
        self.assertEqual(root, c.parent)
        self.assertEqual(c.id, 42)
        self.assertEqual(c.type, syge.COMPONENT_CHIP)
        self.assertEqual(c.GetComponentTypeStr(), "Chip")
        self.assertEqual(c.name, "foo")
        self.assertEqual(c.chipType, 5)
        
        c.model = "model"
        self.assertEqual(c.model, "model")
        
        c.vendor = "vendor"
        self.assertEqual(c.vendor, "vendor")
        
        c.chipType = 6
        self.assertEqual(c.chipType, 6)
        
    def test_memory(self):
        root = syge.Node(0)
        m = syge.Memory(root, 0, "foo", 32)
        self.assertEqual(root, m.parent)
        self.assertEqual(m.id, 0)
        self.assertEqual(m.type, syge.COMPONENT_MEMORY)
        self.assertEqual(m.GetComponentTypeStr(), "Memory")
        self.assertEqual(m.name, "foo")
        self.assertEqual(m.size, 32)
        
        m.size = 64
        self.assertEqual(m.size, 64)
    
    def test_storage(self):
        root = syge.Node(0)
        s = syge.Storage(root)
        self.assertEqual(root, s.parent)
        self.assertEqual(s.type, syge.COMPONENT_STORAGE)
        self.assertEqual(s.GetComponentTypeStr(), "Storage")
        
        s.size = 64
        self.assertEqual(s.size, 64)    

    def test_qubit(self):
        q = syge.Qubit()
        q.SetProperties(1.0, 2.0, 3.0, 4.0, 5.0);
        self.assertEqual(q.t1, 1.0)
        self.assertEqual(q.t2, 2.0)
        self.assertEqual(q.readout_fidelity, 3.0);
        self.assertEqual(q.q1_fidelity, 4.0);
        self.assertEqual(q.readout_length, 5.0);
        self.assertEqual(q.GetComponentTypeStr(), "Qubit")

    def test_quantum_backend(self):
        qb = syge.QuantumBackend();
        s = 3
        gates = [syge.QuantumGate()] * s
        for gate in gates:
            qb.addGate(gate)

        self.assertEqual(qb.GetNumberofGates(), s)
        self.assertEqual(qb.GetAllGateTypes(), gates)
        self.assertEqual(qb.GetGatesBySize(0), gates)
        self.assertEqual(qb.GetComponentTypeStr(), "QuantumBackend")

    def test_atom_site(self):
        a = syge.AtomSite()
        # test if we can access the struct
        a.properties.nRows = 1
        a.properties.nColumns = 2
        a.properties.nAods = 3
        a.properties.nAodIntermediateLevels = 4
        a.properties.nAodCoordinates = 5
        a.properties.interQubitDistance = 6.0
        a.properties.interactionRadius = 7.0
        a.properties.blockingFactor = 8.0
        
    def test_children_insertion_and_removal(self):
        a = syge.Node()
        b = syge.Node()
        c = syge.Node()
        d = syge.Node()
        a.InsertChild(b)
        a.InsertChild(c)
        a.InsertChild(d)
        self.assertEqual(len(a.GetChildren()), 3)
        self.assertEqual(a.RemoveChild(b), 1)
        self.assertEqual(len(a.GetChildren()), 2)
        self.assertNotIn(b, a.GetChildren())
        self.assertIn(c, a.GetChildren())
        self.assertIn(d, a.GetChildren())
    
    def test_get_child(self):
        a = syge.Node()
        b = syge.Node(0)
        c = syge.Node(1)
        d = syge.Node(2)
        a.InsertChild(b)
        a.InsertChild(c)
        a.InsertChild(d)
        self.assertEqual(a.GetChild(0), b)
        self.assertEqual(a.GetChild(1), c)
        self.assertEqual(a.GetChild(4), None)
        
    def test_get_parent_by_type(self):
        a = syge.Cache()
        b = syge.Core(a)
        c = syge.Thread(b)
    def test_component_tree_consistency(self):
        a = syge.Node()
        b = syge.Node()
        c = syge.Node()
        a.InsertChild(b)
        b.InsertChild(c)
        
        self.assertEqual(0, a.CheckComponentTreeConsistency())
        
        c.SetParent(a)
        self.assertEqual(1, a.CheckComponentTreeConsistency())
        
    def test_get_deeper_components(self):
        a = syge.Node()
        b = syge.Node()
        c = syge.Node()
        d = syge.Node()
        a.InsertChild(b)
        a.InsertChild(c)
        c.InsertChild(d)
        array = []
        array += a.GetNthDescendents(1)
        self.assertEqual(2, len(array))
        
    def test_get_subcomponents_by_type(self):
        a = syge.Node()
        b = syge.Chip()
        c = syge.Memory()
        d = syge.Chip()
        
        a.InsertChild(b)
        a.InsertChild(c)
        c.InsertChild(d)
        
        array = []
        array += a.GetSubcomponentsByType(syge.COMPONENT_CHIP)
        
        self.assertEqual(2, len(array))
    
    def test_get_total_number_of_threads(self):
        a = syge.Node()
        b = syge.Thread()
        c = syge.Thread()
        d = syge.Node()
        e = syge.Thread()
        f = syge.Node()
        
        a.InsertChild(b)
        a.InsertChild(c)
        a.InsertChild(d)
        d.InsertChild(e)
        d.InsertChild(f)
        
        self.assertEqual(3, a.CountAllSubcomponentsByType(syge.COMPONENT_THREAD))
    
    def test_linearize_subtree(self):
        a = syge.Node()
        b = syge.Node()
        c = syge.Node()
        d = syge.Node()
        
        a.InsertChild(b)
        b.InsertChild(d)
        a.InsertChild(c)
        
        array = []
        array += a.GetComponentsInSubtree()
        
        self.assertEqual(array, [a, b, d, c])
        
    def test_tree_depth(self):
        a = syge.Node()
        b = syge.Node()
        c = syge.Node()
        d = syge.Node()
        e = syge.Node()
        f = syge.Node()
        g = syge.Node()
        
        a.InsertChild(b)
        b.InsertChild(c)
        a.InsertChild(d)
        d.InsertChild(e)
        e.InsertChild(f)
        a.InsertChild(g)
        
        self.assertEqual(3, a.GetSubtreeDepth())

    def test_attrib(self):
        c = syge.Component()
        c["foo"] = 1
        c["bar"] = 2.0
        c["foobar"] = "test"
        self.assertEqual(c["foo"], 1)
        self.assertEqual(c["bar"], 2.0)
        self.assertEqual(c["foobar"], "test")
        with self.assertRaises(AttributeError):
            c["fail"]
        # indexing follows lexicographical order of keys
        self.assertEqual(c[0], 2.0)
        self.assertEqual(c[1], 1)
        self.assertEqual(c[2], "test")
        with self.assertRaises(TypeError):
            c[1] = 2

if __name__ == "__main__":
    unittest.main()