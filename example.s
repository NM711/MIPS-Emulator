.text
.globl _start
.set noreorder          # Disable assembler reordering
_start:

    addi $t6, $zero, -100
    # Set up condition (1 = true, 0 = false)
    addiu $t0, $zero, 1    # t0 = 0 + 1 (instead of li)
    
    # Test condition - use BEQ instead of BEQZ
    beq $t0, $zero, else_branch
    nop                    # Branch delay slot
    
    # True branch: add 1 + 2
    addiu $t1, $zero, 1    # t1 = 1
    addiu $t2, $zero, 2    # t2 = 2
    add $t3, $t1, $t2      # t3 = 1 + 2 = 3
    j end
    nop                    # Branch delay slot
    
else_branch:
    # False branch: add 5 + 6
    addiu $t1, $zero, 5    # t1 = 5
    addiu $t2, $zero, 6    # t2 = 6
    add $t3, $t1, $t2      # t3 = 5 + 6 = 11

end:
    add $t7, $t6, $t0
    # Infinite loop to end program
    j end
    nop                    # Branch delay slot
