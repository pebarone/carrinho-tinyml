"""Optional model regressions: install TensorFlow to run these on the desktop."""
from pathlib import Path
import re
import unittest
import zipfile


class RealModelInferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        try:
            import numpy as np
            import tensorflow as tf
        except ModuleNotFoundError as error:
            if error.name not in ('numpy', 'tensorflow'):
                raise
            raise unittest.SkipTest('Model regressions require numpy and tensorflow') from error
        cls.np = np
        path = Path(__file__).parents[1] / 'model/cp2-carrinho-real-v1.float32.tflite'
        cls.model = tf.lite.Interpreter(model_path=str(path),
                                       experimental_preserve_all_tensors=True)
        cls.model.allocate_tensors()
        cls.input = cls.model.get_input_details()[0]
        cls.output = cls.model.get_output_details()[0]

    def infer(self, distances):
        # Same Raw Data scale as the exported Edge Impulse DSP, once only.
        values = self.np.asarray([distances], dtype=self.np.float32) * .01
        self.model.set_tensor(self.input['index'], values)
        self.model.invoke()
        return self.model.get_tensor(self.output['index'])[0]

    def test_intermediate_and_far_windows_remain_valid(self):
        for distance in (20, 25, 40, 41, 45, 50, 54, 55, 62, 100, 200, 400):
            with self.subTest(distance=distance):
                scores = self.infer([distance] * 10)
                self.assertTrue(self.np.isfinite(scores).all())
                self.assertTrue(((scores >= 0) & (scores <= 1)).all())
                self.assertAlmostEqual(float(scores.sum()), 1.0, places=5)

    def test_far_distances_are_not_clipped_to_int8_range(self):
        self.assertEqual(self.input['dtype'], self.np.float32)
        self.assertEqual(self.output['dtype'], self.np.float32)
        for distance in (62, 100, 200, 400):
            self.infer([distance] * 10)
            observed = self.model.get_tensor(self.input['index'])
            self.np.testing.assert_allclose(observed, distance * .01, rtol=1e-6)

    def test_arduino_export_contains_the_reference_model_weights(self):
        archive = Path(__file__).parents[1] / 'model/edge-impulse-arduino-real-v1-float32.zip'
        with zipfile.ZipFile(archive) as bundle:
            name = next(n for n in bundle.namelist()
                        if n.endswith('tflite_learn_1140184_3_compiled.cpp'))
            source = bundle.read(name).decode('utf-8')
        arrays = re.findall(r'float tensor_data\d+\[.*?\]\s*=\s*\{(.*?)\};', source, re.S)
        self.assertEqual(len(arrays), 6, 'Expected three dense weight/bias pairs')
        tensors = [self.model.get_tensor(t['index']).ravel()
                   for t in self.model.get_tensor_details()]
        for array in arrays:
            values = self.np.asarray([float(v) for v in array.split(',') if v.strip()],
                                    dtype=self.np.float32)
            self.assertTrue(any(self.np.array_equal(values, t) for t in tensors),
                            'Arduino weights differ from the desktop reference')


if __name__ == '__main__':
    unittest.main()
