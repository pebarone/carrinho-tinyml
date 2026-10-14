"""Gera artefatos de demonstracao local com dados sinteticos.

Arquitetura da aula 20: Input (1) -> Dense (20, ReLU) -> Dense (10, ReLU)
-> Output (2, softmax). 20 ciclos, learning rate 0.005, Adam.
Os resultados nao sao evidencias nem exportacoes do Edge Impulse.
O script nunca escreve nos arquivos da Parte 1.
"""

from __future__ import annotations

import json
import os
from pathlib import Path

os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "3")
os.environ.setdefault("TF_ENABLE_ONEDNN_OPTS", "0")

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
from sklearn.model_selection import train_test_split

ROOT = Path(__file__).resolve().parents[1]
PARTE2 = ROOT / "parte2"
INFER_DIR = PARTE2 / "inferencia_ldr"

SEED = 19
ADC_MAX = 4095.0
LIMIAR = 0.45
JANELA = 10
R10 = 15000.0
GAMMA = 0.6
RFIX = 10000.0
N_POR_CLASSE = 100
EPOCHS = 20
LR = 0.005
PROJETO = "DEMO_LOCAL_SINTETICA"


def br(valor: float, casas: int = 2) -> str:
    return f"{valor:.{casas}f}".replace(".", ",")


def adc_teorico(lux: float) -> float:
    resistencia = R10 * (10.0 / lux) ** GAMMA
    return ADC_MAX * (RFIX / (resistencia + RFIX))


def amostrar_janela(rng: np.random.Generator, lux_min: float, lux_max: float):
    for _ in range(1000):
        lux = float(np.exp(rng.uniform(np.log(lux_min), np.log(lux_max))))
        base = adc_teorico(lux)
        bruto = np.empty(JANELA, dtype=np.int32)
        for i in range(JANELA):
            flicker = float(rng.normal(0.0, 0.015))
            ruidoso = adc_teorico(lux * (1.0 + flicker))
            ruidoso += float(rng.normal(0.0, 18.0))
            bruto[i] = int(np.clip(np.rint(ruidoso), 0, ADC_MAX))
        media = float(np.clip(bruto.mean(), 0.0, ADC_MAX))
        feature = media / ADC_MAX
        feature_txt = round(feature + 1e-12, 4)
        if feature_txt < LIMIAR - 0.02:
            return bruto, feature_txt, 0, lux, base
        if feature_txt > LIMIAR + 0.02:
            return bruto, feature_txt, 1, lux, base
    raise RuntimeError("não consegui separar as classes em torno do limiar")


def gerar_serie(rng: np.random.Generator):
    blocos = [(0, 0.4, 8.0), (1, 40.0, 800.0)]
    brutos = []
    linhas = []
    detalhes = []
    for classe, lo, hi in blocos:
        for _ in range(N_POR_CLASSE):
            bruto, feature, label, lux, base = amostrar_janela(rng, lo, hi)
            if label != classe:
                continue
            brutos.append(bruto)
            linhas.append((feature, label))
            detalhes.append(
                {
                    "lux": lux,
                    "adc_medio": float(bruto.mean()),
                    "ruido_janela": float(bruto.std()),
                    "feature": feature,
                    "label": label,
                }
            )
        if sum(1 for d in detalhes if d["label"] == classe) < N_POR_CLASSE:
            raise RuntimeError(f"classe {classe} ficou incompleta")
    return brutos, linhas, detalhes


def round_div(acc: int, mult: int, shift: int) -> int:
    numero = int(acc) * int(mult)
    vies = 1 << (shift - 1)
    if numero >= 0:
        return (numero + vies) >> shift
    return -(((-numero) + vies) >> shift)


def escolher_mult(razao: float) -> tuple[int, int]:
    shift = 20
    mult = int(np.round(razao * (1 << shift)))
    while abs(mult) > 2**30 and shift > 8:
        shift -= 1
        mult = int(np.round(razao * (1 << shift)))
    while abs(mult) < 2**14 and shift < 28:
        shift += 1
        mult = int(np.round(razao * (1 << shift)))
    return mult, shift


