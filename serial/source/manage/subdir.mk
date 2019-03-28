################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/manage/client.c \
../source/manage/manage.c 

OBJS += \
./source/manage/client.o \
./source/manage/manage.o 

C_DEPS += \
./source/manage/client.d \
./source/manage/manage.d 


# Each subdirectory must supply rules for building sources it contributes
source/manage/%.o: ../source/manage/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


