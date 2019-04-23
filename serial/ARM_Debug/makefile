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
	${CC}  -o "serial" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) serial
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