def quantizar_pesos(matriz: np.ndarray) -> tuple[np.ndarray, float]:
    pico = float(np.max(np.abs(matriz)))
    if pico < 1e-8:
        return np.zeros(matriz.shape, dtype=np.int32), 1.0
    escala = pico / 127.0
    quantizado = np.clip(np.rint(matriz / escala), -127, 127).astype(np.int32)
    return quantizado, escala


def escala_ativacao(valores: np.ndarray) -> float:
    pico = float(np.max(np.abs(valores)))
    if pico < 1e-6:
        pico = 1e-6
    return pico / 127.0


def relu(valores: np.ndarray) -> np.ndarray:
    return np.maximum(valores, 0.0)


def prever_int8(feature: float, pacote: dict) -> tuple[int, np.ndarray]:
    x = int(np.clip(np.rint(np.clip(feature, 0.0, 1.0) * 127.0), 0, 127))
    h1 = np.empty(20, dtype=np.int32)
    for i in range(20):
        acc = int(pacote["b1"][i]) + int(pacote["w1"][i]) * x
        if acc <= 0:
            h1[i] = 0
        else:
            h1[i] = int(np.clip(round_div(acc, pacote["m1"], pacote["s1"]), 0, 127))
    h2 = np.empty(10, dtype=np.int32)
    for i in range(10):
        acc = int(pacote["b2"][i])
        for k in range(20):
            acc += int(pacote["w2"][i, k]) * int(h1[k])
        if acc <= 0:
            h2[i] = 0
        else:
            h2[i] = int(np.clip(round_div(acc, pacote["m2"], pacote["s2"]), 0, 127))
    logits = np.empty(2, dtype=np.int32)
    for i in range(2):
        acc = int(pacote["b3"][i])
        for k in range(10):
            acc += int(pacote["w3"][i, k]) * int(h2[k])
        logits[i] = int(np.clip(round_div(acc, pacote["m3"], pacote["s3"]), -127, 127))
    return (1 if logits[1] > logits[0] else 0), logits


def softmax_ce(logits_q: np.ndarray, escala: float, alvo: int) -> float:
    z = logits_q.astype(np.float64) * escala
    z = z - z.max()
    e = np.exp(z)
    p = e / e.sum()
    return float(-np.log(max(p[alvo], 1e-12)))


def fmt_i32(valores: np.ndarray, por_linha: int = 10) -> str:
    planos = [str(int(v)) for v in np.ravel(valores)]
    linhas = []
    for i in range(0, len(planos), por_linha):
        linhas.append("    " + ", ".join(planos[i : i + por_linha]))
    return ",\n".join(linhas)


def fmt_matriz(matriz: np.ndarray) -> str:
    linhas = []
    for linha in matriz:
        linhas.append("    {" + ", ".join(str(int(v)) for v in linha) + "}")
    return ",\n".join(linhas)


