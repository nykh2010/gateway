################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/bi_client/bi_client.c 

OBJS += \
./source/bi_client/bi_client.o 

C_DEPS += \
./source/bi_client/bi_client.d 


# Each subdirectory must supply rules for building sources it contributes
source/bi_client/%.o: ../source/bi_client/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


