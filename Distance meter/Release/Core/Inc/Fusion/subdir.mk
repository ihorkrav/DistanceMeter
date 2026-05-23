################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/Fusion/FusionAhrs.c \
../Core/Inc/Fusion/FusionBias.c \
../Core/Inc/Fusion/FusionCompass.c 

OBJS += \
./Core/Inc/Fusion/FusionAhrs.o \
./Core/Inc/Fusion/FusionBias.o \
./Core/Inc/Fusion/FusionCompass.o 

C_DEPS += \
./Core/Inc/Fusion/FusionAhrs.d \
./Core/Inc/Fusion/FusionBias.d \
./Core/Inc/Fusion/FusionCompass.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/Fusion/%.o Core/Inc/Fusion/%.su Core/Inc/Fusion/%.cyclo: ../Core/Inc/Fusion/%.c Core/Inc/Fusion/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-Fusion

clean-Core-2f-Inc-2f-Fusion:
	-$(RM) ./Core/Inc/Fusion/FusionAhrs.cyclo ./Core/Inc/Fusion/FusionAhrs.d ./Core/Inc/Fusion/FusionAhrs.o ./Core/Inc/Fusion/FusionAhrs.su ./Core/Inc/Fusion/FusionBias.cyclo ./Core/Inc/Fusion/FusionBias.d ./Core/Inc/Fusion/FusionBias.o ./Core/Inc/Fusion/FusionBias.su ./Core/Inc/Fusion/FusionCompass.cyclo ./Core/Inc/Fusion/FusionCompass.d ./Core/Inc/Fusion/FusionCompass.o ./Core/Inc/Fusion/FusionCompass.su

.PHONY: clean-Core-2f-Inc-2f-Fusion