def emitir_model_h(pacote: dict) -> str:
    return f"""#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include <math.h>

/* Demonstracao local com dados sinteticos. NAO e exportacao Edge Impulse.
 * Quantizacao int8 implementada neste script; nao e quantizacao do EI.
 * Input (1) -> Dense 20 ReLU -> Dense 10 ReLU -> logits (2).
 * Classes: 0 escuro, 1 claro.
 * Treino: 20 ciclos, Adam, learning rate 0.005.
 */

#define MODEL_PROJECT "{PROJETO}"
#define MODEL_N_CLASSES 2

static const char *const MODEL_LABELS[MODEL_N_CLASSES] = {{"escuro", "claro"}};

static const int8_t MODEL_W1[20] = {{
{fmt_i32(pacote["w1"])}
}};
static const int32_t MODEL_B1[20] = {{
{fmt_i32(pacote["b1"])}
}};
static const int32_t MODEL_M1 = {int(pacote["m1"])};
static const int MODEL_SHIFT1 = {int(pacote["s1"])};

static const int8_t MODEL_W2[10][20] = {{
{fmt_matriz(pacote["w2"])}
}};
static const int32_t MODEL_B2[10] = {{
{fmt_i32(pacote["b2"])}
}};
static const int32_t MODEL_M2 = {int(pacote["m2"])};
static const int MODEL_SHIFT2 = {int(pacote["s2"])};

static const int8_t MODEL_W3[2][10] = {{
{fmt_matriz(pacote["w3"])}
}};
static const int32_t MODEL_B3[2] = {{
{fmt_i32(pacote["b3"])}
}};
static const int32_t MODEL_M3 = {int(pacote["m3"])};
static const int MODEL_SHIFT3 = {int(pacote["s3"])};
static const float LOGIT_SCALE = {pacote["logit_scale"]:.8e}f;

static inline int32_t model_mul_shift(int64_t acc, int32_t mult, int shift) {{
  int64_t numero = acc * (int64_t)mult;
  int64_t vies = 1LL << (shift - 1);
  if (numero >= 0) {{
    return (int32_t)((numero + vies) >> shift);
  }}
  return (int32_t)(-(((-numero) + vies) >> shift));
}}

static inline int model_classify(float feature, float *score_escuro, float *score_claro) {{
  int32_t x;
  int8_t h1[20];
  int8_t h2[10];
  int8_t logits[2];
  int i;
  int k;
  int classe;

  if (feature < 0.0f) {{
    feature = 0.0f;
  }} else if (feature > 1.0f) {{
    feature = 1.0f;
  }}
  x = (int32_t)(feature * 127.0f + 0.5f);
  if (x > 127) {{
    x = 127;
  }}

  for (i = 0; i < 20; i++) {{
    int64_t acc = (int64_t)MODEL_B1[i] + (int64_t)MODEL_W1[i] * (int64_t)x;
    int32_t y = 0;
    if (acc > 0) {{
      y = model_mul_shift(acc, MODEL_M1, MODEL_SHIFT1);
      if (y < 0) {{
        y = 0;
      }} else if (y > 127) {{
        y = 127;
      }}
    }}
    h1[i] = (int8_t)y;
  }}

  for (i = 0; i < 10; i++) {{
    int64_t acc = (int64_t)MODEL_B2[i];
    int32_t y = 0;
    for (k = 0; k < 20; k++) {{
      acc += (int64_t)MODEL_W2[i][k] * (int64_t)h1[k];
    }}
    if (acc > 0) {{
      y = model_mul_shift(acc, MODEL_M2, MODEL_SHIFT2);
      if (y < 0) {{
        y = 0;
      }} else if (y > 127) {{
        y = 127;
      }}
    }}
    h2[i] = (int8_t)y;
  }}

  for (i = 0; i < 2; i++) {{
    int64_t acc = (int64_t)MODEL_B3[i];
    int32_t y;
    for (k = 0; k < 10; k++) {{
      acc += (int64_t)MODEL_W3[i][k] * (int64_t)h2[k];
    }}
    y = model_mul_shift(acc, MODEL_M3, MODEL_SHIFT3);
    if (y > 127) {{
      y = 127;
    }} else if (y < -127) {{
      y = -127;
    }}
    logits[i] = (int8_t)y;
  }}

  classe = (logits[1] > logits[0]) ? 1 : 0;
  if (score_escuro != 0 || score_claro != 0) {{
    float z0 = (float)logits[0] * LOGIT_SCALE;
    float z1 = (float)logits[1] * LOGIT_SCALE;
    float m = (z0 > z1) ? z0 : z1;
    float e0 = expf(z0 - m);
    float e1 = expf(z1 - m);
    float soma = e0 + e1;
    if (score_escuro != 0) {{
      *score_escuro = e0 / soma;
    }}
    if (score_claro != 0) {{
      *score_claro = e1 / soma;
    }}
  }}
  return classe;
}}

#endif
"""


