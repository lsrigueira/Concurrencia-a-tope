#!/bin/bash
echo Vamos a executar en 4 nodos:
echo 80 procesos de anulacions
echo 50 procesos de pagos
echo 30 procesos de pre-reservas
echo 25 procesos de gradas
echo 25 procesos de eventos
echo
echo Ejecutando...
echo
sleep 1
./proyectoConCarga 1 4 80 50 30 25 25 &
./proyectoConCarga 2 4 80 50 30 25 25 &
./proyectoConCarga 3 4 80 50 30 25 25 &
./proyectoConCarga 4 4 80 50 30 25 25
