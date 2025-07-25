import unittest
import sys_sage as syge 

class TestDatapaths(unittest.TestCase):
    def test_constructor_1(self):
        a = syge.Component()
        b = syge.Component()
        dp = syge.DataPath(a,b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(syge.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(syge.DATAPATH_TYPE_PHYSICAL, dp.type)
    def test_constructor_2(self):
        a = syge.Component()
        b = syge.Component()
        dp = syge.DataPath(a,b, syge.DATAPATH_ORIENTATION_ORIENTED, 5.0, 42.0)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(syge.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(syge.DATAPATH_TYPE_NONE, dp.type)
        self.assertEqual(5.0, dp.bandwidth)
        self.assertEqual(42.0, dp.latency)
    def test_constructor_3(self):
        a = syge.Component()
        b = syge.Component()
        dp = syge.DataPath(a,b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL, 5.0, 42.0)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(syge.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(syge.DATAPATH_TYPE_PHYSICAL, dp.type)
        self.assertEqual(5.0, dp.bandwidth)
        self.assertEqual(42.0, dp.latency)
    def test_unidirectional_data_path(self):
        a = syge.Component()
        b = syge.Component()
        dp = syge.DataPath(a,b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)
        self.assertEqual([], list(a.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_INCOMING)))
        self.assertEqual([dp], list(a.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_OUTGOING)))
        
        self.assertEqual([dp], list(b.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_INCOMING)))
        self.assertEqual([], list(b.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_OUTGOING)))
    def test_bidirectional_data_path(self):
        a = syge.Component()
        b = syge.Component()
        dp = syge.DataPath(a,b, syge.DATAPATH_ORIENTATION_BIDIRECTIONAL, syge.DATAPATH_TYPE_PHYSICAL)
        self.assertEqual([dp], list(a.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_INCOMING)))
        self.assertEqual([dp], list(a.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_OUTGOING)))
        self.assertEqual([dp], list(b.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_INCOMING)))
        self.assertEqual([dp], list(b.GetAllDataPaths(syge.DATAPATH_TYPE_ANY, syge.DATAPATH_DIRECTION_OUTGOING)))
        
    def test_get_data_path_by_type(self):
        a = syge.Component()
        b = syge.Component()
        dp1 = syge.DataPath(a, b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_LOGICAL)
        dp2 = syge.DataPath(a, b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)
        dp3 = syge.DataPath(a, b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)
        dp4 = syge.DataPath(b, a, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)

        self.assertEqual(dp1, a.GetDataPathByType(syge.DATAPATH_TYPE_LOGICAL, syge.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp2, a.GetDataPathByType(syge.DATAPATH_TYPE_PHYSICAL, syge.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(None, a.GetDataPathByType(syge.DATAPATH_TYPE_L3CAT, syge.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp4, a.GetDataPathByType(syge.DATAPATH_TYPE_PHYSICAL, syge.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual(dp4, b.GetDataPathByType(syge.DATAPATH_TYPE_PHYSICAL, syge.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp2, b.GetDataPathByType(syge.DATAPATH_TYPE_PHYSICAL, syge.DATAPATH_DIRECTION_ANY))

    def test_get_all_data_paths_by_type(self):
        a = syge.Component()
        b = syge.Component()
        dp1 = syge.DataPath(a, b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_LOGICAL)
        dp2 = syge.DataPath(a, b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)
        dp3 = syge.DataPath(a, b, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)
        dp4 = syge.DataPath(b, a, syge.DATAPATH_ORIENTATION_ORIENTED, syge.DATAPATH_TYPE_PHYSICAL)

        v = a.GetAllDataPaths(syge.DATAPATH_TYPE_LOGICAL, syge.DATAPATH_DIRECTION_INCOMING)
        self.assertEqual([], list(v))

        v = a.GetAllDataPaths(syge.DATAPATH_TYPE_PHYSICAL, syge.DATAPATH_DIRECTION_INCOMING)
        self.assertEqual([dp4], list(v))

        v = a.GetAllDataPaths(syge.DATAPATH_TYPE_LOGICAL, syge.DATAPATH_DIRECTION_OUTGOING)
        self.assertEqual([dp1], list(v))

        v = a.GetAllDataPaths(syge.DATAPATH_TYPE_PHYSICAL, syge.DATAPATH_DIRECTION_OUTGOING)
        self.assertEqual([dp2, dp3], list(v))

        v = a.GetAllDataPaths(syge.DATAPATH_TYPE_PHYSICAL, syge.DATAPATH_DIRECTION_ANY)
        self.assertEqual([dp2, dp3, dp4], list(v))

if __name__ == "__main__":
    unittest.main()