def desenhar(caminho: Path, acuracia: float, loss: float, matriz: np.ndarray) -> None:
    fig = plt.figure(figsize=(11.2, 7.0), dpi=140, facecolor="#f7f6f3")
    ax = fig.add_axes((0, 0, 1, 1))
    ax.set_xlim(0, 112)
    ax.set_ylim(0, 70)
    ax.axis("off")
    ax.add_patch(plt.Rectangle((2.2, 2.2), 107.6, 65.6, facecolor="white", edgecolor="#d9d4cc", linewidth=1.2))

    ax.add_patch(plt.Rectangle((3.2, 60.7), 105.6, 5.5, facecolor="#b91c1c", edgecolor="none"))
    ax.text(56.0, 63.45, "DEMONSTRACAO LOCAL SINTETICA - NAO E EDGE IMPULSE",
            fontsize=13, fontweight="bold", color="white", ha="center", va="center")
    ax.text(6.2, 55.2, PROJETO, fontsize=20, fontweight="bold", color="#1c1917", ha="left", va="center")
    ax.add_patch(plt.Rectangle((78.5, 52.2), 26.5, 5.6, facecolor="#57534e", edgecolor="none"))
    ax.text(91.75, 55.0, "Quantizacao local int8", fontsize=10, color="white", ha="center", va="center")

    ax.text(6.2, 48.6, "Diagnostico local - conjunto sintetico de validacao", fontsize=12, color="#57534e", ha="left", va="center")

    ax.text(6.2, 40.5, "ACCURACY", fontsize=11, color="#78716c", ha="left", va="center")
    ax.text(6.2, 33.6, f"{acuracia * 100:.1f}%", fontsize=36, fontweight="bold", color="#1c1917", ha="left", va="center")
    ax.text(6.2, 24.2, "LOSS", fontsize=11, color="#78716c", ha="left", va="center")
    ax.text(6.2, 18.4, f"{loss:.4f}", fontsize=28, fontweight="bold", color="#1c1917", ha="left", va="center")
    ax.text(
        6.2,
        8.6,
        "Input (1) -> Dense (20) -> Dense (10) -> Output (2)\n20 ciclos de treino; learning rate 0.005",
        fontsize=9,
        color="#78716c",
        ha="left",
        va="center",
        linespacing=1.45,
    )

    ax.text(62.0, 48.4, "Matriz de confusao local", fontsize=14, fontweight="bold", color="#1c1917", ha="left", va="center")
    ax.text(62.0, 45.2, "linhas = classe real    colunas = classe prevista", fontsize=8.5, color="#78716c", ha="left", va="center")

    nomes = ["0  escuro", "1  claro"]
    x0, y0, lado = 70.5, 12.2, 13.0
    ax.text(x0 + (lado - 0.7) * 0.5, 41.0, "0", fontsize=11, ha="center", va="center", color="#44403c")
    ax.text(x0 + lado + (lado - 0.7) * 0.5, 41.0, "1", fontsize=11, ha="center", va="center", color="#44403c")
    total = matriz.sum()
    for i in range(2):
        ax.text(x0 - 1.2, y0 + lado * (1.5 - i), nomes[i], fontsize=10, ha="right", va="center", color="#44403c")
        for j in range(2):
            valor = int(matriz[i, j])
            pct = 100.0 * valor / max(total and matriz[i].sum(), 1)
            cor = "#1f7a4d" if i == j else "#f3f1ec"
            tinta = "white" if i == j else "#1c1917"
            ax.add_patch(plt.Rectangle((x0 + j * lado, y0 + (1 - i) * lado), lado - 0.7, lado - 0.7, facecolor=cor, edgecolor="none"))
            ax.text(
                x0 + j * lado + (lado - 0.7) / 2,
                y0 + (1 - i) * lado + (lado - 0.7) / 2 + 1.1,
                f"{pct:.0f}%",
                fontsize=16,
                fontweight="bold",
                color=tinta,
                ha="center",
                va="center",
            )
            ax.text(
                x0 + j * lado + (lado - 0.7) / 2,
                y0 + (1 - i) * lado + (lado - 0.7) / 2 - 2.4,
                f"n = {valor}",
                fontsize=8,
                color=tinta,
                ha="center",
                va="center",
            )

    fig.savefig(caminho, dpi=140)
    plt.close(fig)


