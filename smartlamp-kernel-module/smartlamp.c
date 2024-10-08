#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>

MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Driver de acesso ao SmartLamp (ESP32 com Chip Serial CP2102");
MODULE_LICENSE("GPL");


#define MAX_RECV_LINE 100 // Tamanho máximo de uma linha de resposta do dispositvo USB


//static char recv_line[MAX_RECV_LINE];              // Armazena dados vindos da USB até receber um caractere de nova linha '\n'
static struct usb_device *smartlamp_device;        // Referência para o dispositivo USB
static uint usb_in, usb_out;                       // Endereços das portas de entrada e saida da USB
static char *usb_in_buffer, *usb_out_buffer;       // Buffers de entrada e saída da USB
static int usb_max_size;                           // Tamanho máximo de uma mensagem USB
static char *COMANDO_SMARTLAMP;
static int resp;

#define VENDOR_ID   0x10c4 /* Encontre o VendorID  do smartlamp */
#define PRODUCT_ID  0xea60 /* Encontre o ProductID do smartlamp */
static const struct usb_device_id id_table[] = { { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, {} };

static int  usb_probe(struct usb_interface *ifce, const struct usb_device_id *id); // Executado quando o dispositivo é conectado na USB
static void usb_disconnect(struct usb_interface *ifce);                           // Executado quando o dispositivo USB é desconectado da USB
static int usb_send_cmd(char *cmd, int param);

// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é lido (e.g., cat /sys/kernel/smartlamp/led)
static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff);
// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é escrito (e.g., echo "100" | sudo tee -a /sys/kernel/smartlamp/led)
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count);   
// Variáveis para criar os arquivos no /sys/kernel/smartlamp/{led, ldr}
static struct kobj_attribute  led_attribute = __ATTR(led, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct kobj_attribute  ldr_attribute = __ATTR(ldr, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct kobj_attribute temp_attribute = __ATTR(temp, S_IRUGO, attr_show, NULL);  // Somente leitura
static struct kobj_attribute hum_attribute  = __ATTR(hum, S_IRUGO, attr_show, NULL);   // Somente leitura
static struct attribute      *attrs[]       = { &led_attribute.attr, &ldr_attribute.attr, &temp_attribute.attr, &hum_attribute.attr, NULL};
static struct attribute_group attr_group    = { .attrs = attrs };
static struct kobject        *sys_obj;                                             // Executado para ler a saida da porta serial

MODULE_DEVICE_TABLE(usb, id_table);

bool ignore = true;
int LDR_value = 0;

static struct usb_driver smartlamp_driver = {
    .name        = "smartlamp",     // Nome do driver
    .probe       = usb_probe,       // Executado quando o dispositivo é conectado na USB
    .disconnect  = usb_disconnect,  // Executado quando o dispositivo é desconectado na USB
    .id_table    = id_table,        // Tabela com o VendorID e ProductID do dispositivo
};

module_usb_driver(smartlamp_driver);

// Executado quando o dispositivo é conectado na USB
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_endpoint_descriptor *usb_endpoint_in, *usb_endpoint_out;

    printk(KERN_INFO "SmartLamp: Dispositivo conectado ...\n");

    // Cria arquivos do /sys/kernel/smartlamp/*
    sys_obj = kobject_create_and_add("smartlamp", kernel_kobj);
    ignore = sysfs_create_group(sys_obj, &attr_group); // AQUI

    // Detecta portas e aloca buffers de entrada e saída de dados na USB
    smartlamp_device = interface_to_usbdev(interface);
    ignore =  usb_find_common_endpoints(interface->cur_altsetting, &usb_endpoint_in, &usb_endpoint_out, NULL, NULL);  // AQUI
    usb_max_size = usb_endpoint_maxp(usb_endpoint_in);
    usb_in = usb_endpoint_in->bEndpointAddress;
    usb_out = usb_endpoint_out->bEndpointAddress;
    usb_in_buffer = kmalloc(usb_max_size, GFP_KERNEL);
    usb_out_buffer = kmalloc(usb_max_size, GFP_KERNEL);

    COMANDO_SMARTLAMP = "SET_LED";
    resp = usb_send_cmd(COMANDO_SMARTLAMP, 50);

    printk("Comando: %s", COMANDO_SMARTLAMP);
    printk("Valor do comando: %d", resp);

    return 0;
}

// Executado quando o dispositivo USB é desconectado da USB
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "SmartLamp: Dispositivo desconectado.\n");
    if (sys_obj) kobject_put(sys_obj);      // Remove os arquivos em /sys/kernel/smartlamp
    kfree(usb_in_buffer);                   // Desaloca buffers
    kfree(usb_out_buffer);
}

