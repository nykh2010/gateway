################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/bilink/bilink.c \
../source/bilink/bilink_packet.c 

OBJS += \
./source/bilink/bilink.o \
./source/bilink/bilink_packet.o 

C_DEPS += \
./source/bilink/bilink.d \
./source/bilink/bilink_packet.d 


# Each subdirectory must supply rules for building sources it contributes
source/bilink/%.o: ../source/bilink/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


