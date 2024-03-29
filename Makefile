BASE = ../../../
FCMAKE = C:/Program Files/FITkit/bin/fcmake.exe
FKFLASH = C:/Program Files/FITkit/bin/fkflash.exe
FKTERM = C:/Program Files/FITkit/bin/fkterm.exe
FKTEST = C:/Program Files/FITkit/bin/fktest.exe
PROJECT = project.xml
OUTPUTPREFIX = snake
FPGACHIP = xc3s50
FPGASPEEDGRADE = 4
FPGAPACKAGE = pq208

all: dependencycheck build/snake_f1xx.hex build/snake_f2xx.hex build/snake.bin

#MCU part
#=====================================================================
HEXFILE_F1XX = build/snake_f1xx.hex
HEXFILE_F2XX = build/snake_f2xx.hex

build/mcu/display_f1xx.o: ../../../mcu/libs/lcd/display.c
	$(comp_tpl_f1xx)

build/mcu/display_f2xx.o: ../../../mcu/libs/lcd/display.c
	$(comp_tpl_f2xx)

build/mcu/thermometer_f1xx.o: ../../../mcu/libs/thermometer/thermometer.c
	$(comp_tpl_f1xx)

build/mcu/thermometer_f2xx.o: ../../../mcu/libs/thermometer/thermometer.c
	$(comp_tpl_f2xx)

build/mcu/snake_f1xx.o: mcu/snake.c
	$(comp_tpl_f1xx)

build/mcu/snake_f2xx.o: mcu/snake.c
	$(comp_tpl_f2xx)

build/mcu/vga_block_f1xx.o: mcu/vga_block.c
	$(comp_tpl_f1xx)

build/mcu/vga_block_f2xx.o: mcu/vga_block.c
	$(comp_tpl_f2xx)

build/mcu/keyboard_f1xx.o: mcu/keyboard.c
	$(comp_tpl_f1xx)

build/mcu/keyboard_f2xx.o: mcu/keyboard.c
	$(comp_tpl_f2xx)

build/mcu/main_f1xx.o: mcu/main.c
	$(comp_tpl_f1xx)

build/mcu/main_f2xx.o: mcu/main.c
	$(comp_tpl_f2xx)

OBJFILES_F1XX = build/mcu/display_f1xx.o build/mcu/thermometer_f1xx.o build/mcu/snake_f1xx.o build/mcu/vga_block_f1xx.o build/mcu/keyboard_f1xx.o build/mcu/main_f1xx.o
OBJFILES_F2XX = build/mcu/display_f2xx.o build/mcu/thermometer_f2xx.o build/mcu/snake_f2xx.o build/mcu/vga_block_f2xx.o build/mcu/keyboard_f2xx.o build/mcu/main_f2xx.o

#FPGA part
#=====================================================================
BINFILE = build/snake.bin
HDLFILES  = ../../../fpga/units/clkgen/clkgen_config.vhd
HDLFILES += ../../../fpga/units/clkgen/clkgen.vhd
HDLFILES += ../../../fpga/units/math/math_pack.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc_entity.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc_autoincr.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_reg.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_ctrl.vhd
HDLFILES += ../../../fpga/chips/fpga_xc3s50.vhd
HDLFILES += ../../../fpga/chips/architecture_pc/arch_pc_ifc.vhd
HDLFILES += ../../../fpga/chips/architecture_pc/tlv_pc_ifc.vhd
HDLFILES += ../../../fpga/ctrls/vga/vga_config.vhd
HDLFILES += ../../../fpga/ctrls/vga/vga_ctrl.vhd
HDLFILES += ../../../fpga/ctrls/lcd/lcd_raw.vhd
HDLFILES += ../../../fpga/ctrls/keyboard/keyboard_ctrl.vhd
HDLFILES += ../../../fpga/ctrls/keyboard/keyboard_ctrl_high.vhd
HDLFILES += fpga/snake_top.vhd

build/snake.bin: build/fpga/snake.par.ncd build/fpga/snake.pcf

PKGS_LIST = ../../../mcu/libs/lcd/package.xml
PKGS_LIST += ../../../mcu/libs/thermometer/package.xml
PKGS_LIST += ../../../fpga/units/clkgen/package.xml
PKGS_LIST += ../../../fpga/units/math/package.xml
PKGS_LIST += ../../../fpga/ctrls/spi/package.xml
PKGS_LIST += ../../../fpga/chips/architecture_pc/package.xml
PKGS_LIST += ../../../fpga/ctrls/vga/package.xml
PKGS_LIST += ../../../fpga/ctrls/lcd/package.xml
PKGS_LIST += ../../../fpga/ctrls/keyboard/package.xml

include $(BASE)/base/Makefile.inc
