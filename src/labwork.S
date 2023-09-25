  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #

.global hexasc
.global delay
.global time2string
.global enable_interrupt

.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #
.data
.align 2

.text

hexasc:

	move	$t0,$a0		# copy argument to temporary register
	sll	$t0,$t0,28
	srl	$t0,$t0,28	# shift all the way to the left then back to remove unwanted bits

	slti	$t1,$t0,10
	beq	$t1,$zero,overten # branch if argument was larger than 9
	nop

	addi	$v0,$t0,0x30
	jr 	$ra
	nop
overten:
	addi	$v0,$t0,0x37
	jr 	$ra
	nop

delay:	ble	$a0,0,exit	# exits if argument is 0 or lower
	nop
	addi	$a0,$a0,-1	# subtracts 1 ÄNDRADE DETTA FÖR MCB32, VAR SUBI
	move	$a1,$zero
	j	loop
	nop


loop:	addi	$a1,$a1,1	# adds 1 in every loop
	nop
	bne	$a1,5000,loop
	nop
	j	delay		# jumps back when for loop is done
	nop

exit:	jr	$ra
	nop


time2string:
	PUSH	$s0
	PUSH	$s1
	PUSH	$ra
	move 	$s0,$a0		# move arguments in order to call hexasc later
	move 	$s1,$a1
	move	$a0,$s1		# load time argument to a0 (argument for hexasc)
	jal	hexasc
	nop
	sb	$v0,4($s0)	# store second in s2

	move	$a0,$s1
	srl	$a0,$a0,4	# load argument into a0 and bit shift 4 bits
	jal	hexasc		# seconds*10
	nop
	sb	$v0,3($s0)

	li	$t1,0x3A	# colon
	sb	$t1,2($s0)

	move	$a0,$s1
	srl	$a0,$a0,8
	jal	hexasc
	nop
	sb	$v0,1($s0)

	move	$a0,$s1		#  minutes*10
	srl	$a0,$a0,12
	jal	hexasc
	nop
	sb	$v0,0($s0)


	lb	$t2,4($s0)
	beq	$t2,0x39,nine
	nop

	bne	$t2,0x39,notnine
	nop

	POP	$ra
	POP	$s1
	POP	$s0
	jr	$ra
	nop

nine:	li	$t1,0x4e
	sb	$t1,4($s0)

	li	$t1,0x49
	sb	$t1,5($s0)

	li	$t1,0x4e
	sb	$t1,6($s0)

	li	$t1,0x45
	sb	$t1,7($s0)

	li	$t1,0x00
	sb	$t1,8($s0)
	POP	$ra
	POP	$s1
	POP	$s0
	jr	$ra
	nop


notnine:
	li	$t1,0x00
	sb	$t1,5($s0)
	POP	$ra
	POP	$s1
	POP	$s0
	jr	$ra
	nop


enable_interrupt:
  ei
  jr  $ra
  nop
