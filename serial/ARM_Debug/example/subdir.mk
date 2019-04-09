################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../example/entry_test.c \
../example/ots_test.c \
../example/rts_test.c 

OBJS += \
./example/entry_test.o \
./example/ots_test.o \
./example/rts_test.o 

C_DEPS += \
./example/entry_test.d \
./example/ots_test.d \
./example/rts_test.d 


# Each subdirectory must supply rules for building sources it contributes
example/%.o: ../example/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


