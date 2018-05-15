################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/SystemClock/subdir.mk
-include src/StatusInd/subdir.mk
-include src/MessengerUART/subdir.mk
-include src/Init/subdir.mk
-include src/CommandFromUART/subdir.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include FreeRTOS/efm32gg/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32GG/subdir.mk
-include BSP/subdir.mk
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
all: PEP_HW.axf

# Tool invocations
PEP_HW.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m3 -mthumb -T "PEP_HW.ld" -Xlinker --gc-sections -Xlinker -Map="PEP_HW.map" --specs=nano.specs -u _printf_float -o PEP_HW.axf "./BSP/bsp_stk_buttons.o" "./BSP/bsp_stk_leds.o" "./CMSIS/EFM32GG/startup_gcc_efm32gg.o" "./CMSIS/EFM32GG/system_efm32gg.o" "./Drivers/retargetio.o" "./Drivers/retargetserial.o" "./Drivers/udelay.o" "./FreeRTOS/efm32gg/event_groups.o" "./FreeRTOS/efm32gg/heap_2.o" "./FreeRTOS/efm32gg/list.o" "./FreeRTOS/efm32gg/port_gcc.o" "./FreeRTOS/efm32gg/queue.o" "./FreeRTOS/efm32gg/tasks.o" "./FreeRTOS/efm32gg/timers.o" "./emlib/em_adc.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_rtc.o" "./emlib/em_system.o" "./emlib/em_usart.o" "./src/CommandFromUART/CommandFromUART.o" "./src/Init/Init.o" "./src/MessengerUART/MessengerUART.o" "./src/StatusInd/StatusInd.o" "./src/SystemClock/SystemClock.o" "./src/main.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: PEP_HW.hex'
	arm-none-eabi-objcopy -O ihex "PEP_HW.axf" "PEP_HW.hex"
	@echo ' '

	@echo 'Building bin file: PEP_HW.bin'
	arm-none-eabi-objcopy -O binary "PEP_HW.axf" "PEP_HW.bin"
	@echo ' '

	@echo 'Building s37 file: PEP_HW.s37'
	arm-none-eabi-objcopy -O srec "PEP_HW.axf" "PEP_HW.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "PEP_HW.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) PEP_HW.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
