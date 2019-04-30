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
./proyectoConCarga 1 10 80 50 30 25 25 &
./proyectoConCarga 2 10 80 50 30 25 25 &
./proyectoConCarga 3 10 80 50 30 25 25 &
./proyectoConCarga 4 10 80 50 30 25 25 &
./proyectoConCarga 5 10 80 50 30 25 25 &
./proyectoConCarga 6 10 80 50 30 25 25 &
./proyectoConCarga 7 10 80 50 30 25 25 &
./proyectoConCarga 8 10 80 50 30 25 25 &
./proyectoConCarga 9 10 80 50 30 25 25 &
./proyectoConCarga 10 10 80 50 30 25 25
