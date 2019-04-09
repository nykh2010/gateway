################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/bi_server/bi_server.c 

OBJS += \
./source/bi_server/bi_server.o 

C_DEPS += \
./source/bi_server/bi_server.d 


# Each subdirectory must supply rules for building sources it contributes
source/bi_server/%.o: ../source/bi_server/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


