.data
str_0: .asciiz "FizzBuzz\n"
str_1: .asciiz "Fizz\n"
str_2: .asciiz "Buzz\n"
str_3: .asciiz "\n"

.text
.globl main
main:
addi $sp,$sp,-256
sw $ra,252($sp)
sw $fp,248($sp)
addi $fp,$sp,256
li $t7, 1
sw $t7, -12($fp)
lw $t7, -12($fp)
Lfcond_0:
lw $t7, -12($fp)
li $t6, 100
slt $t5, $t6, $t7
xori $t5, $t5, 1
beq $t5, $zero, Lfend_1
nop
lw $t5, -12($fp)
li $t6, 15
div $t5, $t6
mfhi $t5
li $t6, 0
sub $t5, $t5, $t6
sltiu $t5, $t5, 1
beq $t5, $zero, Lelse_2
nop
la $t5, str_0
move $a0, $t5
jal print_str
nop
move $t5, $zero
j Lend_3
nop
Lelse_2:
lw $t5, -12($fp)
li $t6, 3
div $t5, $t6
mfhi $t5
li $t6, 0
sub $t5, $t5, $t6
sltiu $t5, $t5, 1
beq $t5, $zero, Lelse_4
nop
la $t5, str_1
move $a0, $t5
jal print_str
nop
move $t5, $zero
j Lend_5
nop
Lelse_4:
lw $t5, -12($fp)
li $t6, 5
div $t5, $t6
mfhi $t5
li $t6, 0
sub $t5, $t5, $t6
sltiu $t5, $t5, 1
beq $t5, $zero, Lelse_6
nop
la $t5, str_2
move $a0, $t5
jal print_str
nop
move $t5, $zero
j Lend_7
nop
Lelse_6:
lw $t5, -12($fp)
move $a0, $t5
jal print_int
nop
move $t5, $zero
la $t5, str_3
move $a0, $t5
jal print_str
nop
move $t5, $zero
Lend_7:
Lend_5:
Lend_3:
lw $t5, -12($fp)
li $t6, 1
add $t5, $t5, $t6
sw $t5, -12($fp)
j Lfcond_0
nop
Lfend_1:
li $t5, 0
move $v0, $t5
j __epilogue
nop
move $v0,$zero
__epilogue:
lw $ra,252($sp)
lw $fp,248($sp)
addi $sp,$sp,256
jr $ra
nop
