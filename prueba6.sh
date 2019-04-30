#!/bin/bash
echo Vamos a executar en 6 nodos:
echo 80 procesos de anulacions
echo 50 procesos de pagos
echo 30 procesos de pre-reservas
echo 25 procesos de gradas
echo 25 procesos de eventos
echo
echo Ejecutando...
echo
sleep 1
./proyectoConCarga 1 6 80 50 30 25 25 &
./proyectoConCarga 2 6 80 50 30 25 25 &
./proyectoConCarga 3 6 80 50 30 25 25 &
./proyectoConCarga 4 6 80 50 30 25 25 &
./proyectoConCarga 5 6 80 50 30 25 25 &
./proyectoConCarga 6 6 80 50 30 25 25 
