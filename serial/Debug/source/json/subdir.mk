################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/json/jsonfunc.c 

OBJS += \
./source/json/jsonfunc.o 

C_DEPS += \
./source/json/jsonfunc.d 


# Each subdirectory must supply rules for building sources it contributes
source/json/%.o: ../source/json/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


