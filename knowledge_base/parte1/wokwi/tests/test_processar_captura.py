"""Protocol-level tests for the Wokwi serial capture parser."""

from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path


PARSER_PATH = Path(__file__).resolve().parents[1] / "processar_captura.py"
SPEC = importlib.util.spec_from_file_location("processar_captura", PARSER_PATH)
assert SPEC is not None and SPEC.loader is not None
processar_captura = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(processar_captura)


def raw_lines(values: list[int]) -> list[str]:
    """Render the exact raw line emitted by the capture sketch."""

    return [f"raw,{value}," for value in values]


class ProcessarCapturaTests(unittest.TestCase):
    def parse_lines(self, lines: list[str]):
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "serial.log"
            source.write_text("\n".join(lines) + "\n", encoding="utf-8")
            return processar_captura.parse_log(source)

    def test_valid_dark_and_light_windows_use_sketch_math(self):
        dark = [3000] * processar_captura.WINDOW_SIZE
        light = [1000] * processar_captura.WINDOW_SIZE
        dark_feature = 1.0 - (sum(dark) / processar_captura.WINDOW_SIZE) / processar_captura.ADC_MAX
        light_feature = 1.0 - (sum(light) / processar_captura.WINDOW_SIZE) / processar_captura.ADC_MAX
        lines = ["tipo,valor,label"]
        lines += raw_lines(dark)
        lines.append(f"feature,{dark_feature:.4f},0")
        lines += raw_lines(light)
        lines.append(f"feature,{light_feature:.4f},1")

        raw_values, processed_rows, ignored_lines = self.parse_lines(lines)

        self.assertEqual(raw_values, dark + light)
        self.assertEqual(
            processed_rows,
            [(f"{dark_feature:.4f}", 0), (f"{light_feature:.4f}", 1)],
        )
        self.assertEqual(ignored_lines, 0)

    def test_incomplete_trailing_raw_window_is_kept_but_not_processed(self):
        dark = [3000] * processar_captura.WINDOW_SIZE
        light = [1000] * processar_captura.WINDOW_SIZE
        lines = ["tipo,valor,label"]
        lines += raw_lines(dark)
        lines.append("feature,0.2674,0")
        lines += raw_lines(light)
        lines.append("feature,0.7558,1")
        lines += raw_lines([2000, 2001, 2002])

        raw_values, processed_rows, _ = self.parse_lines(lines)

        self.assertEqual(len(raw_values), 2 * processar_captura.WINDOW_SIZE + 3)
        self.assertEqual(len(processed_rows), 2)

    def test_rejects_feature_before_ten_raw_samples(self):
        values = [3000] * (processar_captura.WINDOW_SIZE - 1)
        lines = ["tipo,valor,label"] + raw_lines(values) + ["feature,0.2674,0"]

        with self.assertRaisesRegex(ValueError, "esperadas 10"):
            self.parse_lines(lines)

    def test_rejects_extra_raw_sample_before_feature(self):
        values = [3000] * (processar_captura.WINDOW_SIZE + 1)
        lines = ["tipo,valor,label"] + raw_lines(values) + ["feature,0.2674,0"]

        with self.assertRaisesRegex(ValueError, "após 11 leituras cruas"):
            self.parse_lines(lines)

    def test_rejects_complete_window_without_feature(self):
        values = [3000] * processar_captura.WINDOW_SIZE
        lines = ["tipo,valor,label"] + raw_lines(values)

        with self.assertRaisesRegex(ValueError, "não contém janelas processadas"):
            self.parse_lines(lines)

    def test_rejects_feature_that_does_not_match_window_mean(self):
        values = [3000] * processar_captura.WINDOW_SIZE
        lines = ["tipo,valor,label"] + raw_lines(values) + ["feature,0.5000,0"]

        with self.assertRaisesRegex(ValueError, "feature não corresponde"):
            self.parse_lines(lines)

    def test_rejects_label_that_does_not_match_threshold(self):
        values = [3000] * processar_captura.WINDOW_SIZE
        lines = ["tipo,valor,label"] + raw_lines(values) + ["feature,0.2674,1"]

        with self.assertRaisesRegex(ValueError, "label não corresponde"):
            self.parse_lines(lines)


if __name__ == "__main__":
    unittest.main()
