//
// Adian Mikulic - Fall 2018 - CS410 - Assignment 3 - part 2
//

#include <stdio.h>
#include <stdlib.h>

#include "inttypes.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#include "driver/uart.h"

#define ECHO_TEST_TXD  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_RXD  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

// H-bridge (DC Motors) defines
#define GPIO_PWM0A_OUT 14   //Set GPIO 14 as PWM0A - 9 on H - bridge
#define GPIO_PWM0B_OUT 12   //Set GPIO 12 as PWM0B - 1 on H - bridge

#define DRIVE_GPIO 27 //  pin 15 on H - bridge upper-R motor
#define DRIVE_GPIO1 32 // pin 10 on H - bridge lower-R motor
#define DRIVE_GPIO2 33 // pin 2 on H - bridge  upper-L motor
#define DRIVE_GPIO3 15 // pin 7 on H - bridge lower-L motor

// LED defines
#define LED_FORWARD     18  // green
#define LED_STOP        5   // red
#define LED_BACKWARD    19  // blue

#define TURN_TIME           65
#define STOP_TIME           100
#define OBJECTIVE_STOP      200
#define FIRST_LEG           500
#define SECOND_LEG          325
#define THIRD_LEG           200
#define FOURTH_LEG          200
#define FIFTH_LEG           300
#define LAST_LEG            500

#define BUF_SIZE (1024)

//FUNCTION DECLARATIONS
void forward();
void fastForward();
void turnRight();
void turnLeft();
void backwards();

static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num);

int currentSpeed = 1;
int setSpeed = 1;

float distance; // The range value output by the red ultrasonic is stored in this variable

static void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm gpio...\n");


    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
}

