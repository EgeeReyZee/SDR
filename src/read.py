import os
import matplotlib.pyplot as plt
import numpy as np

# Получаем абсолютный путь к папке, где лежит read.py (~/pluto_project/src)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Поднимаемся на уровень выше в корень проекта и заходим в build
name = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "build", "Rxdata.pcm"))

print("Читаю:", name)

if not os.path.exists(name):
    raise SystemExit(f"Файл не найден: {name}")

# Считываем все 16-битные знаковые числа мгновенно в память
raw = np.fromfile(name, dtype="<i2")

if raw.size == 0:
    raise SystemExit(f"Файл {name} пуст")

# I (real) — четные отсчеты, Q (imag) — нечетные
real = raw[0::2].astype(np.float64)
imag = raw[1::2].astype(np.float64)

# Выравниваем массивы по минимальной длине на случай неполной пары
n = min(len(real), len(imag))
real = real[:n]
imag = imag[:n]

# === ABS: модуль комплексного сигнала ===
# |I + jQ| = sqrt(I^2 + Q^2)
amplitude_abs = np.sqrt(real**2 + imag**2)

# Ось времени / индексов сэмплов
count = np.arange(n)

# Ограничим зум для отрисовки, чтобы график не зависал
ZOOM = 3200000
m = min(ZOOM, n)

# === ГРАФИК 1: I и Q как раньше + модуль ===
plt.figure(figsize=(12, 5))
plt.plot(count[:m], real[:m], color="blue", label="I (Real)", linewidth=0.8)
plt.plot(count[:m], imag[:m], color="red", label="Q (Imag)", linewidth=0.8)
plt.plot(count[:m], amplitude_abs[:m], color="green", label="|I+jQ| (abs)", linewidth=0.8)
plt.title(f"Первые {m} сэмплов из {n}")
plt.xlabel("Сэмплы")
plt.ylabel("Амплитуда (int16)")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()

# === ГРАФИК 2: Только модуль (огибающая) ===
plt.figure(figsize=(12, 4))
plt.plot(count[:m], amplitude_abs[:m], color="green", linewidth=0.8)
plt.title(f"Огибающая (абсолютное значение) — первые {m} сэмплов из {n}")
plt.xlabel("Сэмплы")
plt.ylabel("|I+jQ|")
plt.grid(True, alpha=0.3)
plt.tight_layout()

plt.show()