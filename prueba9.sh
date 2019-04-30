#!/bin/bash
echo Vamos a executar en 9 nodos:
echo 80 procesos de anulacions
echo 50 procesos de pagos
echo 30 procesos de pre-reservas
echo 25 procesos de gradas
echo 25 procesos de eventos
echo
echo Ejecutando...
echo
sleep 1
./proyectoConCarga 1 9 80 50 30 25 25 &
./proyectoConCarga 2 9 80 50 30 25 25 &
./proyectoConCarga 3 9 80 50 30 25 25 &
./proyectoConCarga 4 9 80 50 30 25 25 &
./proyectoConCarga 5 9 80 50 30 25 25 &
./proyectoConCarga 6 9 80 50 30 25 25 &
./proyectoConCarga 7 9 80 50 30 25 25 &
./proyectoConCarga 8 9 80 50 30 25 25 &
./proyectoConCarga 9 9 80 50 30 25 25
