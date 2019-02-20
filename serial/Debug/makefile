################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

CC := gcc
RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include serial/subdir.mk
-include packet/subdir.mk
-include debug_ARM/serial/subdir.mk
-include debug_ARM/packet/subdir.mk
-include debug_ARM/subdir.mk
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
all: serialapp

# Tool invocations
serialapp: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	${CC}  -o "serialapp" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) serialapp
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
