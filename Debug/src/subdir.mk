################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/0VideoOut.c \
../src/3d.c \
../src/D3DDemo.c \
../src/PaletteDemo.c \
../src/ParticleDemo.c \
../src/Shell.c \
../src/SpriteDemo.c \
../src/TileDemo.c \
../src/cr_startup_lpc11.c \
../src/math3D.c \
../src/rbox_main.c 

OBJS += \
./src/0VideoOut.o \
./src/3d.o \
./src/D3DDemo.o \
./src/PaletteDemo.o \
./src/ParticleDemo.o \
./src/Shell.o \
./src/SpriteDemo.o \
./src/TileDemo.o \
./src/cr_startup_lpc11.o \
./src/math3D.o \
./src/rbox_main.o 

C_DEPS += \
./src/0VideoOut.d \
./src/3d.d \
./src/D3DDemo.d \
./src/PaletteDemo.d \
./src/ParticleDemo.d \
./src/Shell.d \
./src/SpriteDemo.d \
./src/TileDemo.d \
./src/cr_startup_lpc11.d \
./src/math3D.d \
./src/rbox_main.d 


# Each subdirectory must supply rules for building sources it contributes
src/0VideoOut.o: ../src/0VideoOut.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O3 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/0VideoOut.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/3d.o: ../src/3d.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O3 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/3d.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/D3DDemo.o: ../src/D3DDemo.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/D3DDemo.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/PaletteDemo.o: ../src/PaletteDemo.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/PaletteDemo.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ParticleDemo.o: ../src/ParticleDemo.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/ParticleDemo.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/Shell.o: ../src/Shell.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/Shell.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/SpriteDemo.o: ../src/SpriteDemo.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/SpriteDemo.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/TileDemo.o: ../src/TileDemo.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/TileDemo.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/math3D.o: ../src/math3D.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -x c++ -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/math3D.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/rbox_main.o: ../src/rbox_main.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC11xx -D__CODE_RED -D__REDLIB__ -I"C:\Documents and Settings\Administrator\My Documents\workspace\CMSISv1p30_LPC11xx\inc" -O2 -O2 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/rbox_main.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


