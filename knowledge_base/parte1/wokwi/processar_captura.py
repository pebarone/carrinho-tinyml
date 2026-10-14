#!/usr/bin/env python3
"""Validate a captured Wokwi serial log and split it into two CSV files."""

from __future__ import annotations

import argparse
import csv
import math
import os
import tempfile
from collections import Counter
from pathlib import Path


PARTE1 = Path(__file__).resolve().parents[1]
RAW_HEADER = ["adc"]
DATA_HEADER = ["feature", "label"]
MIXED_HEADER = ["kind", "index", "value", "label"]
SIMPLE_HEADER = ["tipo", "valor", "label"]
ADC_MAX = 4095
WINDOW_SIZE = 10
THRESHOLD = 0.45


def parse_log(source: Path) -> tuple[list[int], list[tuple[str, int]], int]:
    raw_values: list[int] = []
    processed_rows: list[tuple[str, int]] = []
    window: list[int] = []
    ignored_lines = 0
    last_raw_index: int | None = None
    last_window_index: int | None = None

    with source.open("r", encoding="utf-8-sig", newline="") as stream:
        for line_number, line in enumerate(stream, start=1):
            try:
                fields = next(csv.reader([line], strict=True))
            except (csv.Error, StopIteration):
                ignored_lines += 1
                continue

            if not fields or fields[0] not in {"raw", "processed", "feature", "kind", "tipo"}:
                ignored_lines += 1
                continue
            if fields in (MIXED_HEADER, SIMPLE_HEADER):
                continue

            if fields[0] == "raw":
                if len(fields) == 3 and fields[2] == "":
                    raw_text = fields[1]
                    sample_index = last_raw_index + 1 if last_raw_index is not None else 0
                elif len(fields) == 4 and fields[3] == "":
                    raw_text = fields[2]
                    try:
                        sample_index = int(fields[1])
                    except ValueError as error:
                        raise ValueError(f"linha {line_number}: índice raw não inteiro") from error
                else:
                    raise ValueError(f"linha {line_number}: formato raw inválido")
                try:
                    adc = int(raw_text)
                except ValueError as error:
                    raise ValueError(f"linha {line_number}: ADC não inteiro") from error
                if sample_index < 0 or not 0 <= adc <= ADC_MAX:
                    raise ValueError(f"linha {line_number}: índice ou ADC fora da faixa")
                if last_raw_index is not None and sample_index != last_raw_index + 1:
                    raise ValueError(f"linha {line_number}: índice raw repetido ou com lacuna")
                last_raw_index = sample_index
                raw_values.append(adc)
                window.append(adc)
                continue

            if fields[0] in {"processed", "feature"}:
                if fields[0] == "feature" and len(fields) == 3:
                    try:
                        window_index = last_window_index + 1 if last_window_index is not None else 0
                        feature_value = float(fields[1])
                        label = int(fields[2])
                    except ValueError as error:
                        raise ValueError(f"linha {line_number}: feature/label inválido") from error
                    feature_text = fields[1]
                elif fields[0] == "processed" and len(fields) == 4:
                    try:
                        window_index = int(fields[1])
                        feature_value = float(fields[2])
                        label = int(fields[3])
                    except ValueError as error:
                        raise ValueError(f"linha {line_number}: janela/feature/label inválido") from error
                    feature_text = fields[2]
                else:
                    raise ValueError(f"linha {line_number}: formato processed inválido")

                if window_index < 0 or not math.isfinite(feature_value):
                    raise ValueError(f"linha {line_number}: janela ou feature inválido")
                if not 0.0 <= feature_value <= 1.0 or label not in (0, 1):
                    raise ValueError(f"linha {line_number}: feature/label fora da faixa")
                if last_window_index is not None and window_index != last_window_index + 1:
                    raise ValueError(f"linha {line_number}: índice de janela repetido ou com lacuna")
                if len(window) != WINDOW_SIZE:
                    raise ValueError(
                        f"linha {line_number}: janela processada após {len(window)} leituras cruas; "
                        f"esperadas {WINDOW_SIZE}"
                    )

                mean = sum(window) / WINDOW_SIZE
                expected_feature = 1.0 - (mean / ADC_MAX)
                expected_label = 0 if expected_feature < THRESHOLD else 1
                if abs(feature_value - expected_feature) > 0.000051:
                    raise ValueError(f"linha {line_number}: feature não corresponde à média das 10 leituras")
                if label != expected_label:
                    raise ValueError(f"linha {line_number}: label não corresponde ao limiar {THRESHOLD}")

                processed_rows.append((feature_text, label))
                last_window_index = window_index
                window.clear()

    if not raw_values:
        raise ValueError("o log não contém leituras raw reconhecíveis")
    if not processed_rows:
        raise ValueError("o log não contém janelas processadas reconhecíveis")
    if len(window) >= WINDOW_SIZE:
        raise ValueError("faltam linhas processed para um ou mais grupos completos de 10 leituras")
    counts = Counter(label for _, label in processed_rows)
    if not counts[0] or not counts[1]:
        raise ValueError("a captura precisa conter as duas classes; obtenha leituras abaixo e acima do limiar")

    return raw_values, processed_rows, ignored_lines


