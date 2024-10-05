#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>

MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Driver de acesso ao SmartLamp (ESP32 com Chip Serial CP2102)");
MODULE_LICENSE("GPL");

#define MAX_RECV_LINE 100 // Tamanho máximo de uma linha de resposta do dispositvo USB

static struct usb_device *smartlamp_device;        // Referência para o dispositivo USB
static uint usb_in, usb_out;                       // Endereços das portas de entrada e saída da USB
static char *usb_in_buffer, *usb_out_buffer;       // Buffers de entrada e saída da USB
static int usb_max_size;                           // Tamanho máximo de uma mensagem USB

#define VENDOR_ID   SUBSTITUA_PELO_VENDORID
#define PRODUCT_ID  SUBSTITUA_PELO_PRODUCTID
static const struct usb_device_id id_table[] = { { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, {} };

static int  usb_probe(struct usb_interface *ifce, const struct usb_device_id *id); 
static void usb_disconnect(struct usb_interface *ifce);
static int  usb_read_serial(void);   

// Função chamada ao ler os arquivos led e ldr
static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff) {
    if (strcmp(attr->attr.name, "led") == 0) {
        // Retorna o nome do usuário quando o arquivo 'led' é lido
        return sprintf(buff, "Joao Pedro\n");  // Nome de um dos integrantes
    } else if (strcmp(attr->attr.name, "ldr") == 0) {
        // Retorna "DevTITANS" quando o arquivo 'ldr' é lido
        return sprintf(buff, "DevTITANS\n");
    }
    return 0;
}

// Função chamada ao escrever nos arquivos led e ldr
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count) {
    int value;

    if (strcmp(attr->attr.name, "led") == 0) {
        // Escreve o valor no buffer do kernel e mostra via printk
        if (sscanf(buff, "%d", &value) == 1) {
            printk(KERN_INFO "SmartLamp: Valor escrito no LED: %d\n", value);
        }
    } else if (strcmp(attr->attr.name, "ldr") == 0) {
        // Retorna erro ao tentar escrever no arquivo 'ldr'
        printk(KERN_ERR "SmartLamp: Tentativa de escrita no arquivo LDR bloqueada.\n");
        return -EPERM;  // Retorna "Operação não permitida"
    }

    return count;
}

// Variáveis para criar os arquivos no /sys/kernel/smartlamp/{led, ldr}
static struct kobj_attribute  led_attribute = __ATTR(led, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct kobj_attribute  ldr_attribute = __ATTR(ldr, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct attribute      *attrs[]       = { &led_attribute.attr, &ldr_attribute.attr, NULL };
static struct attribute_group attr_group    = { .attrs = attrs };
static struct kobject        *sys_obj;                                            

MODULE_DEVICE_TABLE(usb, id_table);

bool ignore = true;
int LDR_value = 0;

static struct usb_driver smartlamp_driver = {
    .name        = "smartlamp",     
    .probe       = usb_probe,       
    .disconnect  = usb_disconnect,  
    .id_table    = id_table,        
};

module_usb_driver(smartlamp_driver);

// Executado quando o dispositivo é conectado na USB
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_endpoint_descriptor *usb_endpoint_in, *usb_endpoint_out;

    printk(KERN_INFO "SmartLamp: Dispositivo conectado ...\n");

    // Cria arquivos do /sys/kernel/smartlamp/*
    sys_obj = kobject_create_and_add("smartlamp", kernel_kobj);
    ignore = sysfs_create_group(sys_obj, &attr_group);

    // Detecta portas e aloca buffers de entrada e saída de dados na USB
    smartlamp_device = interface_to_usbdev(interface);
    ignore =  usb_find_common_endpoints(interface->cur_altsetting, &usb_endpoint_in, &usb_endpoint_out, NULL, NULL);
    usb_max_size = usb_endpoint_maxp(usb_endpoint_in);
    usb_in = usb_endpoint_in->bEndpointAddress;
    usb_out = usb_endpoint_out->bEndpointAddress;
    usb_in_buffer = kmalloc(usb_max_size, GFP_KERNEL);
    usb_out_buffer = kmalloc(usb_max_size, GFP_KERNEL);

    LDR_value = usb_read_serial();

    printk("LDR Value: %d\n", LDR_value);

    return 0;
}

// Executado quando o dispositivo USB é desconectado da USB
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "SmartLamp: Dispositivo desconectado.\n");
    if (sys_obj) kobject_put(sys_obj);      // Remove os arquivos do sysfs
}