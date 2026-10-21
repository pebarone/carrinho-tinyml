from pathlib import Path
import hashlib
import json
import unittest
import zipfile


class ModelExportTests(unittest.TestCase):
    def test_exports_match_firmware_input_contract(self):
        model_dir = Path(__file__).parents[1] / 'model'
        for archive, project_id, datatype in (
            ('edge-impulse-arduino-v1.zip', 1139087, 'INT8'),
            ('edge-impulse-arduino-real-v1.zip', 1140184, 'INT8'),
            ('edge-impulse-arduino-real-v1-float32.zip', 1140184, 'FLOAT32'),
        ):
            with self.subTest(archive=archive), zipfile.ZipFile(model_dir / archive) as bundle:
                metadata_paths = [name for name in bundle.namelist()
                                  if name.endswith('/model-parameters/model_metadata.h')]
                self.assertEqual(len(metadata_paths), 1)
                metadata = bundle.read(metadata_paths[0]).decode('utf-8')
                for name, value in (
                    ('PROJECT_ID', project_id),
                    ('RAW_SAMPLE_COUNT', 10),
                    ('RAW_SAMPLES_PER_FRAME', 1),
                    ('INTERVAL_MS', 100),
                    ('LABEL_COUNT', 3),
                    ('FREQUENCY', 10),
                ):
                    self.assertRegex(metadata, rf'#define EI_CLASSIFIER_{name}\s+{value}\b')
                for direction in ('INPUT', 'OUTPUT'):
                    self.assertRegex(
                        metadata,
                        rf'#define EI_CLASSIFIER_TFLITE_{direction}_DATATYPE\s+'
                        rf'EI_CLASSIFIER_DATATYPE_{datatype}\b',
                    )

    def test_active_export_integrity(self):
        model_dir = Path(__file__).parents[1] / 'model'
        meta = json.loads((model_dir / 'export-real-v1-float32.json').read_text())
        for file_key, hash_key in (('file', 'sha256'),
                                   ('reference_model', 'reference_model_sha256')):
            digest = hashlib.sha256((model_dir / meta[file_key]).read_bytes()).hexdigest()
            self.assertEqual(digest, meta[hash_key])


if __name__ == '__main__':
    unittest.main()
