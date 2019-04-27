################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../client/client.c 

OBJS += \
./client/client.o 

C_DEPS += \
./client/client.d 


# Each subdirectory must supply rules for building sources it contributes
client/%.o: ../client/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


