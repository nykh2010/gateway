################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bilink/bilink.c \
../bilink/bilink_packet.c 

OBJS += \
./bilink/bilink.o \
./bilink/bilink_packet.o 

C_DEPS += \
./bilink/bilink.d \
./bilink/bilink_packet.d 


# Each subdirectory must supply rules for building sources it contributes
bilink/%.o: ../bilink/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


