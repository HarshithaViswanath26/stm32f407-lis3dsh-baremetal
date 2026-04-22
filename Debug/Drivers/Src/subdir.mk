################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Src/gpio.c \
../Drivers/Src/rcc.c 

OBJS += \
./Drivers/Src/gpio.o \
./Drivers/Src/rcc.o 

C_DEPS += \
./Drivers/Src/gpio.d \
./Drivers/Src/rcc.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Src/%.o Drivers/Src/%.su Drivers/Src/%.cyclo: ../Drivers/Src/%.c Drivers/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"C:/Users/49157/Desktop/ST/Projects/stm32f407-lis3dsh-baremetal/Drivers/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Src

clean-Drivers-2f-Src:
	-$(RM) ./Drivers/Src/gpio.cyclo ./Drivers/Src/gpio.d ./Drivers/Src/gpio.o ./Drivers/Src/gpio.su ./Drivers/Src/rcc.cyclo ./Drivers/Src/rcc.d ./Drivers/Src/rcc.o ./Drivers/Src/rcc.su

.PHONY: clean-Drivers-2f-Src

