################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Configuracion.c \
../Globales.c \
../Serial.c \
../Serializacion.c \
../Sockets.c 

OBJS += \
./Configuracion.o \
./Globales.o \
./Serial.o \
./Serializacion.o \
./Sockets.o 

C_DEPS += \
./Configuracion.d \
./Globales.d \
./Serial.d \
./Serializacion.d \
./Sockets.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


