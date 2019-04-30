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
./proyectoSinCarga 1 4 80 50 30 25 25 &
./proyectoSinCarga 2 4 80 50 30 25 25 &
./proyectoSinCarga 3 4 80 50 30 25 25 &
./proyectoSinCarga 4 4 80 50 30 25 25
