#!/bin/bash 

if [ -z "$1" ]; then
    echo "Esperando argumento (nome do arquivo sem extensão). EX: $0 probe"
    echo "Ou digite './driver.sh remove' para apagar todos os arquivos make."
    exit 1
fi

arquivo=$1

if [ "$arquivo" = "remove" ]; then
    sudo rmmod cp210x 2> /dev/null
    sudo rmmod probe 2> /dev/null
    sudo rmmod serial 2> /dev/null
    sudo rmmod serial_write 2> /dev/null
    make clean
    sudo dmesg
else 
    #remover os 2> e &> /dev/null para voltar a ver a saída no terminal
    sudo rmmod cp210x 2> /dev/null
    sudo rmmod "$arquivo".ko 2> /dev/null
    make clean 2> /dev/null
    make &> /dev/null 
    echo "Aguardando a execução do comando make..."
    #sudo modprobe cp210x
    sudo insmod "$arquivo".ko
    echo "Arquivos make criados!!!"
    sudo dmesg

    echo "Fim da execução do driver.sh! (^W^)"
fi

