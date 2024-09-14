

# DevTITANS 06 - HandsOn Linux - Equipe 05

Bem-vindo ao repositório da Equipe 05 do HandsON de Linux do DevTITANS! Este projeto contém um firmware para o ESP32 escrito em formato Arduino `.ino`, bem como um driver do kernel Linux escrito em C. O objetivo é demonstrar como criar uma solução completa de hardware e software que integra um dispositivo ESP32 com um sistema Linux.

## Tabela de Conteúdos

- [Contribuidores](#contribuidores)
- [Introdução](#introdução)
- [Recursos](#recursos)
- [Requisitos](#requisitos)
- [Configuração de Hardware](#configuração-de-hardware)
- [Instalação](#instalação)
- [Uso](#uso)
- [Contato](#contato)

## Contribuidores

<img src="https://github.com/user-attachments/assets/b31e7885-82ee-43ac-b5c5-74b80e022d08" width="180" >
<img src="https://github.com/user-attachments/assets/61db94ca-ef4c-4f0e-821b-8d99d35665ff" width="180" >
<img src="https://github.com/user-attachments/assets/d27553ed-ded5-425b-9fbe-d5c1b24df727" width="180" >
<img src="https://github.com/user-attachments/assets/0746677b-b19d-495c-8c16-88db0deebca8" width="180" >
<img src="https://github.com/user-attachments/assets/9733a478-7589-4267-9468-74e481a26af6" width="180" >
<img src="https://github.com/DevTITANS05/Hands-On-Linux-fork-/assets/21023906/85e61f3e-476c-47a4-82d5-4054e856c67b" width="180" >

- **Emidio Aranha:** Desenvolvedor do Firmware e Mantenedor do Projeto
- **Alcir Castro:** Desenvolvedor do Firmware
- **Joáo Pedro:** Desenvolvedor do Driver Linux
- **Luiz Eduardo:** Desenvolvedor do Driver Linux
- **Karen Alves:** Desenvolvedor do Firmware e Escritor da Documentação

## Introdução

Este projeto serve como um exemplo para desenvolvedores interessados em construir e integrar soluções de hardware personalizadas com sistemas Linux. Inclui os seguintes componentes:
- Firmware para o microcontrolador ESP32 para lidar com operações específicas do dispositivo.
- Um driver do kernel Linux que se comunica com o dispositivo ESP32, permitindo troca de dados e controle.

## Recursos

- **Firmware ESP32:**
  - Aquisição básica de dados de sensores.
  - Comunicação via Serial com o driver Linux.
  
- **Driver do Kernel Linux:**
  - Rotinas de inicialização e limpeza.
  - Operações de arquivo de dispositivo (`GET_LED`, `SET_LED`, `GET_LDR`).
  - Comunicação com o ESP32 via Serial.

## Requisitos

- **Hardware:**
  - Placa de Desenvolvimento ESP32
  - Máquina Linux
  - Protoboard e Cabos Jumper
  - Sensor LDR
  
- **Software:**
  - Arduino IDE
  - Kernel Linux 4.0 ou superior
  - GCC 4.8 ou superior
  - Make 3.81 ou superior

## Configuração de Hardware

1. **Conecte o ESP32 à sua Máquina Linux:**
    - Use um cabo USB.
    - Conecte os sensores ao ESP32 conforme especificado no firmware.

2. **Garanta a alimentação e conexões adequadas:**
    - Use um protoboard e cabos jumper para montar o circuito.
    - Consulte o diagrama esquemático fornecido no diretório `esp32` para conexões detalhadas.

## Instalação

### Firmware ESP32

1. **Abra o Arduino IDE e carregue o firmware:**
    ```sh
    Arquivo -> Abrir -> Selecione `smartlamp.ino`
    ```

2. **Configure a Placa e a Porta:**
    ```sh
    Ferramentas -> Placa -> Node32s
    Ferramentas -> Porta -> Selecione a porta apropriada
    ```

3. **Carregue o Firmware:**
    ```sh
    Sketch -> Upload (Ctrl+U)
    ```

### Driver Linux

1. **Clone o Repositório:**
    ```sh
    git clone https://github.com/seuusuario/Hands-On-Linux.git
    cd Hands-On-Linux
    ```

2. **Compile o Driver:**
    ```sh
    cd smartlamp-kernel-module
    make
    ```

3. **Carregue o Driver:**
    ```sh
    sudo insmod smartlamp.ko
    ```

4. **Verifique o Driver:**
    ```sh
    dmesg | tail
    ```

## Uso

Depois que o driver e o firmware estiverem configurados, você poderá interagir com o dispositivo ESP32 através do sistema Linux.

- **Escrever para o Dispositivo:**
    ```sh
    echo "GET_LED" > /sys/kernel/smartlamp/led
    ```

- **Ler do Dispositivo:**
    ```sh
    cat /sys/kernel/smartlamp/led
    ```

- **Verificar Mensagens do Driver:**
    ```sh
    dmesg | tail
    ```

- **Remover o Driver:**
    ```sh
    sudo rmmod smartlamp
    ```
    
## Contato

Para perguntas, sugestões ou feedback, entre em contato com o mantenedor do projeto em [maintainer@example.com](mailto:maintainer@example.com).
