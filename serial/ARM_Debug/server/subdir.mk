################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../server/server.c 

OBJS += \
./server/server.o 

C_DEPS += \
./server/server.d 


# Each subdirectory must supply rules for building sources it contributes
server/%.o: ../server/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