def main() -> None:
    rng = np.random.default_rng(SEED)
    tf.random.set_seed(SEED)
    np.random.seed(SEED)

    brutos, linhas, detalhes = gerar_serie(rng)
    PARTE2.mkdir(parents=True, exist_ok=True)
    INFER_DIR.mkdir(parents=True, exist_ok=True)

    x = np.array([f for f, _ in linhas], dtype=np.float32).reshape(-1, 1)
    y = np.array([lbl for _, lbl in linhas], dtype=np.int32)
    x_tr, x_va, y_tr, y_va = train_test_split(x, y, test_size=0.2, random_state=SEED, stratify=y)

    modelo = tf.keras.Sequential(
        [
            tf.keras.layers.Input(shape=(1,)),
            tf.keras.layers.Dense(20, activation="relu"),
            tf.keras.layers.Dense(10, activation="relu"),
            tf.keras.layers.Dense(2, activation="softmax"),
        ]
    )
    modelo.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=LR),
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"],
    )
    historico = modelo.fit(x_tr, y_tr, validation_data=(x_va, y_va), epochs=EPOCHS, batch_size=32, verbose=0)
    loss_f, acc_f = modelo.evaluate(x_va, y_va, verbose=0)

    k1, b1 = modelo.layers[0].get_weights()
    k2, b2 = modelo.layers[1].get_weights()
    k3, b3 = modelo.layers[2].get_weights()
    # kernel Keras é (entrada, saída). O header usa (saída, entrada).
    w1 = np.asarray(k1.T, dtype=np.float64).reshape(20)
    w2 = np.asarray(k2.T, dtype=np.float64)
    w3 = np.asarray(k3.T, dtype=np.float64)

    h1 = relu(x_tr @ k1 + b1)
    h2 = relu(h1 @ k2 + b2)
    logits = h2 @ k3 + b3

    s_x = 1.0 / 127.0
    w1_q, s_w1 = quantizar_pesos(w1)
    w2_q, s_w2 = quantizar_pesos(w2)
    w3_q, s_w3 = quantizar_pesos(w3)
    s_h1 = escala_ativacao(h1)
    s_h2 = escala_ativacao(h2)
    s_log = escala_ativacao(logits)

    def vies(b, s_in, s_w):
        return np.rint(b / (s_in * s_w)).astype(np.int64)

    b1_q = vies(b1, s_x, s_w1)
    b2_q = vies(b2, s_h1, s_w2)
    b3_q = vies(b3, s_h2, s_w3)
    m1, sh1 = escolher_mult((s_x * s_w1) / s_h1)
    m2, sh2 = escolher_mult((s_h1 * s_w2) / s_h2)
    m3, sh3 = escolher_mult((s_h2 * s_w3) / s_log)

    for nome, arr in (("b1", b1_q), ("b2", b2_q), ("b3", b3_q)):
        if np.max(np.abs(arr)) > 2**31 - 1:
            raise RuntimeError(f"viés {nome} não cabe em int32")

    pacote = {
        "w1": w1_q,
        "b1": b1_q,
        "m1": m1,
        "s1": sh1,
        "w2": w2_q,
        "b2": b2_q,
        "m2": m2,
        "s2": sh2,
        "w3": w3_q,
        "b3": b3_q,
        "m3": m3,
        "s3": sh3,
        "logit_scale": s_log,
    }

    preds = []
    perdas = []
    for feature, alvo in zip(x_va.ravel(), y_va):
        classe, logits_q = prever_int8(float(feature), pacote)
        preds.append(classe)
        perdas.append(softmax_ce(logits_q, s_log, int(alvo)))
    preds_np = np.array(preds)
    matriz = np.zeros((2, 2), dtype=np.int32)
    for real, previsto in zip(y_va, preds_np):
        matriz[int(real), int(previsto)] += 1
    acc_i = float((preds_np == y_va).mean())
    loss_i = float(np.mean(perdas))

    concordancia = []
    probs = modelo.predict(x_va, verbose=0)
    for feature, prob in zip(x_va.ravel(), probs):
        classe, _ = prever_int8(float(feature), pacote)
        concordancia.append(classe == int(np.argmax(prob)))
    acordo = float(np.mean(concordancia))
    if acc_i < 0.95:
        raise RuntimeError(f"acurácia int8 baixa: {acc_i:.3f}")

    cabecalho = emitir_model_h(pacote)
    (INFER_DIR / "modelo_local_demonstracao.h").write_text(cabecalho, encoding="utf-8")
    desenhar(PARTE2 / "diagnostico_treino_local_sintetico.png", acc_i, loss_i, matriz)

    adc_all = np.concatenate(brutos)
    feat = np.array([d["feature"] for d in detalhes])
    labels = np.array([d["label"] for d in detalhes])
    ruido = np.array([d["ruido_janela"] for d in detalhes])
    limiar_lux = float(next(lux for lux in np.linspace(5.0, 40.0, 3501) if adc_teorico(float(lux)) / ADC_MAX >= LIMIAR))
    luxes = [1, 5, 10, round(limiar_lux, 1), 50, 100, 300, 1000]
    tabela = []
    for lux in luxes:
        adc = adc_teorico(lux)
        tabela.append({"lux": lux, "adc": round(adc, 1), "feature": round(adc / ADC_MAX, 4)})

    resumo = {
        "status": "demonstracao local sintetica; nao e resultado do Edge Impulse",
        "fonte_dados": "dados sinteticos deterministas gerados neste script; sem captura de hardware ou Wokwi",
        "modelo": "Keras float32 com quantizador int8 local implementado neste script",
        "exportacao_edge_impulse": {
            "link_publico": None,
            "print_classifier": None,
            "biblioteca_arduino": None,
        },
        "n_janelas": len(linhas),
        "n_brutas": int(adc_all.size),
        "adc_min": int(adc_all.min()),
        "adc_max": int(adc_all.max()),
        "adc_medio": float(adc_all.mean()),
        "ruido_medio_dentro_da_janela": float(ruido.mean()),
        "feature_min": float(feat.min()),
        "feature_max": float(feat.max()),
        "feature_escuro_min": float(feat[labels == 0].min()),
        "feature_escuro_max": float(feat[labels == 0].max()),
        "feature_escuro_media": float(feat[labels == 0].mean()),
        "feature_escuro_std": float(feat[labels == 0].std()),
        "feature_claro_min": float(feat[labels == 1].min()),
        "feature_claro_max": float(feat[labels == 1].max()),
        "feature_claro_media": float(feat[labels == 1].mean()),
        "feature_claro_std": float(feat[labels == 1].std()),
        "adc_escuro_min": int(min(d["adc_medio"] for d in detalhes if d["label"] == 0)),
        "adc_escuro_max": int(max(d["adc_medio"] for d in detalhes if d["label"] == 0)),
        "adc_claro_min": int(min(d["adc_medio"] for d in detalhes if d["label"] == 1)),
        "adc_claro_max": int(max(d["adc_medio"] for d in detalhes if d["label"] == 1)),
        "modelo_local_float32_val_accuracy": float(acc_f),
        "modelo_local_float32_val_loss": float(loss_f),
        "modelo_local_quantizado_int8_val_accuracy": acc_i,
        "modelo_local_quantizado_int8_val_loss": loss_i,
        "modelo_local_int8_vs_float_agreement": acordo,
        "modelo_local_confusion_matrix_val": matriz.tolist(),
        "val_size": int(y_va.size),
        "epochs": EPOCHS,
        "learning_rate": LR,
        "final_train_loss": float(historico.history["loss"][-1]),
        "tabela_lux_adc_estimada": tabela,
        "nota_tabela_lux_adc": "estimativa didatica idealizada; nao e calibracao do ADC nem medicao do sensor",
        "limiar_lux_estimado": limiar_lux,
        "limiar_feature": LIMIAR,
    }
    (PARTE2 / "metricas_demo_local.json").write_text(json.dumps(resumo, indent=2), encoding="utf-8")
    print(json.dumps({k: resumo[k] for k in (
        "adc_min", "adc_max", "ruido_medio_dentro_da_janela",
        "feature_escuro_min", "feature_escuro_max", "feature_escuro_media", "feature_escuro_std",
        "feature_claro_min", "feature_claro_max", "feature_claro_media", "feature_claro_std",
        "adc_escuro_min", "adc_escuro_max", "adc_claro_min", "adc_claro_max",
        "modelo_local_float32_val_accuracy", "modelo_local_float32_val_loss",
        "modelo_local_quantizado_int8_val_accuracy", "modelo_local_quantizado_int8_val_loss",
        "modelo_local_int8_vs_float_agreement", "modelo_local_confusion_matrix_val", "val_size",
        "tabela_lux_adc_estimada",
    )}, indent=2))


if __name__ == "__main__":
    main()
