################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ots/ots.c 

OBJS += \
./ots/ots.o 

C_DEPS += \
./ots/ots.d 


# Each subdirectory must supply rules for building sources it contributes
ots/%.o: ../ots/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

