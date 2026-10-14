"""Coleta real e preparo de janelas CSV; origem registrada por sessao."""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import random
import time

LABELS = ('livre', 'aproximando', 'bloqueado')
WINDOW = 10


def parse_line(line):
    if line.startswith('#') or line.startswith('timestamp,'):
        return None
    fields = line.strip().split(',')
    if len(fields) != 3:
        raise ValueError(f'Linha serial invalida: {line!r}')
    timestamp, distance, valid = int(fields[0]), float(fields[1]), int(fields[2])
    if timestamp < 0 or valid not in (0, 1) or not math.isfinite(distance):
        raise ValueError('Valores invalidos')
    if valid and not 2 <= distance <= 400:
        raise ValueError('Distancia fora da faixa')
    return timestamp, distance, valid


def windows(rows):
    buffer = []
    previous = None
    for row in rows:
        timestamp, distance, valid = row
        if previous is not None and timestamp <= previous:
            raise ValueError('Timestamp repetido ou reinicio da placa')
        if not valid or (previous is not None and not 80 <= timestamp - previous <= 120):
            buffer = []
        previous = timestamp
        if not valid:
            continue
        buffer.append(row)
        if len(buffer) == WINDOW:
            yield buffer
            buffer = []


def write_session(path, rows, label, origin):
    path.parent.mkdir(parents=True, exist_ok=True)
    # Nao sobrescrever uma coleta anterior.
    with path.open('x', newline='', encoding='utf-8') as stream:
        writer = csv.writer(stream)
        writer.writerow(['timestamp', 'distance_cm', 'valid'])
        writer.writerows(rows)
    path.with_suffix('.json').write_text(json.dumps({
        'label': label, 'origin': origin, 'measurements': len(rows),
        'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
    }, indent=2), encoding='utf-8')


def prepare(source, destination, allow_simulated=False):
    if destination.exists():
        raise ValueError('Destino ja existe; use uma nova pasta para preservar o dataset')
    sessions = {label: [] for label in LABELS}
    for path in sorted(source.glob('*.csv')):
        meta = json.loads(path.with_suffix('.json').read_text(encoding='utf-8'))
        if meta['origin'] not in ('real', 'synthetic', 'wokwi'):
            raise ValueError('Origem desconhecida')
        if meta['origin'] != 'real' and not allow_simulated:
            raise ValueError('Dados simulados exigem --allow-simulated')
        if meta['sha256'] != hashlib.sha256(path.read_bytes()).hexdigest():
            raise ValueError('CSV alterado depois da coleta')
        if meta['label'] not in LABELS:
            raise ValueError('Classe desconhecida')
        with path.open(encoding='utf-8') as stream:
            rows = [parse_line(line) for line in stream]
        clips = list(windows(row for row in rows if row is not None))
        if not clips:
            raise ValueError(f'Sem janela valida: {path}')
        sessions[meta['label']].append((path, meta, clips))
    for label, items in sessions.items():
        if len(items) < 5:
            raise ValueError(f'{label}: colete ao menos 5 sessoes independentes')
    manifest = []
    for label, items in sessions.items():
        # Split por sessao: janelas vizinhas nunca cruzam treino/teste.
        random.Random(42).shuffle(items)
        test_count = max(1, round(len(items) * 0.2))
        for index, (path, meta, clips) in enumerate(items):
            subset = 'testing' if index < test_count else 'training'
            for number, clip in enumerate(clips):
                target = destination / subset / f'{label}.{path.stem}-{number:04d}.csv'
                target.parent.mkdir(parents=True, exist_ok=True)
                with target.open('x', newline='', encoding='utf-8') as stream:
                    writer = csv.writer(stream)
                    writer.writerow(['timestamp', 'distance_cm'])
                    writer.writerows((i * 100, row[1]) for i, row in enumerate(clip))
                manifest.append({'file': target.relative_to(destination).as_posix(),
                                 'session': path.name, 'label': label, 'origin': meta['origin'],
                                 'measurements': len(clip)})
    (destination / 'manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
    print(f'{len(manifest)} janelas, {len(manifest) * WINDOW} medicoes exportadas')


def synthetic(destination):
    rng = random.Random(2026)
    for label in LABELS:
        for session in range(10):
            rows = []
            for clip in range(20):
                start = rng.uniform(70, 180) if label == 'livre' else rng.uniform(9, 23)
                if label == 'aproximando':
                    start = rng.uniform(40, 100)
                for i in range(WINDOW):
                    value = start - (i * rng.uniform(2, 4) if label == 'aproximando' else 0)
                    rows.append(((clip * WINDOW + i) * 100, round(max(2, value + rng.gauss(0, 1)), 3), 1))
            write_session(destination / f'{label}-s{session:02d}.csv', rows, label, 'synthetic')
    print('SIMULACAO: 6000 medicoes sinteticas; nao sao dados do grupo')


def capture(port, label, seconds, destination):
    import serial
    rows = []
    with serial.Serial(port, 115200, timeout=1) as connection:
        time.sleep(2)
        connection.reset_input_buffer()
        end = time.monotonic() + seconds
        while time.monotonic() < end:
            line = connection.readline().decode('utf-8', errors='strict').strip()
            if not line:
                continue
            row = parse_line(line)
            if row is not None:
                rows.append(row)
    if not rows:
        raise ValueError('Nenhuma leitura recebida')
    write_session(destination, rows, label, 'real')
    print(f'{len(rows)} medicoes gravadas em {destination}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    sim = commands.add_parser('simulate')
    sim.add_argument('destination', type=Path)
    prep = commands.add_parser('prepare')
    prep.add_argument('source', type=Path)
    prep.add_argument('destination', type=Path)
    prep.add_argument('--allow-simulated', action='store_true')
    cap = commands.add_parser('capture')
    cap.add_argument('--port', required=True)
    cap.add_argument('--label', choices=LABELS, required=True)
    cap.add_argument('--seconds', type=int, default=30)
    cap.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.command == 'simulate':
        synthetic(args.destination)
    elif args.command == 'prepare':
        prepare(args.source, args.destination, args.allow_simulated)
    else:
        if args.seconds <= 0:
            parser.error('--seconds deve ser positivo')
        capture(args.port, args.label, args.seconds, args.output)


if __name__ == '__main__':
    main()
