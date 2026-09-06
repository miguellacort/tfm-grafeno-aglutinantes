# tfm-grafeno-aglutinantes

Código empleado en el Trabajo Fin de Máster sobre la mejora de la adhesión de
recubrimientos de grafeno a sustratos metálicos mediante aglutinantes. Reúne los
scripts de procesado de datos de las distintas técnicas de caracterización y el
firmware del sistema de deposición motorizado.

## Contenido
- **adherencia.py** — Estima el porcentaje de material retenido tras el ensayo de
  adherencia comparando las imágenes de antes (_A) y después (_D) mediante
  umbralización por brillo y análisis de textura local.
- **sketch_AccelStepper2.ino** — Firmware (Arduino) del carro de deposición
  motorizado: control de un motor paso a paso con AccelStepper, interfaz de
  velocidad y distancia mediante encoder y pantalla LCD, y parada de emergencia.
  
## Autor
Miguel — Universidad de Córdoba, 2026