// Envia um comando via USB, espera e retorna a resposta do dispositivo (convertido para int)
// Exemplo de Comando:  SET_LED 80
// Exemplo de Resposta: RES SET_LED 1
// Exemplo de chamada da função usb_send_cmd para SET_LED: usb_send_cmd("SET_LED", 80);
static int usb_send_cmd(char *cmd, int param) {
    int ret, actual_size;
    int retries = 25;  // Tentativas de leitura da resposta
    char resp_expected[MAX_RECV_LINE];  // Resposta esperada do comando
    int resp_number = -1;  // Número retornado pelo dispositivo
    char temp_buffer[MAX_RECV_LINE];  // Buffer temporário para ir acrescentando
    char *start_cmd;  // Para identificar o fim da linha
    sprintf(usb_out_buffer, "%s %d", cmd, param);
    
    printk(KERN_INFO "SmartLamp: Enviando comando: %s\n", usb_out_buffer);


    // preencha o buffer                     // Caso contrário, é só o comando mesmo

    // Envia o comando (usb_out_buffer) para a USB
    // Procure a documentação da função usb_bulk_msg para entender os parâmetros
    //sprintf(usb_out_buffer, cmd); //use a variavel usb_out_buffer para armazernar o comando em formato de texto que o firmware reconheça

    ret = usb_bulk_msg(smartlamp_device, usb_sndbulkpipe(smartlamp_device, usb_out), usb_out_buffer, strlen(usb_out_buffer), &actual_size, 1000*HZ);
    if (ret) {
        printk(KERN_ERR "SmartLamp: Erro de codigo %d ao enviar comando!\n", ret);
        return -1;
    }
    sprintf(resp_expected, "RES %s", cmd);  // Resposta esperada. Ficará lendo linhas até receber essa resposta.


    // Espera pela resposta correta do dispositivo (desiste depois de várias tentativas)
    while (retries > 0) {
        // Lê os dados da porta serial e armazena em usb_in_buffer
        ret = usb_bulk_msg(smartlamp_device, usb_rcvbulkpipe(smartlamp_device, usb_in), usb_in_buffer, min(usb_max_size, MAX_RECV_LINE), &actual_size, 1000);
        if (ret) {
            printk(KERN_ERR "SmartLamp: Erro ao ler dados da USB (tentativa %d). Código: %d\n", retries, ret);
            retries--;
            continue;   
        }

        printk("Buffer: %s", usb_in_buffer);

        // Acumula o conteúdo lido no temp_buffer até encontrar '\n'
        strncat(temp_buffer, usb_in_buffer, actual_size);

        // Verifica se o buffer temporário contém um fim de linha
        printk("Buffer temporario: %s", temp_buffer);

        // Verifica se o buffer contém o comando esperado
        start_cmd = strstr(temp_buffer, resp_expected);
        if (start_cmd != NULL) {
            // Tenta encontrar o número após a resposta esperada, sem mover ponteiros originais
            char *param_position = start_cmd + strlen(resp_expected);
        
            // Tenta encontrar o número após a substring e armazená-lo em resp_number
            if (sscanf(param_position, "%d", &resp_number) == 1) {
                printk(KERN_INFO "SmartLamp: Valor do comando lido: %d\n", resp_number);
                return resp_number;  // Retorna o valor obtido
            } else {
                printk(KERN_ERR "SmartLamp: Falha ao extrair o valor após o comando esperado.\n");
            }
        }

        // Reseta o temp_buffer após processar uma linha completa
        //memset(temp_buffer, 0, sizeof(temp_buffer));

        retries--;
    }

    // Se todas as tentativas falharem, retorna -1
    printk(KERN_ERR "SmartLamp: Falha ao ler o valor do comando após várias tentativas.\n");
    return -1;
}

// Função chamada ao ler os arquivos led, ldr, temp, hum
static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff) {
    int result;

    if (strcmp(attr->attr.name, "led") == 0) {
        result = usb_send_cmd("GET_LED", -1);
        if (result >= 0) {
            return sprintf(buff, "%d\n", result);  // Retorna o valor do LED lido
        }
        printk(KERN_ERR "SmartLamp: Falha ao ler o valor do LED.\n");
        return -EIO;  // Retorna erro de I/O se a leitura falhar
    } else if (strcmp(attr->attr.name, "ldr") == 0) {
        result = usb_send_cmd("GET_LDR", -1);
        if (result >= 0) {
            return sprintf(buff, "%d\n", result);  // Retorna o valor do LDR lido
        }
        printk(KERN_ERR "SmartLamp: Falha ao ler o valor do LDR.\n");
        return -EIO;  // Retorna erro de I/O se a leitura falhar
    } else if (strcmp(attr->attr.name, "temp") == 0) {
        // Envia o comando GET_TEMP com o parâmetro -1
        result = usb_send_cmd("GET_TEMP", -1);
        if (result >= 0) {
            return sprintf(buff, "%d\n", result);  // Retorna o valor da temperatura lida
        }
        printk(KERN_ERR "SmartLamp: Falha ao ler o valor da temperatura.\n");
        return -EIO;  // Retorna erro de I/O se a leitura falhar
    } else if (strcmp(attr->attr.name, "hum") == 0) {
        // Envia o comando GET_HUM com o parâmetro -1
        result = usb_send_cmd("GET_HUM", -1);
        if (result >= 0) {
            return sprintf(buff, "%d\n", result);  // Retorna o valor da umidade lida
        }
        printk(KERN_ERR "SmartLamp: Falha ao ler o valor da umidade.\n");
        return -EIO;  // Retorna erro de I/O se a leitura falhar
    }
    
    return 0;
}



// Essa função não deve ser alterada durante a task sysfs
// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é escrito (e.g., echo "100" | sudo tee -a /sys/kernel/smartlamp/led)
// Função chamada ao escrever nos arquivos led e ldr
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count) {
    int value;

    if (strcmp(attr->attr.name, "led") == 0) {
        // Lê o valor fornecido e envia o comando SET_LED
        if (sscanf(buff, "%d", &value) == 1) {
            printk(KERN_INFO "SmartLamp: Valor escrito no LED: %d\n", value);
            usb_send_cmd("SET_LED", value);  // Envia o comando SET_LED com o valor escrito
        }
    } else if (strcmp(attr->attr.name, "ldr") == 0) {
        // Retorna erro ao tentar escrever no arquivo 'ldr'
        printk(KERN_ERR "SmartLamp: Tentativa de escrita no arquivo LDR bloqueada.\n");
        return -EPERM;  // Retorna "Operação não permitida"
    }

    return count;
}