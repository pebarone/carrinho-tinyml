import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('dataset', Path(__file__).parents[1] / 'tools/dataset.py')
dataset = importlib.util.module_from_spec(spec)
spec.loader.exec_module(dataset)


class DatasetTests(unittest.TestCase):
    def test_protocol(self):
        self.assertIsNone(dataset.parse_line('# collection'))
        self.assertEqual(dataset.parse_line('100,22.5,1'), (100, 22.5, 1))
        for bad in ('1,nan,1', '1,401,1', '1,20,2', '-1,20,1', 'garbage'):
            with self.assertRaises(ValueError):
                dataset.parse_line(bad)

    def test_invalid_and_gap_discard_window(self):
        rows = [(i * 100, 20, 1) for i in range(25)]
        rows[5] = (500, -1, 0)
        self.assertEqual(len(list(dataset.windows(rows))), 1)
        rows = [(i * 100, 20, 1) for i in range(5)] + [(2000 + i * 100, 20, 1) for i in range(10)]
        self.assertEqual(len(list(dataset.windows(rows))), 1)
        with self.assertRaises(ValueError):
            list(dataset.windows([(100, 20, 1), (100, 20, 1)]))

    def test_provenance_and_session_split(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            dataset.synthetic(root / 'raw')
            with self.assertRaises(ValueError):
                dataset.prepare(root / 'raw', root / 'blocked')
            dataset.prepare(root / 'raw', root / 'export', True)
            manifest = json.loads((root / 'export/manifest.json').read_text())
            self.assertEqual(len(manifest), 600)
            train = {m['session'] for m in manifest if m['file'].startswith('training/')}
            test = {m['session'] for m in manifest if m['file'].startswith('testing/')}
            self.assertFalse(train & test)
            self.assertEqual(sum(m['file'].startswith('testing/') for m in manifest), 120)
            path = next((root / 'raw').glob('*.csv'))
            path.write_text('alterado')
            with self.assertRaises(ValueError):
                dataset.prepare(root / 'raw', root / 'tampered', True)


if __name__ == '__main__':
    unittest.main()
