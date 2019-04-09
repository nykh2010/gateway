################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/inifun/inirw.c 

OBJS += \
./source/inifun/inirw.o 

C_DEPS += \
./source/inifun/inirw.d 


# Each subdirectory must supply rules for building sources it contributes
source/inifun/%.o: ../source/inifun/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


