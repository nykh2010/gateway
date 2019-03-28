################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include source/timer/subdir.mk
-include source/task/subdir.mk
-include source/serial/subdir.mk
-include source/rts/subdir.mk
-include source/packet/subdir.mk
-include source/ots/subdir.mk
-include source/log/subdir.mk
-include source/inifun/subdir.mk
-include source/entry/subdir.mk
-include source/bilink/subdir.mk
-include source/bi_server/subdir.mk
-include source/bi_client/subdir.mk
-include source/subdir.mk
-include example/subdir.mk
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
	gcc  -o "serialapp" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) serialapp
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
