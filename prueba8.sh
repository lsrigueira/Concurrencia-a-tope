#!/bin/bash
echo Vamos a executar en 8 nodos:
echo 80 procesos de anulacions
echo 50 procesos de pagos
echo 30 procesos de pre-reservas
echo 25 procesos de gradas
echo 25 procesos de eventos
echo
echo Ejecutando...
echo
sleep 1
./proyectoConCarga 1 8 80 50 30 25 25 &
./proyectoConCarga 2 8 80 50 30 25 25 &
./proyectoConCarga 3 8 80 50 30 25 25 &
./proyectoConCarga 4 8 80 50 30 25 25 &
./proyectoConCarga 5 8 80 50 30 25 25 &
./proyectoConCarga 6 8 80 50 30 25 25 &
./proyectoConCarga 7 8 80 50 30 25 25 &
./proyectoConCarga 8 8 80 50 30 25 25
