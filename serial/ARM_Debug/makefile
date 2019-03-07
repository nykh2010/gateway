################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include timer/subdir.mk
-include thread/subdir.mk
-include serial/subdir.mk
-include rts/subdir.mk
-include packet/subdir.mk
-include ots/subdir.mk
-include arm_Debug/timer/subdir.mk
-include arm_Debug/thread/subdir.mk
-include arm_Debug/serial/subdir.mk
-include arm_Debug/rts/subdir.mk
-include arm_Debug/packet/subdir.mk
-include arm_Debug/ots/subdir.mk
-include arm_Debug/subdir.mk
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
