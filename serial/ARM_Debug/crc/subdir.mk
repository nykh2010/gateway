################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../crc/crc.c 

OBJS += \
./crc/crc.o 

C_DEPS += \
./crc/crc.d 


# Each subdirectory must supply rules for building sources it contributes
crc/%.o: ../crc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