static void motor_gpio_init()
{
    gpio_pad_select_gpio(DRIVE_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(DRIVE_GPIO, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(DRIVE_GPIO1);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(DRIVE_GPIO1, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(DRIVE_GPIO2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(DRIVE_GPIO2, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(DRIVE_GPIO3);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(DRIVE_GPIO3, GPIO_MODE_OUTPUT);
    gpio_set_level(DRIVE_GPIO2, 1);
    gpio_set_level(DRIVE_GPIO3, 0);
    gpio_set_level(DRIVE_GPIO, 1);
    gpio_set_level(DRIVE_GPIO1, 0);

    //1. mcpwm gpio initialization and mcpwm configuration
    mcpwm_example_gpio_initialize();
    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
}

static void led_gpio_init()
{
    gpio_pad_select_gpio(LED_FORWARD);
    gpio_set_direction(LED_FORWARD, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LED_STOP);
    gpio_set_direction(LED_STOP, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LED_BACKWARD);
    gpio_set_direction(LED_BACKWARD, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_FORWARD, 0);
    gpio_set_level(LED_STOP, 0);
    gpio_set_level(LED_BACKWARD, 0);
}

/**
 * @brief motor moves in right motor with duty cycle = duty %
 */
static void brushed_motor_Right(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    //mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves left motor with duty cycle = duty %
 */
static void brushed_motor_Left(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    //mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}

/**
 * @brief Control for left brushed dc motor
 */
void left_motor_control(float duty_cycle)
{
    brushed_motor_Left(MCPWM_UNIT_0, MCPWM_TIMER_0, duty_cycle);
}

/**
 * @brief Control for right brushed dc motor
 */
void right_motor_control(float duty_cycle)
{
    brushed_motor_Right(MCPWM_UNIT_0, MCPWM_TIMER_0, duty_cycle);
}


/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void mcpwm_example_brushed_motor_control(void *arg)
{
    //1. mcpwm gpio initialization and mcpwm configuration
    mcpwm_example_gpio_initialize();

    printf("Configuring Initial Parameters of mcpwm...\n");

    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
}

void turnLeft()
{
    gpio_set_level(DRIVE_GPIO2,1);
    gpio_set_level(DRIVE_GPIO3, 0);
    gpio_set_level(DRIVE_GPIO, 1);
    gpio_set_level(DRIVE_GPIO1, 0);

    printf("turning left...\n");
    gpio_set_level(LED_FORWARD, 0);
    gpio_set_level(LED_STOP, 1);
    gpio_set_level(LED_BACKWARD, 1);

    //printf("Turn left\n");
    brushed_motor_Right(MCPWM_UNIT_0, MCPWM_TIMER_0, 0);
    brushed_motor_Left(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
}

void turnRight()
{
    gpio_set_level(DRIVE_GPIO2,1);
    gpio_set_level(DRIVE_GPIO3, 0);
    gpio_set_level(DRIVE_GPIO, 1);
    gpio_set_level(DRIVE_GPIO1, 0);

    printf("turning right...\n");
    gpio_set_level(LED_FORWARD, 1);
    gpio_set_level(LED_STOP, 1);
    gpio_set_level(LED_BACKWARD, 0);

    //printf("Turn right\n");
    brushed_motor_Right(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
    brushed_motor_Left(MCPWM_UNIT_0, MCPWM_TIMER_0, 0);
}

void forward()
{
    gpio_set_level(DRIVE_GPIO2,1);
    gpio_set_level(DRIVE_GPIO3, 0);
    gpio_set_level(DRIVE_GPIO, 1);
    gpio_set_level(DRIVE_GPIO1, 0);

    printf("slow forward...\n");
    gpio_set_level(LED_FORWARD, 1);
    gpio_set_level(LED_STOP, 0);
    gpio_set_level(LED_BACKWARD, 0);

    //printf("Moving forward\n");
    brushed_motor_Right(MCPWM_UNIT_0, MCPWM_TIMER_0, 75);
    brushed_motor_Left(MCPWM_UNIT_0, MCPWM_TIMER_0, 75);
}

void fastForward()
{
    gpio_set_level(DRIVE_GPIO2,1);
    gpio_set_level(DRIVE_GPIO3, 0);
    gpio_set_level(DRIVE_GPIO, 1);
    gpio_set_level(DRIVE_GPIO1, 0);

    printf("fast forward...\n");
    gpio_set_level(LED_FORWARD, 1);
    gpio_set_level(LED_STOP, 0);
    gpio_set_level(LED_BACKWARD, 0);

    //printf("Moving forward fast\n");
    brushed_motor_Right(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
    brushed_motor_Left(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
}

void backwards()
{
    gpio_set_level(DRIVE_GPIO2, 0);
    gpio_set_level(DRIVE_GPIO3, 1);
    gpio_set_level(DRIVE_GPIO, 0);
    gpio_set_level(DRIVE_GPIO1, 1);

    printf("backwards...\n");
    gpio_set_level(LED_FORWARD, 0);
    gpio_set_level(LED_STOP, 0);
    gpio_set_level(LED_BACKWARD, 1);

    brushed_motor_Right(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
    brushed_motor_Left(MCPWM_UNIT_0, MCPWM_TIMER_0, 100);
}

void stop()
{
    printf("Stopped\n");
    gpio_set_level(LED_FORWARD, 0);
    gpio_set_level(LED_STOP, 1);
    gpio_set_level(LED_BACKWARD, 0);
    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
}

void app_main()
{
    motor_gpio_init();
    led_gpio_init();

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    //char data_in_char[2];
    int len;
    uint8_t hex = 0x0;

    while(1)
    {
        len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);

        if (len != 0)
        {
            printf("input detected!\n");
            // data_in_char[0] = (char)data[1];
            hex = (int)*data;

            if(hex == 103) // ascii for 'g'
            {
                printf("character match! starting driving sequence...\n");

                stop();
                vTaskDelay(STOP_TIME);

                forward();
                vTaskDelay(FIRST_LEG);

                stop();
                vTaskDelay(STOP_TIME);

                turnLeft();
                vTaskDelay(TURN_TIME);

                stop();
                vTaskDelay(STOP_TIME);

                forward();
                vTaskDelay(SECOND_LEG);

                stop();
                vTaskDelay(STOP_TIME);

                turnRight();
                vTaskDelay(TURN_TIME);

                stop();
                vTaskDelay(STOP_TIME);

                forward();
                vTaskDelay(THIRD_LEG);

                stop();
                vTaskDelay(OBJECTIVE_STOP);

                backwards();
                vTaskDelay(FOURTH_LEG);

                stop();
                vTaskDelay(STOP_TIME);

                turnRight();
                vTaskDelay(TURN_TIME);

                stop();
                vTaskDelay(STOP_TIME);

                forward();
                vTaskDelay(FIFTH_LEG);

                stop();
                vTaskDelay(STOP_TIME);

                turnRight();
                vTaskDelay(TURN_TIME);

                stop();
                vTaskDelay(STOP_TIME);

                fastForward();
                vTaskDelay(LAST_LEG);

                stop();

                printf("end of sequence\n");
            }
        }
        else
        {
            printf("listening...\n");
        }
        vTaskDelay(10);
    }
}
