################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/main.c \
../src/syscalls.c \
../src/system_stm32f4xx.c \
../src/tm_stm32f4_dac.c \
../src/tm_stm32f4_delay.c \
../src/tm_stm32f4_gpio.c \
../src/tm_stm32f4_nrf24l01.c \
../src/tm_stm32f4_pwm.c \
../src/tm_stm32f4_spi.c \
../src/tm_stm32f4_timer_properties.c \
../src/tm_stm32f4_usart.c 

OBJS += \
./src/main.o \
./src/syscalls.o \
./src/system_stm32f4xx.o \
./src/tm_stm32f4_dac.o \
./src/tm_stm32f4_delay.o \
./src/tm_stm32f4_gpio.o \
./src/tm_stm32f4_nrf24l01.o \
./src/tm_stm32f4_pwm.o \
./src/tm_stm32f4_spi.o \
./src/tm_stm32f4_timer_properties.o \
./src/tm_stm32f4_usart.o 

C_DEPS += \
./src/main.d \
./src/syscalls.d \
./src/system_stm32f4xx.d \
./src/tm_stm32f4_dac.d \
./src/tm_stm32f4_delay.d \
./src/tm_stm32f4_gpio.d \
./src/tm_stm32f4_nrf24l01.d \
./src/tm_stm32f4_pwm.d \
./src/tm_stm32f4_spi.d \
./src/tm_stm32f4_timer_properties.d \
./src/tm_stm32f4_usart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32F4 -DSTM32F407VGTx -DSTM32F407G_DISC1 -DDEBUG -DSTM32F40XX -DSTM32F40_41xxx -DUSE_STDPERIPH_DRIVER -I"C:/Users/eepc17adm/workspace/ppm_test/StdPeriph_Driver/inc" -I"C:/Users/eepc17adm/workspace/ppm_test/inc" -I"C:/Users/eepc17adm/workspace/ppm_test/CMSIS/device" -I"C:/Users/eepc17adm/workspace/ppm_test/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


