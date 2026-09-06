import cv2
import glob


def varianza(gris):
    h = len(gris)
    w = len(gris[0])
    var = [[0.0] * w for _ in range(h)]
    for i in range(1, h - 1):
        for j in range(1, w - 1):
            suma = 0
            for a in range(-1, 2):
                for b in range(-1, 2):
                    suma += gris[i + a][j + b]
            m = suma / 9.0
            v = 0.0
            for a in range(-1, 2):
                for b in range(-1, 2):
                    d = gris[i + a][j + b] - m
                    v += d * d
            var[i][j] = v / 9.0
    return var


def calcula_umbral(gris):
    h = len(gris)
    w = len(gris[0])
    hist = [0] * 256
    for i in range(h):
        for j in range(w):
            hist[gris[i][j]] += 1
    total = h * w
    suma_total = 0
    for t in range(256):
        suma_total += t * hist[t]
    mejor_t = 0
    mejor_valor = 0.0
    peso_bajo = 0
    suma_bajo = 0
    for t in range(256):
        peso_bajo += hist[t]
        if peso_bajo == 0:
            continue
        peso_alto = total - peso_bajo
        if peso_alto == 0:
            break
        suma_bajo += t * hist[t]
        media_bajo = suma_bajo / peso_bajo
        media_alto = (suma_total - suma_bajo) / peso_alto
        entre = peso_bajo * peso_alto * (media_bajo - media_alto) ** 2
        if entre > mejor_valor:
            mejor_valor = entre
            mejor_t = t
    return mejor_t


def fraccion(gris, var, umbral_brillo, umbral_textura):
    h = len(gris)
    w = len(gris[0])
    expuesto = 0.0
    total = 0
    for i in range(h):
        for j in range(w):
            total += 1
            if gris[i][j] > umbral_brillo:
                if var[i][j] < umbral_textura:
                    expuesto += 1.0
                else:
                    expuesto += 0.5
    return expuesto / total


antes = ''
despues = ''
for f in glob.glob('*'):
    n = f.lower()
    if n.endswith(('.jpg', '.jpeg', '.png', '.bmp', '.tif', '.tiff')):
        base = f.rsplit('.', 1)[0]
        if base.endswith('_A'):
            antes = f
        elif base.endswith('_D'):
            despues = f

if antes == '' or despues == '':
    raise SystemExit('Faltan las imagenes _A o _D')

gris_antes = cv2.cvtColor(cv2.imread(antes), cv2.COLOR_BGR2GRAY).tolist()
gris_despues = cv2.cvtColor(cv2.imread(despues), cv2.COLOR_BGR2GRAY).tolist()

var_antes = varianza(gris_antes)
var_despues = varianza(gris_despues)

h = len(gris_antes)
w = len(gris_antes[0])

umbral_brillo = calcula_umbral(gris_antes)

suma_v = 0.0
for i in range(h):
    for j in range(w):
        suma_v += var_antes[i][j]
umbral_textura = suma_v / (h * w)

exp_antes = fraccion(gris_antes, var_antes, umbral_brillo, umbral_textura)
exp_despues = fraccion(gris_despues, var_despues, umbral_brillo, umbral_textura)

pct = (exp_despues - exp_antes) / (1 - exp_antes) * 100
if pct < 0:
    pct = 0
adherencia = 100 - pct

pieza = antes.rsplit('.', 1)[0][:-2]

salida = open(pieza + ' - adherencia.txt', 'w')
salida.write('Pieza: ' + pieza + '\n')
salida.write('Adherencia: ' + str(round(adherencia, 1)) + ' %\n')
salida.close()

print(pieza, round(adherencia, 1), '%')
