################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include timer/subdir.mk
-include task/subdir.mk
-include server/subdir.mk
-include serialp/subdir.mk
-include rts/subdir.mk
-include packet/subdir.mk
-include ots/subdir.mk
-include log/subdir.mk
-include inifun/subdir.mk
-include example/subdir.mk
-include entry/subdir.mk
-include crc/subdir.mk
-include client/subdir.mk
-include bilink/subdir.mk
-include ARM_Debug/timer/subdir.mk
-include ARM_Debug/task/subdir.mk
-include ARM_Debug/server/subdir.mk
-include ARM_Debug/serialp/subdir.mk
-include ARM_Debug/rts/subdir.mk
-include ARM_Debug/packet/subdir.mk
-include ARM_Debug/ots/subdir.mk
-include ARM_Debug/log/subdir.mk
-include ARM_Debug/inifun/subdir.mk
-include ARM_Debug/example/subdir.mk
-include ARM_Debug/entry/subdir.mk
-include ARM_Debug/crc/subdir.mk
-include ARM_Debug/client/subdir.mk
-include ARM_Debug/bilink/subdir.mk
-include ARM_Debug/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: serial

# Tool invocations
serial: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc  -o "serial" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) serial
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
