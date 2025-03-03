echo "MEMORY_ADDRESS  SYMBOL_TYPE  SYMBOL_BIND  SECTION  SIZE  SYMBOL_NAME
--------------  -----------  -----------  -------  ----  ----------" > eval_ram_size_output.txt
arm-none-eabi-objdump -j .bss -j .data -t build/firmware.elf | sort -k5 -rn | while read -r line; do
  printf "%-14s %-10s %-10s %-7s %-4s %-s\n" $line
done >> eval_ram_size_output.txt