def write_csv(path: Path, header: list[str], rows: list[list[str | int]]) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="", delete=False, dir=path.parent, suffix=".tmp"
    )
    temp_path = Path(handle.name)
    try:
        with handle:
            writer = csv.writer(handle)
            writer.writerow(header)
            writer.writerows(rows)
        return temp_path
    except Exception:
        temp_path.unlink(missing_ok=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Valida o log misto do sketch captura_wokwi.ino e grava leituras_brutas.csv/dados.csv."
    )
    parser.add_argument("log_serial", type=Path, help="arquivo de texto copiado do Monitor Serial do Wokwi")
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=PARTE1,
        help="pasta dos dois CSVs (deve ficar dentro de deliverables/parte1)",
    )
    args = parser.parse_args()

    source = args.log_serial.resolve()
    output_dir = args.output_dir.resolve()
    if not source.is_file():
        parser.error(f"arquivo de captura não encontrado: {source}")
    if not output_dir.is_relative_to(PARTE1):
        parser.error(f"a pasta de saída precisa estar dentro de {PARTE1}")

    try:
        raw_values, processed_rows, ignored_lines = parse_log(source)
        output_dir.mkdir(parents=True, exist_ok=True)
        raw_path = output_dir / "leituras_brutas.csv"
        data_path = output_dir / "dados.csv"
        raw_temp = write_csv(raw_path, RAW_HEADER, [[value] for value in raw_values])
        data_temp = write_csv(
            data_path,
            DATA_HEADER,
            [[feature, label] for feature, label in processed_rows],
        )
        try:
            os.replace(raw_temp, raw_path)
            os.replace(data_temp, data_path)
        finally:
            raw_temp.unlink(missing_ok=True)
            data_temp.unlink(missing_ok=True)
    except (OSError, ValueError) as error:
        parser.error(str(error))

    classes = Counter(label for _, label in processed_rows)
    print(f"Leituras raw validadas: {len(raw_values)}")
    print(f"Janelas processadas validadas: {len(processed_rows)}")
    print(f"Rótulos: classe 0={classes[0]}, classe 1={classes[1]}")
    print(f"Linhas não pertencentes ao protocolo ignoradas: {ignored_lines}")
    if raw_values and len(raw_values) % WINDOW_SIZE:
        print(f"Aviso: {len(raw_values) % WINDOW_SIZE} leitura(s) final(is) sem janela completa; mantidas apenas no CSV bruto.")
    print(f"Saída bruta: {raw_path}")
    print(f"Saída processada: {data_path}")
    print("Origem: saída serial capturada do Wokwi; não é medição física.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
