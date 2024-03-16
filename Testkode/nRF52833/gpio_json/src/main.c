
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <stdbool.h>
#include <string.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000
/* To properly simulate the json format we should have multiple input types */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);

struct gpio_data{
        bool led0;
        bool led1;
        bool led2;
        bool led3;
        bool btn0;
        bool btn1;
        bool btn2;
        bool btn3;
};

/* UART SETUP */
static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));
#define MSG_SIZE 12
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 5, 4);
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;
/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;
	if (!uart_irq_update(uart_dev)) {
		return;
	}
	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}
	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}
/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
	int msg_len = strlen(buf);
	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

void bool_to_json(char *str, bool b){
  print_uart("\"");print_uart(str); print_uart("\"");
  print_uart(" : ");
  char * bOut = b ? "true" : "false";
  print_uart("\"");print_uart(bOut);print_uart("\"");
}


void gpio_report_to_json(struct gpio_data *fetched_gpio_data, uint8_t* command, size_t size){
  print_uart("{");

  //- Print command
  print_uart("\"command\" : ");
  print_uart("\"");
  char buffer[MSG_SIZE];
  for(int i = 0; i < MSG_SIZE; i++){
       buffer[i] = (char)command[i];
  }
  print_uart(buffer);
  print_uart("\"");
  print_uart(",");

  bool_to_json("Led0", fetched_gpio_data->led0);print_uart(",");
  bool_to_json("Led1", fetched_gpio_data->led1);print_uart(",");
  bool_to_json("Led2", fetched_gpio_data->led2);print_uart(",");
  bool_to_json("Led3", fetched_gpio_data->led3);print_uart(",");
  //- Print led states
  bool_to_json("SW0", fetched_gpio_data->btn0);print_uart(",");
  bool_to_json("SW1", fetched_gpio_data->btn1);print_uart(",");
  bool_to_json("SW2", fetched_gpio_data->btn2);print_uart(",");
  bool_to_json("SW3", fetched_gpio_data->btn3);print_uart(",");

  print_uart("}\n\r");
}


int main(void)
{
	int ret;
	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
		return 0;
	}
	if (!device_is_ready(led0.port)){
		printk("GPIO device is not ready\r\n");
		return 1;
	}
        if (!device_is_ready(button0.port)) {
		printk("GPIO device is not ready\r\n");
		return 1;
	}
        ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
        if (ret < 0) {
                return 1 ; 
        }
        ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
        if (ret < 0) {
                return 1 ;
        }
        ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
        if (ret < 0) {
                return 1 ;
        }
        ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
        if (ret < 0){
                return 1;
        }
        ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
	if (ret < 0) {
		return 1;
	}
        ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
	if (ret < 0) {
		return 1;
	}
        ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
	if (ret < 0) {
		return 1;
	}
        ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
	if (ret < 0) {
		return 1;
	}
	ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
        
        struct gpio_data fetched_gpio_data;
        uint8_t command[MSG_SIZE];

        while(1){
        /* Reset data */
        fetched_gpio_data.led0 = 0;
        fetched_gpio_data.led1 = 0;
        fetched_gpio_data.led2 = 0;
        fetched_gpio_data.led3 = 0;
        fetched_gpio_data.btn0 = 0;
        fetched_gpio_data.btn1 = 0;
        fetched_gpio_data.btn2 = 0;
        fetched_gpio_data.btn3 = 0;

        /* Wait for command and number */
        uart_irq_rx_enable(uart_dev);
	print_uart("Waiting for a command.\r\n");
	while (k_msgq_get(&uart_msgq, &command, K_FOREVER) == 0) {
		print_uart("Echo: ");
		print_uart(command);
		print_uart("\r\n");
                break;
	}
        uart_irq_rx_disable(uart_dev);

        /* Do reading*/
        fetched_gpio_data.led0 = gpio_pin_get_dt(&led0);
        fetched_gpio_data.led1 = gpio_pin_get_dt(&led1);
        fetched_gpio_data.led2 = gpio_pin_get_dt(&led2);
        fetched_gpio_data.led3 = gpio_pin_get_dt(&led3);
        fetched_gpio_data.btn0 = gpio_pin_get_dt(&button0);
        fetched_gpio_data.btn1 = gpio_pin_get_dt(&button1);
        fetched_gpio_data.btn2 = gpio_pin_get_dt(&button2);
        fetched_gpio_data.btn3 = gpio_pin_get_dt(&button3);


        /* Report result*/
        gpio_report_to_json(&fetched_gpio_data, command, MSG_SIZE);
        }
}