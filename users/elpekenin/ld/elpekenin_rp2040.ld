/*
    ChibiOS - Copyright (C) 2006..2021 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * RP2040 memory setup.
 */
MEMORY
{
    flash0 (rx) : org = 0x00000000, len = 16k   /* ROM                  */
    flash1 (rx) : org = 0x10000000, len = DEFINED(FLASH_LEN) ? FLASH_LEN : 2048k /* XIP */
    flash2 (rx) : org = 0x00000000, len = 0
    flash3 (rx) : org = 0x00000000, len = 0
    flash4 (rx) : org = 0x00000000, len = 0
    flash5 (rx) : org = 0x00000000, len = 0
    flash6 (rx) : org = 0x00000000, len = 0
    flash7 (rx) : org = 0x00000000, len = 0

    /**
     * ram0 modified to round_down(8/9) of total RAM size
     * ram1 seems wrong, should probably remove it or make it `len = 0`
     * ram2 "created" after ram0, with round_up(1/9) of its length, as shadow memory for KASAN
     *   - this is because rules_memory "consumes" ram0 to reserve space for heap
     *   - however, we want our extra symbols and space on RAM, to track (un)usable addresses
     *   - this solution was much simpler and cleaner than hacking/duplicating ChibiOS' file
     */

    ram0   (wx) : org = 0x20000000, len = 227k  /* SRAM0 striped        */
    ram1   (wx) : org = 0x00000000, len = 256k  /* SRAM0 non striped    */
    ram2   (wx) : org = 0x20038c00, len = 29k   /* End of SRAM0         */
    ram3   (wx) : org = 0x00000000, len = 0
    ram4   (wx) : org = 0x20040000, len = 4k    /* SRAM4                */
    ram5   (wx) : org = 0x20041000, len = 4k    /* SRAM5                */
    ram6   (wx) : org = 0x00000000, len = 0
    ram7   (wx) : org = 0x20041f00, len = 256   /* SRAM5 boot           */
}

/* For each data/text section two region are defined, a virtual region
   and a load region (_LMA suffix).*/

/* Flash region to be used for exception vectors.*/
REGION_ALIAS("VECTORS_FLASH", flash1);
REGION_ALIAS("VECTORS_FLASH_LMA", flash1);

/* Flash region to be used for constructors and destructors.*/
REGION_ALIAS("XTORS_FLASH", flash1);
REGION_ALIAS("XTORS_FLASH_LMA", flash1);

/* Flash region to be used for code text.*/
REGION_ALIAS("TEXT_FLASH", flash1);
REGION_ALIAS("TEXT_FLASH_LMA", flash1);

/* Flash region to be used for read only data.*/
REGION_ALIAS("RODATA_FLASH", flash1);
REGION_ALIAS("RODATA_FLASH_LMA", flash1);

/* Flash region to be used for various.*/
REGION_ALIAS("VARIOUS_FLASH", flash1);
REGION_ALIAS("VARIOUS_FLASH_LMA", flash1);

/* Flash region to be used for RAM(n) initialization data.*/
REGION_ALIAS("RAM_INIT_FLASH_LMA", flash1);

/* RAM region to be used for Main stack. This stack accommodates the processing
   of all exceptions and interrupts.*/
REGION_ALIAS("MAIN_STACK_RAM", ram4);

/* RAM region to be used for the process stack. This is the stack used by
   the main() function.*/
REGION_ALIAS("PROCESS_STACK_RAM", ram4);

/* RAM region to be used for Main stack. This stack accommodates the processing
   of all exceptions and interrupts.*/
REGION_ALIAS("C1_MAIN_STACK_RAM", ram5);

/* RAM region to be used for the process stack. This is the stack used by
   the main() function.*/
REGION_ALIAS("C1_PROCESS_STACK_RAM", ram5);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", ram0);
REGION_ALIAS("DATA_RAM_LMA", flash1);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", ram0);

/* RAM region to be used for the default heap.*/
REGION_ALIAS("HEAP_RAM", ram0);

SECTIONS
{
    .flash_begin : {
        __flash_binary_start = .;
    } > flash1

    .boot2 : {
        __boot2_start__ = .;
        KEEP (*(.boot2))
        __boot2_end__ = .;
    } > flash1
}

/* Generic rules inclusion.*/
INCLUDE rules_stacks.ld
INCLUDE rules_stacks_c1.ld
INCLUDE RP2040_rules_code_with_boot2.ld
INCLUDE RP2040_rules_data_with_timecrit.ld

/* elpekenin change: keep pico SDK symbols */
SECTIONS {
    .preinit : ALIGN(4){
        . = ALIGN(4);
        /* preinit data */
        __preinit_array_base__ = .;
        KEEP(*(SORT(.preinit_array.*)))
        KEEP(*(.preinit_array))
        __preinit_array_end__ = .;
    } > XTORS_FLASH AT > XTORS_FLASH_LMA
}

/* elpekenin change: kasan */
SECTIONS {
    .kasan_shadow : {
        . = ALIGN(8);
        __kasan_shadow_base__ = .;
        . = ORIGIN(ram2) + LENGTH(ram2);
        __kasan_shadow_end__ = .;
    } > ram2
}

/* elpekenin change: keep build id */
SECTIONS {
    .build_id : {
        __gnu_build_id__ = .;
        *(.note.gnu.build-id)
    } > flash1
}

INCLUDE rules_memory.ld

SECTIONS {
    .flash_end : {
        __flash_binary_end = .;
    } > flash1
}
