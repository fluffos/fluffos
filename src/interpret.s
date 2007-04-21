	.file	"interpret.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"*error_handler*"
.LC1:
	.string	"*unknown*"
.LC2:
	.string	"*invalid*"
.LC3:
	.string	"*lvalue*"
.LC4:
	.string	"*ref*"
.LC5:
	.string	"*lvalue_byte*"
.LC6:
	.string	"*lvalue_range*"
	.text
.globl type_name
	.type	type_name, @function
type_name:
.LFB52:
	movl	$0, %eax
	movl	$2, %edx
.L2:
	testl	%edi, %edx
	jne	.L17
	addl	%edx, %edx
	cmpl	$8, %eax
	je	.L6
	addl	$1, %eax
	jmp	.L2
.L6:
	movl	$.LC5, %eax
	cmpl	$1024, %edi
	je	.L5
	cmpl	$1024, %edi
	jg	.L15
	movl	$.LC2, %eax
	testl	%edi, %edi
	je	.L5
	cmpl	$1, %edi
	jne	.L8
	.p2align 4,,2
	jmp	.L10
.L15:
	movl	$.LC0, %eax
	cmpl	$4096, %edi
	.p2align 4,,2
	je	.L5
	movl	$.LC4, %eax
	cmpl	$16384, %edi
	je	.L5
	cmpl	$2048, %edi
	je	.L12
.L8:
	movl	$.LC1, %eax
	ret
.L10:
	movl	$.LC3, %eax
	ret
.L17:
	cltq
	movq	type_names(,%rax,8), %rax
	ret
.L12:
	movl	$.LC6, %eax
.L5:
	rep ; ret
.LFE52:
	.size	type_name, .-type_name
.globl assign_svalue_no_free
	.type	assign_svalue_no_free, @function
assign_svalue_no_free:
.LFB66:
	cmpw	$16, (%rsi)
	jne	.L19
	movq	8(%rsi), %rax
	testq	%rax, %rax
	je	.L21
	testb	$16, 2(%rax)
	je	.L19
.L21:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdi)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdi)
	ret
.L19:
	movq	(%rsi), %rax
	movq	%rax, (%rdi)
	movq	8(%rsi), %rax
	movq	%rax, 8(%rdi)
	movzwl	(%rdi), %eax
	testb	$32, %ah
	je	.L24
	cmpw	$8192, %ax
	je	.L24
	andb	$223, %ah
	movw	%ax, (%rdi)
.L24:
	movzwl	(%rsi), %eax
	cmpw	$4, %ax
	jne	.L27
	testb	$1, 2(%rsi)
	je	.L33
	movq	8(%rdi), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L30
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L30:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdi), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	ret
.L27:
	testl	$17272, %eax
	je	.L33
	movq	8(%rsi), %rax
	addw	$1, (%rax)
.L33:
	rep ; ret
.LFE66:
	.size	assign_svalue_no_free, .-assign_svalue_no_free
.globl pop_control_stack
	.type	pop_control_stack, @function
pop_control_stack:
.LFB82:
	movq	csp(%rip), %rax
	movq	16(%rax), %rdx
	movq	%rdx, current_object(%rip)
	movq	32(%rax), %rdx
	movq	%rdx, current_prog(%rip)
	movq	24(%rax), %rdx
	movq	%rdx, previous_ob(%rip)
	movzwl	72(%rax), %edx
	movw	%dx, caller_type(%rip)
	movq	48(%rax), %rdx
	movq	%rdx, pc(%rip)
	movq	56(%rax), %rdx
	movq	%rdx, fp(%rip)
	movl	64(%rax), %edx
	movl	%edx, function_index_offset(%rip)
	movl	68(%rax), %edx
	movl	%edx, variable_index_offset(%rip)
	subq	$80, %rax
	movq	%rax, csp(%rip)
	ret
.LFE82:
	.size	pop_control_stack, .-pop_control_stack
.globl remove_fake_frame
	.type	remove_fake_frame, @function
remove_fake_frame:
.LFB99:
	movq	csp(%rip), %rax
	movq	16(%rax), %rdx
	movq	%rdx, current_object(%rip)
	movq	32(%rax), %rdx
	movq	%rdx, current_prog(%rip)
	movq	24(%rax), %rdx
	movq	%rdx, previous_ob(%rip)
	movzwl	72(%rax), %edx
	movw	%dx, caller_type(%rip)
	movq	48(%rax), %rdx
	movq	%rdx, pc(%rip)
	movq	56(%rax), %rdx
	movq	%rdx, fp(%rip)
	movl	64(%rax), %edx
	movl	%edx, function_index_offset(%rip)
	movl	68(%rax), %edx
	movl	%edx, variable_index_offset(%rip)
	subq	$80, %rax
	movq	%rax, csp(%rip)
	ret
.LFE99:
	.size	remove_fake_frame, .-remove_fake_frame
	.type	ffbn_recurse, @function
ffbn_recurse:
.LFB105:
	pushq	%r15
.LCFI0:
	pushq	%r14
.LCFI1:
	pushq	%r13
.LCFI2:
	pushq	%r12
.LCFI3:
	pushq	%rbp
.LCFI4:
	pushq	%rbx
.LCFI5:
	subq	$8, %rsp
.LCFI6:
	movq	%rdi, %r13
	movq	%rsi, %r12
	movq	%rdx, (%rsp)
	movq	%rcx, %r15
	movzwl	142(%rdi), %eax
	movl	%eax, %edx
	subl	$1, %edx
	js	.L39
	movq	48(%rdi), %rdi
	movl	$0, %esi
.L41:
	leal	(%rsi,%rdx), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movq	(%rax,%rdi), %rax
	cmpq	%rax, %r12
	jae	.L42
	leal	-1(%rcx), %edx
	jmp	.L44
.L42:
	cmpq	%rax, %r12
	jbe	.L45
	leal	1(%rcx), %esi
	.p2align 4,,5
	jmp	.L44
.L45:
	movzwl	10(%r13), %eax
	leal	(%rcx,%rax), %esi
	movslq	%esi,%rdx
	movq	56(%r13), %rax
	movzwl	(%rax,%rdx,2), %eax
	testb	$10, %al
	jne	.L47
	movq	(%rsp), %rax
	movl	%ecx, (%rax)
	movl	%esi, (%r15)
	jmp	.L49
.L44:
	cmpl	%esi, %edx
	jge	.L41
.L39:
	movzwl	150(%r13), %ebp
	leal	-1(%rbp), %eax
	cltq
	movq	%rax, %rbx
	salq	$4, %rbx
	jmp	.L50
.L51:
	movq	%rbx, %r14
	movq	104(%r13), %rax
	movq	(%rax,%rbx), %rdi
	movq	%r15, %rcx
	movq	(%rsp), %rdx
	movq	%r12, %rsi
	call	ffbn_recurse
	movq	%rax, %rdx
	subq	$16, %rbx
	testq	%rax, %rax
	je	.L50
	movq	104(%r13), %rax
	movzwl	8(%r14,%rax), %eax
	addl	%eax, (%r15)
	movq	%rdx, %r13
	jmp	.L49
.L50:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L51
.L47:
	movl	$0, %r13d
.L49:
	movq	%r13, %rax
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE105:
	.size	ffbn_recurse, .-ffbn_recurse
	.type	ffbn_recurse2, @function
ffbn_recurse2:
.LFB106:
	pushq	%r15
.LCFI7:
	pushq	%r14
.LCFI8:
	pushq	%r13
.LCFI9:
	pushq	%r12
.LCFI10:
	pushq	%rbp
.LCFI11:
	pushq	%rbx
.LCFI12:
	subq	$24, %rsp
.LCFI13:
	movq	%rdi, %r14
	movq	%rsi, %r12
	movq	%rdx, 16(%rsp)
	movq	%rcx, %r15
	movq	%r8, 8(%rsp)
	movq	%r9, (%rsp)
	movzwl	142(%rdi), %eax
	movl	%eax, %edx
	subl	$1, %edx
	js	.L58
	movq	48(%rdi), %rdi
	movl	$0, %esi
.L60:
	leal	(%rsi,%rdx), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movq	(%rax,%rdi), %rax
	cmpq	%rax, %r12
	jae	.L61
	leal	-1(%rcx), %edx
	jmp	.L63
.L61:
	cmpq	%rax, %r12
	jbe	.L64
	leal	1(%rcx), %esi
	.p2align 4,,5
	jmp	.L63
.L64:
	movzwl	10(%r14), %eax
	leal	(%rcx,%rax), %esi
	movslq	%esi,%rdx
	movq	56(%r14), %rax
	movzwl	(%rax,%rdx,2), %eax
	testb	$10, %al
	jne	.L66
	movq	16(%rsp), %rax
	movl	%ecx, (%rax)
	movl	%esi, (%r15)
	movq	(%rsp), %rcx
	movl	$0, (%rcx)
	movq	8(%rsp), %rax
	movl	$0, (%rax)
	jmp	.L68
.L63:
	cmpl	%esi, %edx
	jge	.L60
.L58:
	movzwl	150(%r14), %ebp
	leal	-1(%rbp), %eax
	cltq
	movq	%rax, %rbx
	salq	$4, %rbx
	jmp	.L69
.L70:
	movq	%rbx, %r13
	movq	104(%r14), %rax
	movq	(%rax,%rbx), %rdi
	movq	(%rsp), %r9
	movq	8(%rsp), %r8
	movq	%r15, %rcx
	movq	16(%rsp), %rdx
	movq	%r12, %rsi
	call	ffbn_recurse2
	movq	%rax, %rdx
	subq	$16, %rbx
	testq	%rax, %rax
	je	.L69
	movq	104(%r14), %rax
	movzwl	8(%r13,%rax), %eax
	addl	%eax, (%r15)
	movq	104(%r14), %rax
	movzwl	8(%r13,%rax), %eax
	movq	8(%rsp), %rcx
	addl	%eax, (%rcx)
	movq	104(%r14), %rax
	movzwl	10(%r13,%rax), %eax
	movq	(%rsp), %rcx
	addl	%eax, (%rcx)
	movq	%rdx, %r14
	jmp	.L68
.L69:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L70
.L66:
	movl	$0, %r14d
.L68:
	movq	%r14, %rax
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE106:
	.size	ffbn_recurse2, .-ffbn_recurse2
.globl check_co_args
	.type	check_co_args, @function
check_co_args:
.LFB110:
	rep ; ret
.LFE110:
	.size	check_co_args, .-check_co_args
.globl function_name
	.type	function_name, @function
function_name:
.LFB116:
	movq	56(%rdi), %rcx
	movslq	%esi,%rax
	movzwl	(%rcx,%rax,2), %eax
	movl	%eax, %edx
	andl	$32767, %edx
	cmpw	$-1, %ax
	cmovle	%edx, %esi
	movslq	%esi,%rax
	testb	$1, (%rcx,%rax,2)
	je	.L81
.L91:
	movzwl	150(%rdi), %eax
	leal	-1(%rax), %ecx
	movl	$0, %r8d
	testl	%ecx, %ecx
	jle	.L85
	movq	104(%rdi), %r9
	movl	$0, %r8d
.L86:
	leal	1(%r8,%rcx), %eax
	movl	%eax, %edx
	sarl	%edx
	movslq	%edx,%rax
	salq	$4, %rax
	movzwl	8(%rax,%r9), %eax
	cmpl	%esi, %eax
	jg	.L87
	movl	%edx, %r8d
	jmp	.L89
.L87:
	leal	-1(%rdx), %ecx
.L89:
	cmpl	%r8d, %ecx
	jg	.L86
.L85:
	movslq	%r8d,%rax
	salq	$4, %rax
	addq	104(%rdi), %rax
	movzwl	8(%rax), %edx
	subl	%edx, %esi
	movq	(%rax), %rdi
	movslq	%esi,%rdx
	movq	56(%rdi), %rax
	testb	$1, (%rax,%rdx,2)
	jne	.L91
.L81:
	movzwl	10(%rdi), %eax
	subl	%eax, %esi
	movl	%esi, %eax
	cltq
	movq	48(%rdi), %rdx
	salq	$4, %rax
	movq	(%rax,%rdx), %rax
	ret
.LFE116:
	.size	function_name, .-function_name
	.type	get_trace_details, @function
get_trace_details:
.LFB117:
	movslq	%esi,%rsi
	salq	$4, %rsi
	addq	48(%rdi), %rsi
	movq	(%rsi), %rax
	movq	%rax, (%rdx)
	movzbl	10(%rsi), %eax
	movl	%eax, (%rcx)
	movzbl	11(%rsi), %eax
	movl	%eax, (%r8)
	ret
.LFE117:
	.size	get_trace_details, .-get_trace_details
.globl translate_absolute_line
	.type	translate_absolute_line, @function
translate_absolute_line:
.LFB120:
	movzwl	(%rsi), %eax
	cmpl	%eax, %edi
	jle	.L97
	movq	%rsi, %r8
.L99:
	subl	%eax, %edi
	addq	$4, %r8
	movzwl	(%r8), %eax
	cmpl	%edi, %eax
	jl	.L99
	movzwl	2(%r8), %r9d
	cmpq	%r8, %rsi
	jae	.L101
.L103:
	movzwl	2(%rsi), %eax
	cmpl	%r9d, %eax
	jne	.L104
	movzwl	(%rsi), %eax
	addl	%eax, %edi
.L104:
	addq	$4, %rsi
	cmpq	%r8, %rsi
	jb	.L103
.L101:
	movl	%edi, (%rcx)
	movl	%r9d, (%rdx)
	ret
.L97:
	movzwl	2(%rsi), %r9d
	jmp	.L101
.LFE120:
	.size	translate_absolute_line, .-translate_absolute_line
	.type	find_line, @function
find_line:
.LFB121:
	pushq	%rbp
.LCFI14:
	pushq	%rbx
.LCFI15:
	subq	$16, %rsp
.LCFI16:
	movq	%rsi, %rbx
	movq	%rdx, %rbp
	movq	$0, (%rdx)
	movl	$0, (%rcx)
	movl	$1, %eax
	testq	%rsi, %rsi
	je	.L113
	movl	$2, %eax
	cmpq	$fake_prog, %rsi
	je	.L113
	movq	24(%rsi), %rsi
	movl	$4, %eax
	testq	%rsi, %rsi
	je	.L113
	movl	%edi, %edx
	subl	16(%rbx), %edx
	movzbl	(%rsi), %eax
	cmpl	%eax, %edx
	jle	.L118
.L121:
	subl	%eax, %edx
	addq	$5, %rsi
	movzbl	(%rsi), %eax
	cmpl	%edx, %eax
	jl	.L121
.L118:
	movzbl	1(%rsi), %eax
	movb	%al, 12(%rsp)
	movzbl	2(%rsi), %eax
	movb	%al, 13(%rsp)
	movzbl	3(%rsi), %eax
	movb	%al, 14(%rsp)
	movzbl	4(%rsi), %eax
	movb	%al, 15(%rsp)
	movzbl	5(%rsi), %eax
	movb	%al, 16(%rsp)
	movzbl	6(%rsi), %eax
	movb	%al, 17(%rsp)
	movzbl	7(%rsi), %eax
	movb	%al, 18(%rsp)
	movzbl	8(%rsi), %eax
	movb	%al, 19(%rsp)
	leaq	8(%rsp), %rdx
	movq	32(%rbx), %rsi
	addq	$4, %rsi
	movl	12(%rsp), %edi
	call	translate_absolute_line
	movslq	8(%rsp),%rdx
	movq	80(%rbx), %rax
	movq	-8(%rax,%rdx,8), %rax
	movq	%rax, (%rbp)
	movl	$0, %eax
.L113:
	addq	$16, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE121:
	.size	find_line, .-find_line
.globl get_line_number_info
	.type	get_line_number_info, @function
get_line_number_info:
.LFB123:
	pushq	%rbx
.LCFI17:
	movq	%rdi, %rbx
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	movq	current_prog(%rip), %rsi
	movq	pc(%rip), %rdi
	call	find_line
	cmpq	$0, (%rbx)
	jne	.L126
	movq	current_prog(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, (%rbx)
.L126:
	popq	%rbx
	ret
.LFE123:
	.size	get_line_number_info, .-get_line_number_info
.globl strpref
	.type	strpref, @function
strpref:
.LFB134:
	jmp	.L128
.L129:
	cmpb	(%rsi), %al
	je	.L130
	movl	$0, %eax
	ret
.L130:
	addq	$1, %rdi
	addq	$1, %rsi
.L128:
	movzbl	(%rdi), %eax
	testb	%al, %al
	jne	.L129
	movl	$1, %eax
	ret
.LFE134:
	.size	strpref, .-strpref
.globl save_context
	.type	save_context, @function
save_context:
.LFB136:
	movl	$0, %eax
	cmpq	$control_stack+11920, csp(%rip)
	je	.L138
	movq	sp(%rip), %rax
	movq	%rax, 208(%rdi)
	movq	csp(%rip), %rax
	movq	%rax, 200(%rdi)
	movq	cgsp(%rip), %rax
	movq	%rax, 216(%rdi)
	movq	current_error_context(%rip), %rax
	movq	%rax, 224(%rdi)
	movq	%rdi, current_error_context(%rip)
	movl	$1, %eax
.L138:
	rep ; ret
.LFE136:
	.size	save_context, .-save_context
.globl pop_context
	.type	pop_context, @function
pop_context:
.LFB137:
	movq	224(%rdi), %rax
	movq	%rax, current_error_context(%rip)
	ret
.LFE137:
	.size	pop_context, .-pop_context
	.section	.rodata.str1.1
.LC7:
	.string	"remove_object_from_stack"
	.text
.globl remove_object_from_stack
	.type	remove_object_from_stack, @function
remove_object_from_stack:
.LFB133:
	pushq	%rbp
.LCFI18:
	pushq	%rbx
.LCFI19:
	subq	$8, %rsp
.LCFI20:
	movq	%rdi, %rbp
	cmpq	$start_of_stack, sp(%rip)
	jb	.L149
	movl	$start_of_stack+16, %ebx
.L145:
	cmpw	$16, -16(%rbx)
	jne	.L146
	cmpq	%rbp, -8(%rbx)
	jne	.L146
	movl	$.LC7, %esi
	movq	%rbp, %rdi
	call	free_object
	movw	$2, -16(%rbx)
	movq	$0, -8(%rbx)
.L146:
	movq	%rbx, %rax
	addq	$16, %rbx
	cmpq	sp(%rip), %rax
	jbe	.L145
.L149:
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE133:
	.size	remove_object_from_stack, .-remove_object_from_stack
	.section	.rodata.str1.1
.LC8:
	.string	"Recent instruction trace:\n"
.LC9:
	.string	""
.LC10:
	.string	"%d"
.LC11:
	.string	"%6x: %3d %8s %-25s (%d)\n"
	.text
.globl last_instructions
	.type	last_instructions, @function
last_instructions:
.LFB132:
	pushq	%r14
.LCFI21:
	pushq	%r13
.LCFI22:
	pushq	%r12
.LCFI23:
	pushq	%rbp
.LCFI24:
	pushq	%rbx
.LCFI25:
	subq	$16, %rsp
.LCFI26:
	movl	$.LC8, %edi
	movl	$0, %eax
	call	debug_message
	movl	last(%rip), %ebp
	movabsq	$-8608480567731124087, %r14
.L152:
	movslq	%ebp,%rbx
	movl	previous_instruction(,%rbx,4), %edi
	testl	%edi, %edi
	je	.L153
	movl	stack_size(,%rbx,4), %r13d
	call	query_instr_name
	movq	%rax, %r12
	movq	previous_pc(,%rbx,8), %rsi
	leal	1(%rbp), %eax
	movslq	%eax,%rdi
	movq	%rdi, %rax
	mulq	%r14
	shrq	$5, %rdx
	leaq	0(,%rdx,4), %rcx
	salq	$6, %rdx
	subq	%rcx, %rdx
	movq	%rdi, %rax
	subq	%rdx, %rax
	cltq
	movq	previous_pc(,%rax,8), %rax
	subq	%rsi, %rax
	movl	$.LC9, %ecx
	cmpq	$1, %rax
	jle	.L157
	cmpq	$2, %rax
	jne	.L158
	movsbl	1(%rsi),%edx
	movl	$.LC10, %esi
	movl	$buff.12957, %edi
	movl	$0, %eax
	call	sprintf
	movl	$buff.12957, %ecx
	jmp	.L157
.L158:
	cmpq	$3, %rax
	jne	.L160
	movzbl	1(%rsi), %eax
	movb	%al, 12(%rsp)
	movzbl	2(%rsi), %eax
	movb	%al, 13(%rsp)
	movswl	12(%rsp),%edx
	movl	$.LC10, %esi
	movl	$buff.12957, %edi
	movl	$0, %eax
	call	sprintf
	movl	$buff.12957, %ecx
	jmp	.L157
.L160:
	movl	$.LC9, %ecx
	cmpq	$5, %rax
	jne	.L157
	movzbl	1(%rsi), %eax
	movb	%al, 12(%rsp)
	movzbl	2(%rsi), %eax
	movb	%al, 13(%rsp)
	movzbl	3(%rsi), %eax
	movb	%al, 14(%rsp)
	movzbl	4(%rsi), %eax
	movb	%al, 15(%rsp)
	movzbl	5(%rsi), %eax
	movb	%al, 16(%rsp)
	movzbl	6(%rsi), %eax
	movb	%al, 17(%rsp)
	movzbl	7(%rsi), %eax
	movb	%al, 18(%rsp)
	movzbl	8(%rsi), %eax
	movb	%al, 19(%rsp)
	movl	12(%rsp), %edx
	movl	$.LC10, %esi
	movl	$buff.12957, %edi
	movl	$0, %eax
	call	sprintf
	movl	$buff.12957, %ecx
.L157:
	movslq	%ebp,%rax
	movl	previous_instruction(,%rax,4), %edx
	movq	previous_pc(,%rax,8), %rsi
	leal	1(%r13), %r9d
	movq	%r12, %r8
	movl	$.LC11, %edi
	movl	$0, %eax
	call	debug_message
.L153:
	leal	1(%rbp), %eax
	movslq	%eax,%rsi
	movq	%rsi, %rax
	mulq	%r14
	shrq	$5, %rdx
	leaq	0(,%rdx,4), %rcx
	salq	$6, %rdx
	subq	%rcx, %rdx
	movq	%rsi, %rax
	subq	%rdx, %rax
	movl	%eax, %ebp
	cmpl	last(%rip), %eax
	jne	.L152
	addq	$16, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret
.LFE132:
	.size	last_instructions, .-last_instructions
	.section	.rodata.str1.1
.LC12:
	.string	"v2.7"
.LC13:
	.string	"FluffOS %s"
	.text
.globl get_version
	.type	get_version, @function
get_version:
.LFB50:
	subq	$8, %rsp
.LCFI27:
	movl	$.LC12, %edx
	movl	$.LC13, %esi
	movl	$0, %eax
	call	sprintf
	addq	$8, %rsp
	ret
.LFE50:
	.size	get_version, .-get_version
	.section	.rodata.str1.1
.LC14:
	.string	"Too deep recursion.\n"
	.text
.globl setup_fake_frame
	.type	setup_fake_frame, @function
setup_fake_frame:
.LFB98:
	pushq	%rbx
.LCFI28:
	movq	%rdi, %rbx
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L171
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L171:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	csp(%rip), %rax
	movw	$7, (%rax)
	movq	csp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movq	$fake_program, pc(%rip)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movw	$64, caller_type(%rip)
	movq	csp(%rip), %rax
	movl	$0, 40(%rax)
	movq	$fake_prog, current_prog(%rip)
	movq	current_object(%rip), %rax
	movq	%rax, previous_ob(%rip)
	movq	8(%rbx), %rax
	movq	%rax, current_object(%rip)
	popq	%rbx
	ret
.LFE98:
	.size	setup_fake_frame, .-setup_fake_frame
	.section	.rodata.str1.1
.LC15:
	.string	"stack overflow"
	.text
.globl push_constant_string
	.type	push_constant_string, @function
push_constant_string:
.LFB93:
	pushq	%rbx
.LCFI29:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L175
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L175:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$4, (%rax)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE93:
	.size	push_constant_string, .-push_constant_string
.globl push_malloced_string
	.type	push_malloced_string, @function
push_malloced_string:
.LFB91:
	pushq	%rbx
.LCFI30:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L179
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L179:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$4, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	movw	$1, 2(%rax)
	popq	%rbx
	ret
.LFE91:
	.size	push_malloced_string, .-push_malloced_string
.globl push_refed_class
	.type	push_refed_class, @function
push_refed_class:
.LFB90:
	pushq	%rbx
.LCFI31:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L183
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L183:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$512, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE90:
	.size	push_refed_class, .-push_refed_class
.globl push_class
	.type	push_class, @function
push_class:
.LFB89:
	pushq	%rbx
.LCFI32:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L187
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L187:
	addq	$16, sp(%rip)
	addw	$1, (%rbx)
	movq	sp(%rip), %rax
	movw	$512, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE89:
	.size	push_class, .-push_class
.globl push_refed_mapping
	.type	push_refed_mapping, @function
push_refed_mapping:
.LFB88:
	pushq	%rbx
.LCFI33:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L191
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L191:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$32, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE88:
	.size	push_refed_mapping, .-push_refed_mapping
.globl push_mapping
	.type	push_mapping, @function
push_mapping:
.LFB87:
	pushq	%rbx
.LCFI34:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L195
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L195:
	addq	$16, sp(%rip)
	addw	$1, (%rbx)
	movq	sp(%rip), %rax
	movw	$32, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE87:
	.size	push_mapping, .-push_mapping
.globl push_refed_buffer
	.type	push_refed_buffer, @function
push_refed_buffer:
.LFB86:
	pushq	%rbx
.LCFI35:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L199
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L199:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$256, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE86:
	.size	push_refed_buffer, .-push_refed_buffer
.globl push_buffer
	.type	push_buffer, @function
push_buffer:
.LFB85:
	pushq	%rbx
.LCFI36:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L203
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L203:
	addq	$16, sp(%rip)
	addw	$1, (%rbx)
	movq	sp(%rip), %rax
	movw	$256, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE85:
	.size	push_buffer, .-push_buffer
.globl push_refed_array
	.type	push_refed_array, @function
push_refed_array:
.LFB84:
	pushq	%rbx
.LCFI37:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L207
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L207:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$8, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE84:
	.size	push_refed_array, .-push_refed_array
.globl push_array
	.type	push_array, @function
push_array:
.LFB83:
	pushq	%rbx
.LCFI38:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L211
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L211:
	addq	$16, sp(%rip)
	addw	$1, (%rbx)
	movq	sp(%rip), %rax
	movw	$8, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE83:
	.size	push_array, .-push_array
.globl push_control_stack
	.type	push_control_stack, @function
push_control_stack:
.LFB81:
	pushq	%rbx
.LCFI39:
	movl	%edi, %ebx
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L215
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L215:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	%bx, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	popq	%rbx
	ret
.LFE81:
	.size	push_control_stack, .-push_control_stack
	.section	.rodata.str1.1
.LC16:
	.string	"Bad Argument %d to %s()\n"
	.text
.globl bad_arg
	.type	bad_arg, @function
bad_arg:
.LFB79:
	pushq	%rbx
.LCFI40:
	movl	%edi, %ebx
	movl	%esi, %edi
	call	query_instr_name
	movq	%rax, %rdx
	movl	%ebx, %esi
	movl	$.LC16, %edi
	movl	$0, %eax
	call	error
	popq	%rbx
	ret
.LFE79:
	.size	bad_arg, .-bad_arg
.globl push_undefined
	.type	push_undefined, @function
push_undefined:
.LFB55:
	subq	$8, %rsp
.LCFI41:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L221
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L221:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	addq	$8, %rsp
	ret
.LFE55:
	.size	push_undefined, .-push_undefined
.globl push_real
	.type	push_real, @function
push_real:
.LFB54:
	pushq	%rbx
.LCFI42:
	subq	$16, %rsp
.LCFI43:
	movss	%xmm0, 12(%rsp)
	movl	12(%rsp), %ebx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L225
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L225:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$128, (%rax)
	movq	sp(%rip), %rax
	movl	%ebx, 8(%rax)
	addq	$16, %rsp
	popq	%rbx
	ret
.LFE54:
	.size	push_real, .-push_real
.globl push_number
	.type	push_number, @function
push_number:
.LFB53:
	pushq	%rbx
.LCFI44:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L229
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L229:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$2, (%rax)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	popq	%rbx
	ret
.LFE53:
	.size	push_number, .-push_number
.globl push_object
	.type	push_object, @function
push_object:
.LFB51:
	pushq	%rbx
.LCFI45:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L233
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L233:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	testq	%rbx, %rbx
	je	.L235
	testb	$16, 2(%rbx)
	je	.L237
.L235:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L239
.L237:
	movw	$16, (%rdx)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	addw	$1, (%rbx)
.L239:
	popq	%rbx
	ret
.LFE51:
	.size	push_object, .-push_object
.globl copy_and_push_string
	.type	copy_and_push_string, @function
copy_and_push_string:
.LFB57:
	movq	%rbx, -16(%rsp)
.LCFI46:
	movq	%rbp, -8(%rsp)
.LCFI47:
	subq	$24, %rsp
.LCFI48:
	movq	%rdi, %rbp
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L241
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L241:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$4, (%rax)
	movq	sp(%rip), %rax
	movw	$1, 2(%rax)
	movq	sp(%rip), %rbx
	movq	%rbp, %rdi
	call	int_string_copy
	movq	%rax, 8(%rbx)
	movq	8(%rsp), %rbx
	movq	16(%rsp), %rbp
	addq	$24, %rsp
	ret
.LFE57:
	.size	copy_and_push_string, .-copy_and_push_string
.globl make_ref
	.type	make_ref, @function
make_ref:
.LFB49:
	subq	$8, %rsp
.LCFI49:
	movl	$56, %edi
	call	xalloc
	movq	%rax, %rdx
	movq	global_ref_list(%rip), %rax
	movq	%rax, 8(%rdx)
	movq	$0, 16(%rdx)
	testq	%rax, %rax
	je	.L245
	movq	%rdx, 16(%rax)
.L245:
	movq	%rdx, global_ref_list(%rip)
	movq	csp(%rip), %rax
	movq	%rax, 24(%rdx)
	movw	$1, (%rdx)
	movq	%rdx, %rax
	addq	$8, %rsp
	ret
.LFE49:
	.size	make_ref, .-make_ref
.globl transfer_push_some_svalues
	.type	transfer_push_some_svalues, @function
transfer_push_some_svalues:
.LFB70:
	movq	%rbx, -24(%rsp)
.LCFI50:
	movq	%rbp, -16(%rsp)
.LCFI51:
	movq	%r12, -8(%rsp)
.LCFI52:
	subq	$24, %rsp
.LCFI53:
	movq	%rdi, %r12
	movslq	%esi,%rsi
	movq	%rsi, %rbx
	salq	$4, %rbx
	movq	%rbx, %rbp
	movq	%rbx, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L249
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L249:
	movq	sp(%rip), %rdi
	addq	$16, %rdi
	movq	%rbx, %rdx
	movq	%r12, %rsi
	call	memcpy
	addq	%rbp, sp(%rip)
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	16(%rsp), %r12
	addq	$24, %rsp
	ret
.LFE70:
	.size	transfer_push_some_svalues, .-transfer_push_some_svalues
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
.LC18:
	.long	1092616192
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC19:
	.long	2147483648
	.long	0
	.long	0
	.long	0
	.text
	.type	_strtof, @function
_strtof:
.LFB135:
	pushq	%rbp
.LCFI54:
	pushq	%rbx
.LCFI55:
	subq	$8, %rsp
.LCFI56:
	movq	%rdi, %rbx
	movq	%rsi, %rbp
	call	__ctype_b_loc
	movq	(%rax), %rdi
	movq	%rbx, %rsi
.L253:
	movzbl	(%rsi), %edx
	movsbl	%dl,%ecx
	addq	$1, %rsi
	movslq	%ecx,%rax
	testb	$32, 1(%rdi,%rax,2)
	jne	.L253
	cmpb	$45, %dl
	jne	.L255
	movsbl	(%rsi),%ecx
	addq	$1, %rsi
	movl	$1, %r9d
	jmp	.L257
.L255:
	movl	$0, %r9d
	cmpb	$43, %dl
	jne	.L257
	movsbl	(%rsi),%ecx
	leaq	1(%rsi), %rsi
	movl	$0, %r9d
.L257:
	xorps	%xmm2, %xmm2
	movl	$0, %r8d
	movl	$1, %edx
	movss	.LC18(%rip), %xmm3
.L260:
	movslq	%ecx,%rax
	testb	$8, 1(%rdi,%rax,2)
	je	.L261
	leal	-48(%rcx), %eax
	cmpl	$1, %edx
	jne	.L277
	jmp	.L263
.L261:
	cmpl	$1, %edx
	.p2align 4,,5
	jne	.L265
	cmpl	$46, %ecx
	.p2align 4,,7
	jne	.L265
	movb	$10, %dl
	.p2align 4,,5
	jmp	.L267
.L263:
	movaps	%xmm2, %xmm1
	mulss	%xmm3, %xmm1
	cvtsi2ss	%eax, %xmm0
	movaps	%xmm0, %xmm2
	addss	%xmm1, %xmm2
	movl	$1, %r8d
	jmp	.L267
.L277:
	cvtsi2ss	%eax, %xmm0
	cvtsi2ss	%edx, %xmm1
	divss	%xmm1, %xmm0
	addss	%xmm0, %xmm2
	leal	(%rdx,%rdx,4), %eax
	leal	(%rax,%rax), %edx
	movl	$1, %r8d
.L267:
	movsbl	(%rsi),%ecx
	addq	$1, %rsi
	jmp	.L260
.L265:
	testl	%r9d, %r9d
	je	.L268
	xorps	.LC19(%rip), %xmm2
.L268:
	testq	%rbp, %rbp
	je	.L270
	leaq	-1(%rsi), %rax
	testl	%r8d, %r8d
	cmovne	%rax, %rbx
	movq	%rbx, (%rbp)
.L270:
	movaps	%xmm2, %xmm0
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE135:
	.size	_strtof, .-_strtof
	.section	.rodata.str1.1
.LC20:
	.string	"Object: "
.LC21:
	.string	", Program: "
.LC22:
	.string	"\n   in "
.LC23:
	.string	"() at "
.LC24:
	.string	"\n"
	.text
	.type	dump_trace_line, @function
dump_trace_line:
.LFB125:
	pushq	%r13
.LCFI57:
	pushq	%r12
.LCFI58:
	pushq	%rbp
.LCFI59:
	pushq	%rbx
.LCFI60:
	subq	$264, %rsp
.LCFI61:
	movq	%rdi, %r12
	movq	%rsi, %rbp
	movq	%rdx, %rbx
	movq	%rcx, %r13
	movq	%rsp, %rdi
	movl	$.LC20, %edx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	cmpb	$60, (%rbx)
	je	.L279
	leaq	256(%rsp), %rax
	cmpq	%rax, %rdi
	jae	.L279
	movb	$47, (%rdi)
	addq	$1, %rdi
.L279:
	movq	%rbx, %rdx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	movl	$.LC21, %edx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	cmpb	$60, (%rbp)
	je	.L281
	leaq	256(%rsp), %rax
	cmpq	%rax, %rdi
	jae	.L281
	movb	$47, (%rdi)
	addq	$1, %rdi
.L281:
	movq	%rbp, %rdx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	movl	$.LC22, %edx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	movq	%r12, %rdx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	movl	$.LC23, %edx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	movq	%r13, %rdx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rax, %rdi
	movl	$.LC24, %edx
	leaq	256(%rsp), %rsi
	call	strput
	movq	%rsp, %rdi
	movl	$0, %eax
	call	debug_message
	addq	$264, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	ret
.LFE125:
	.size	dump_trace_line, .-dump_trace_line
	.section	.rodata.str1.1
.LC25:
	.string	"/%s:%d"
	.text
.globl get_line_number
	.type	get_line_number, @function
get_line_number:
.LFB124:
	pushq	%rbx
.LCFI62:
	subq	$16, %rsp
.LCFI63:
	movq	%rsi, %rbx
	leaq	4(%rsp), %rcx
	leaq	8(%rsp), %rdx
	call	find_line
	cmpl	$5, %eax
	ja	.L285
	mov	%eax, %eax
	jmp	*.L291(,%rax,8)
	.section	.rodata
	.align 8
	.align 4
.L291:
	.quad	.L285
	.quad	.L286
	.quad	.L287
	.quad	.L288
	.quad	.L289
	.quad	.L290
	.text
.L286:
	movl	$544173608, buf.12319(%rip)
	movl	$1735357040, buf.12319+4(%rip)
	movl	$695034226, buf.12319+8(%rip)
	movb	$0, buf.12319+12(%rip)
	jmp	.L292
.L287:
	movb	$0, buf.12319(%rip)
	jmp	.L292
.L288:
	movl	$1836016424, buf.12319(%rip)
	movl	$1701603696, buf.12319+4(%rip)
	movl	$1919950948, buf.12319+8(%rip)
	movl	$1634887535, buf.12319+12(%rip)
	movw	$10605, buf.12319+16(%rip)
	movb	$0, buf.12319+18(%rip)
	jmp	.L292
.L289:
	movl	$544173608, buf.12319(%rip)
	movl	$1701734764, buf.12319+4(%rip)
	movl	$1836412448, buf.12319+8(%rip)
	movl	$1936876898, buf.12319+12(%rip)
	movw	$41, buf.12319+16(%rip)
	jmp	.L292
.L290:
	movl	$1668180264, buf.12319(%rip)
	movl	$1701082476, buf.12319+4(%rip)
	movl	$1869881459, buf.12319+8(%rip)
	movl	$1701060719, buf.12319+12(%rip)
	movl	$2715749, buf.12319+16(%rip)
	jmp	.L292
.L285:
	cmpq	$0, 8(%rsp)
	jne	.L293
	movq	(%rbx), %rax
	movq	%rax, 8(%rsp)
.L293:
	movl	4(%rsp), %ecx
	movq	8(%rsp), %rdx
	movl	$.LC25, %esi
	movl	$buf.12319, %edi
	movl	$0, %eax
	call	sprintf
.L292:
	movl	$buf.12319, %eax
	addq	$16, %rsp
	popq	%rbx
	ret
.LFE124:
	.size	get_line_number, .-get_line_number
.globl get_line_number_if_any
	.type	get_line_number_if_any, @function
get_line_number_if_any:
.LFB128:
	subq	$8, %rsp
.LCFI64:
	movq	current_prog(%rip), %rsi
	movl	$0, %eax
	testq	%rsi, %rsi
	je	.L299
	movq	pc(%rip), %rdi
	call	get_line_number
.L299:
	addq	$8, %rsp
	ret
.LFE128:
	.size	get_line_number_if_any, .-get_line_number_if_any
.globl push_shared_string
	.type	push_shared_string, @function
push_shared_string:
.LFB92:
	pushq	%rbx
.LCFI65:
	movq	%rdi, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L302
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L302:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$4, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	movw	$3, 2(%rax)
	movq	%rbx, %rdi
	call	ref_string
	popq	%rbx
	ret
.LFE92:
	.size	push_shared_string, .-push_shared_string
.globl share_and_push_string
	.type	share_and_push_string, @function
share_and_push_string:
.LFB58:
	movq	%rbx, -16(%rsp)
.LCFI66:
	movq	%rbp, -8(%rsp)
.LCFI67:
	subq	$24, %rsp
.LCFI68:
	movq	%rdi, %rbp
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L306
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L306:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$4, (%rax)
	movq	sp(%rip), %rax
	movw	$3, 2(%rax)
	movq	sp(%rip), %rbx
	movq	%rbp, %rdi
	call	make_shared_string
	movq	%rax, 8(%rbx)
	movq	8(%rsp), %rbx
	movq	16(%rsp), %rbp
	addq	$24, %rsp
	ret
.LFE58:
	.size	share_and_push_string, .-share_and_push_string
.globl find_function_by_name
	.type	find_function_by_name, @function
find_function_by_name:
.LFB107:
	movq	%rbx, -24(%rsp)
.LCFI69:
	movq	%rbp, -16(%rsp)
.LCFI70:
	movq	%r12, -8(%rsp)
.LCFI71:
	subq	$24, %rsp
.LCFI72:
	movq	%rdi, %rbp
	movq	%rsi, %rdi
	movq	%rdx, %r12
	movq	%rcx, %rbx
	call	findstring
	movl	$0, %edx
	testq	%rax, %rax
	je	.L312
	movq	40(%rbp), %rdi
	movq	%rbx, %rcx
	movq	%r12, %rdx
	movq	%rax, %rsi
	call	ffbn_recurse
	movq	%rax, %rdx
.L312:
	movq	%rdx, %rax
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	16(%rsp), %r12
	addq	$24, %rsp
	ret
.LFE107:
	.size	find_function_by_name, .-find_function_by_name
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC26:
	.string	"Bad argument %d in call to %s() in %s\nExpected: %s Got %s.\n"
	.section	.rodata.str1.1
.LC27:
	.string	"driver"
	.text
.globl check_co_args2
	.type	check_co_args2, @function
check_co_args2:
.LFB109:
	pushq	%r15
.LCFI73:
	pushq	%r14
.LCFI74:
	pushq	%r13
.LCFI75:
	pushq	%r12
.LCFI76:
	pushq	%rbp
.LCFI77:
	pushq	%rbx
.LCFI78:
	subq	$1080, %rsp
.LCFI79:
	movq	%rdi, %r15
	movq	%rdx, 24(%rsp)
	movq	%rcx, 16(%rsp)
	leal	-1(%rsi), %eax
	cltq
	movq	%rax, %r13
	salq	$4, %r13
	movl	%esi, %r12d
	movl	$0, %r14d
.L315:
	subl	$1, %r12d
	movzwl	(%r15,%r14,2), %edi
	call	convert_type
	movl	%eax, %ebp
	cmpl	$17406, %eax
	je	.L316
	movq	sp(%rip), %rax
	subq	%r13, %rax
	movzwl	(%rax), %edx
	movswl	%dx,%edi
	cmpl	%edi, %ebp
	je	.L316
	cmpw	$2, %dx
	jne	.L319
	cmpq	$0, 8(%rax)
	je	.L316
.L319:
	.p2align 4,,7
	call	type_name
	movq	%rax, %rbx
	movl	%ebp, %edi
	call	type_name
	leal	1(%r14), %edx
	leaq	32(%rsp), %rbp
	movq	%rbx, (%rsp)
	movq	%rax, %r9
	movq	16(%rsp), %r8
	movq	24(%rsp), %rcx
	movl	$.LC26, %esi
	movq	%rbp, %rdi
	movl	$0, %eax
	call	sprintf
	movq	current_prog(%rip), %rsi
	testq	%rsi, %rsi
	je	.L321
	leaq	1068(%rsp), %rcx
	leaq	1056(%rsp), %rdx
	movq	pc(%rip), %rdi
	call	find_line
	movl	1068(%rsp), %esi
	movq	1056(%rsp), %rdi
	movl	$1, %ecx
	movq	%rbp, %rdx
	call	smart_log
	jmp	.L316
.L321:
	movl	$1, %ecx
	leaq	32(%rsp), %rdx
	movl	$0, %esi
	movl	$.LC27, %edi
	call	smart_log
.L316:
	addq	$1, %r14
	subq	$16, %r13
	testl	%r12d, %r12d
	jne	.L315
	addq	$1080, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE109:
	.size	check_co_args2, .-check_co_args2
	.section	.rodata.str1.1
.LC28:
	.string	"function>"
	.text
.globl add_slash
	.type	add_slash, @function
add_slash:
.LFB65:
	pushq	%rbp
.LCFI80:
	pushq	%rbx
.LCFI81:
	subq	$8, %rsp
.LCFI82:
	movq	%rdi, %rbp
	cmpb	$60, (%rdi)
	jne	.L328
	leaq	1(%rdi), %rsi
	movl	$.LC28, %edi
	movl	$10, %ecx
	cld
	repz
	cmpsb
	seta	%dl
	setb	%al
	cmpb	%al, %dl
	jne	.L328
	movq	%rbp, %rdi
	call	int_string_copy
	movq	%rax, %rbx
	jmp	.L331
.L328:
	movq	%rbp, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	movl	%ecx, %edi
	notl	%edi
	call	int_new_string
	movq	%rax, %rbx
	movb	$47, (%rax)
	leaq	1(%rax), %rdi
	movq	%rbp, %rsi
	call	strcpy
.L331:
	movq	%rbx, %rax
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE65:
	.size	add_slash, .-add_slash
	.section	.rodata.str1.1
.LC29:
	.string	"()"
	.section	.rodata.str1.8
	.align 8
.LC30:
	.string	"Bad argument %d to %s%s\nExpected: "
	.section	.rodata.str1.1
.LC31:
	.string	" or "
.LC32:
	.string	" Got: "
.LC33:
	.string	".\n"
	.text
.globl bad_argument
	.type	bad_argument, @function
bad_argument:
.LFB80:
	pushq	%r15
.LCFI83:
	pushq	%r14
.LCFI84:
	pushq	%r13
.LCFI85:
	pushq	%r12
.LCFI86:
	pushq	%rbp
.LCFI87:
	pushq	%rbx
.LCFI88:
	subq	$24, %rsp
.LCFI89:
	movq	%rdi, %r15
	movl	%esi, %ebp
	movl	%edx, %r13d
	movl	%ecx, %r12d
	movq	%rsp, %rdi
	call	outbuf_zero
	cmpl	$119, %r12d
	movl	$.LC29, %ebx
	movl	$.LC9, %eax
	cmovle	%rax, %rbx
	movl	%r12d, %edi
	call	query_instr_name
	movq	%rax, %rcx
	movq	%rsp, %rdi
	movq	%rbx, %r8
	movl	%r13d, %edx
	movl	$.LC30, %esi
	movl	$0, %eax
	call	outbuf_addv
	movl	$0, %r13d
	movl	$2, %r12d
	movl	$0, %ebx
.L337:
	testl	%ebp, %r12d
	je	.L338
	testl	%r13d, %r13d
	jne	.L340
	movb	$1, %r13b
	jmp	.L342
.L340:
	movl	$.LC31, %esi
	movq	%rsp, %rdi
	call	outbuf_add
.L342:
	movq	type_names(,%rbx,8), %rsi
	movq	%rsp, %rdi
	call	outbuf_add
.L338:
	addl	%r12d, %r12d
	addq	$1, %rbx
	cmpq	$9, %rbx
	jne	.L337
	movl	$.LC32, %esi
	movq	%rsp, %rdi
	call	outbuf_add
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	movq	%rsp, %rsi
	movq	%r15, %rdi
	call	svalue_to_string
	movl	$.LC33, %esi
	movq	%rsp, %rdi
	call	outbuf_add
	movq	%rsp, %rdi
	call	outbuf_fix
	movq	8(%rsp), %rdi
	call	error_needs_free
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE80:
	.size	bad_argument, .-bad_argument
.globl process_efun_callback
	.type	process_efun_callback, @function
process_efun_callback:
.LFB62:
	movq	%rbx, -48(%rsp)
.LCFI90:
	movq	%rbp, -40(%rsp)
.LCFI91:
	movq	%r12, -32(%rsp)
.LCFI92:
	movq	%r13, -24(%rsp)
.LCFI93:
	movq	%r14, -16(%rsp)
.LCFI94:
	movq	%r15, -8(%rsp)
.LCFI95:
	subq	$56, %rsp
.LCFI96:
	movl	%edi, %r13d
	movq	%rsi, %rbx
	movl	%edx, %r14d
	movl	st_num_arg(%rip), %r12d
	movslq	%r12d,%rax
	salq	$4, %rax
	movq	sp(%rip), %rdx
	subq	%rax, %rdx
	movslq	%edi,%rax
	salq	$4, %rax
	addq	%rax, %rdx
	leaq	16(%rdx), %rbp
	cmpw	$64, 16(%rdx)
	jne	.L348
	movq	8(%rbp), %rax
	movq	%rax, 8(%rsi)
	movq	$0, (%rsi)
	movl	%r12d, %eax
	subl	%edi, %eax
	subl	$1, %eax
	movl	%eax, 16(%rsi)
	leaq	32(%rdx), %rax
	movq	%rax, 24(%rsi)
	jmp	.L361
.L348:
	movq	8(%rbp), %rax
	movq	%rax, 8(%rsi)
	leal	2(%r13), %eax
	cmpl	%eax, %r12d
	jge	.L351
	movq	current_object(%rip), %rax
	movq	%rax, (%rsi)
	movl	$0, 16(%rsi)
	jmp	.L361
.L351:
	leaq	16(%rbp), %r15
	movzwl	16(%rbp), %eax
	cmpw	$16, %ax
	jne	.L353
	movq	8(%r15), %rax
	movq	%rax, (%rsi)
	jmp	.L355
.L353:
	cmpw	$4, %ax
	jne	.L356
	movq	8(%r15), %rdi
	call	find_object
	movq	%rax, (%rbx)
	testq	%rax, %rax
	je	.L358
	movq	%rax, %rdi
	call	object_visible
	testl	%eax, %eax
	jne	.L355
.L358:
	movl	%r14d, %ecx
	movl	$3, %edx
	movl	$20, %esi
	movq	%r15, %rdi
	call	bad_argument
	jmp	.L355
.L356:
	movl	%r14d, %ecx
	movl	$3, %edx
	movl	$20, %esi
	movq	%r15, %rdi
	call	bad_argument
.L355:
	movl	%r12d, %eax
	subl	%r13d, %eax
	subl	$2, %eax
	movl	%eax, 16(%rbx)
	leaq	32(%rbp), %rax
	movq	%rax, 24(%rbx)
	movq	(%rbx), %rax
	testb	$16, 2(%rax)
	je	.L361
	movl	%r14d, %ecx
	movl	$3, %edx
	movl	$20, %esi
	movq	%r15, %rdi
	call	bad_argument
.L361:
	movq	8(%rsp), %rbx
	movq	16(%rsp), %rbp
	movq	24(%rsp), %r12
	movq	32(%rsp), %r13
	movq	40(%rsp), %r14
	movq	48(%rsp), %r15
	addq	$56, %rsp
	ret
.LFE62:
	.size	process_efun_callback, .-process_efun_callback
.globl free_string_svalue
	.type	free_string_svalue, @function
free_string_svalue:
.LFB59:
	subq	$8, %rsp
.LCFI97:
	movq	8(%rdi), %rdx
	testb	$1, 2(%rdi)
	je	.L370
	leaq	-4(%rdx), %rsi
	movzwl	-4(%rdx), %ecx
	movzwl	-2(%rdx), %eax
	testw	%ax, %ax
	je	.L365
	subl	$1, %eax
	movw	%ax, -2(%rdx)
	testw	%ax, %ax
	jne	.L365
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rdi)
	je	.L368
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	movq	%rdx, %rdi
	call	deallocate_string
	jmp	.L370
.L368:
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L370
.L365:
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L370:
	addq	$8, %rsp
	ret
.LFE59:
	.size	free_string_svalue, .-free_string_svalue
.globl unlink_string_svalue
	.type	unlink_string_svalue, @function
unlink_string_svalue:
.LFB60:
	movq	%rbx, -24(%rsp)
.LCFI98:
	movq	%rbp, -16(%rsp)
.LCFI99:
	movq	%r12, -8(%rsp)
.LCFI100:
	subq	$24, %rsp
.LCFI101:
	movq	%rdi, %rbp
	movzwl	2(%rdi), %eax
	cmpw	$1, %ax
	je	.L374
	cmpw	$3, %ax
	je	.L375
	testw	%ax, %ax
	je	.L373
	.p2align 4,,7
	jmp	.L379
.L374:
	movq	8(%rdi), %rdi
	cmpw	$1, -2(%rdi)
	.p2align 4,,5
	jbe	.L379
	.p2align 4,,7
	call	int_string_unlink
	movq	%rax, 8(%rbp)
	.p2align 4,,2
	jmp	.L379
.L375:
	movq	8(%rdi), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %r12d
	movl	%r12d, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L377
	movq	8(%rdi), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %r12d
.L377:
	movl	%r12d, %edi
	call	int_new_string
	movq	%rax, %rbx
	leal	1(%r12), %edx
	movslq	%edx,%rdx
	movq	8(%rbp), %rsi
	movq	%rax, %rdi
	call	strncpy
	movq	8(%rbp), %rdi
	call	free_string
	movw	$1, 2(%rbp)
	movq	%rbx, 8(%rbp)
	jmp	.L379
.L373:
	movq	8(%rdi), %rdi
	call	int_string_copy
	movq	%rax, 8(%rbp)
	movw	$1, 2(%rbp)
.L379:
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	16(%rsp), %r12
	addq	$24, %rsp
	ret
.LFE60:
	.size	unlink_string_svalue, .-unlink_string_svalue
	.section	.rodata.str1.8
	.align 8
.LC34:
	.string	"Format string cannot end in '%%' in sscanf()\n"
	.align 8
.LC35:
	.string	"Too few arguments to sscanf()\n"
	.align 8
.LC36:
	.string	"Bad regexp format: '%%%s' in sscanf format string\n"
	.align 8
.LC37:
	.string	"Bad type : '%%%c' in sscanf() format string\n"
	.align 8
.LC38:
	.string	"Too few arguments to sscanf().\n"
	.align 8
.LC39:
	.string	"Illegal to have 2 adjacent %%s's in format string in sscanf()\n"
	.align 8
.LC40:
	.string	"Bad regexp format : '%%%s' in sscanf format string\n"
	.align 8
.LC41:
	.string	"Format string can't end in '%%'.\n"
	.text
.globl inter_sscanf
	.type	inter_sscanf, @function
inter_sscanf:
.LFB129:
	pushq	%r15
.LCFI102:
	pushq	%r14
.LCFI103:
	pushq	%r13
.LCFI104:
	pushq	%r12
.LCFI105:
	pushq	%rbp
.LCFI106:
	pushq	%rbx
.LCFI107:
	subq	$72, %rsp
.LCFI108:
	movq	%rdi, %r14
	movq	%rsi, %rbx
	movq	%rdx, %rbp
	movl	%ecx, %r15d
	testb	$4, (%rsi)
	jne	.L381
	movl	$114, %ecx
	movl	$1, %edx
	movl	$4, %esi
	movq	%rbx, %rdi
	call	bad_argument
.L381:
	movq	8(%rbx), %rax
	movq	%rax, 64(%rsp)
	testb	$4, (%rbp)
	jne	.L383
	movl	$114, %ecx
	movl	$2, %edx
	movl	$4, %esi
	movq	%rbp, %rdi
	call	bad_argument
.L383:
	movq	8(%rbp), %rbp
	movl	$0, 52(%rsp)
	testl	%r15d, %r15d
	jns	.L545
	jmp	.L385
.L388:
	cmpb	$37, %dl
	.p2align 4,,2
	jne	.L389
	leaq	1(%rbp), %rbx
	movzbl	1(%rbp), %eax
	cmpb	$37, %al
	.p2align 4,,3
	jne	.L391
	movq	64(%rsp), %rax
	cmpb	$37, (%rax)
	jne	.L393
	addq	$1, %rax
	movq	%rax, 64(%rsp)
	addq	$2, %rbp
	jmp	.L545
.L391:
	movq	%rbx, %rbp
	testb	%al, %al
	jne	.L395
	movl	$.LC34, %edi
	call	error
	movzbl	(%rbx), %edx
	testb	%dl, %dl
	jne	.L398
	jmp	.L397
.L389:
	movq	64(%rsp), %rax
	cmpb	(%rax), %dl
	.p2align 4,,2
	jne	.L393
	addq	$1, %rbp
	addq	$1, %rax
	movq	%rax, 64(%rsp)
.L545:
	movzbl	(%rbp), %edx
	testb	%dl, %dl
	jne	.L388
.L397:
	movq	64(%rsp), %rdi
	cmpb	$0, (%rdi)
	je	.L393
	testl	%r15d, %r15d
	je	.L393
	addl	$1, 52(%rsp)
	movw	$4, (%r14)
	call	int_string_copy
	movq	%rax, 8(%r14)
	movw	$1, 2(%r14)
	jmp	.L393
.L398:
	cmpb	$42, %dl
	sete	%al
	movzbl	%al, %eax
	movl	%eax, 32(%rsp)
	testl	%eax, %eax
	je	.L401
	leaq	1(%rbp), %rbx
	jmp	.L403
.L401:
	testl	%r15d, %r15d
	jne	.L403
	cmpb	$37, %dl
	.p2align 4,,5
	je	.L405
	movl	$.LC35, %edi
	movl	$0, %eax
	call	error
.L403:
	leaq	1(%rbx), %rbp
	movzbl	(%rbx), %eax
	cmpb	$102, %al
	je	.L410
	cmpb	$102, %al
	jg	.L413
	cmpb	$40, %al
	je	.L408
	cmpb	$100, %al
	.p2align 4,,5
	jne	.L407
	.p2align 4,,7
	jmp	.L409
.L413:
	cmpb	$115, %al
	.p2align 4,,7
	je	.L411
	cmpb	$120, %al
	.p2align 4,,7
	jne	.L407
	.p2align 4,,7
	jmp	.L412
.L408:
	movq	%rbp, %r13
	movl	$1, %ebx
	.p2align 4,,5
	jmp	.L546
.L412:
	movl	$16, %edx
	.p2align 4,,5
	jmp	.L415
.L409:
	movl	$10, %edx
.L415:
	movq	64(%rsp), %rbx
	leaq	64(%rsp), %rsi
	movl	$0, %ecx
	movq	%rbx, %rdi
	call	__strtol_internal
	cmpq	64(%rsp), %rbx
	je	.L393
	cmpl	$0, 32(%rsp)
	jne	.L417
	movw	$2, (%r14)
	movw	$0, 2(%r14)
	cltq
	movq	%rax, 8(%r14)
	subq	$16, %r14
	subl	$1, %r15d
	jmp	.L417
.L410:
	movq	64(%rsp), %rbx
	leaq	64(%rsp), %rsi
	movq	%rbx, %rdi
	call	_strtof
	movss	%xmm0, 12(%rsp)
	movl	12(%rsp), %eax
	cmpq	64(%rsp), %rbx
	je	.L393
	cmpl	$0, 32(%rsp)
	jne	.L417
	movw	$128, (%r14)
	movl	%eax, 8(%r14)
	subq	$16, %r14
	subl	$1, %r15d
	jmp	.L417
.L546:
	movzbl	(%r13), %eax
	cmpb	$41, %al
	je	.L424
	cmpb	$41, %al
	jg	.L426
	testb	%al, %al
	.p2align 4,,3
	je	.L422
	cmpb	$40, %al
	.p2align 4,,5
	jne	.L421
	.p2align 4,,7
	jmp	.L423
.L426:
	cmpb	$92, %al
	.p2align 4,,7
	jne	.L421
	addq	$1, %r13
	cmpb	$0, (%r13)
	.p2align 4,,5
	je	.L422
	addq	$1, %r13
	.p2align 4,,3
	jmp	.L546
.L422:
	movq	%rbp, %rsi
	movl	$.LC36, %edi
	movl	$0, %eax
	call	error
.L423:
	addl	$1, %ebx
.L421:
	addq	$1, %r13
	jmp	.L546
.L424:
	subl	$1, %ebx
	je	.L428
	addq	$1, %r13
	.p2align 4,,3
	jmp	.L546
.L428:
	movl	%r13d, %ebx
	subl	%ebp, %ebx
	leal	1(%rbx), %edi
	call	xalloc
	movq	%rax, %r12
	movslq	%ebx,%rbx
	movq	%rbx, %rdx
	movq	%rbp, %rsi
	movq	%rax, %rdi
	call	memcpy
	movb	$0, (%r12,%rbx)
	movl	$1, regexp_user(%rip)
	movl	$0, %esi
	movq	%r12, %rdi
	call	regcomp
	movq	%rax, %rbp
	movq	%r12, %rdi
	call	free
	testq	%rbp, %rbp
	jne	.L430
	movq	regexp_error(%rip), %rdi
	movl	$0, %eax
	call	error
.L430:
	movq	64(%rsp), %rsi
	movq	%rbp, %rdi
	call	regexec
	testl	%eax, %eax
	je	.L432
	movq	(%rbp), %rax
	cmpq	64(%rsp), %rax
	je	.L434
.L432:
	movq	%rbp, %rdi
	call	free
	jmp	.L393
.L434:
	cmpl	$0, 32(%rsp)
	.p2align 4,,3
	jne	.L435
	movq	80(%rbp), %rbx
	subl	%eax, %ebx
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %r12
	movslq	%ebx,%rbx
	movq	64(%rsp), %rsi
	movq	%rbx, %rdx
	movq	%rax, %rdi
	call	memcpy
	movb	$0, (%r12,%rbx)
	movw	$4, (%r14)
	movq	%r12, 8(%r14)
	movw	$1, 2(%r14)
	subq	$16, %r14
	subl	$1, %r15d
.L435:
	movq	80(%rbp), %rax
	movq	%rax, 64(%rsp)
	movq	%rbp, %rdi
	call	free
	leaq	1(%r13), %rbp
	jmp	.L417
.L407:
	movsbl	-1(%rbp),%esi
	movl	$.LC37, %edi
	movl	$0, %eax
	call	error
.L411:
	movzbl	(%rbp), %eax
	testb	%al, %al
	jne	.L437
	addl	$1, 52(%rsp)
	cmpl	$0, 32(%rsp)
	jne	.L393
	movw	$4, (%r14)
	movq	64(%rsp), %rdi
	call	int_string_copy
	movq	%rax, 8(%r14)
	movw	$1, 2(%r14)
	jmp	.L393
.L437:
	addq	$1, %rbp
	cmpb	$37, %al
	jne	.L440
	movq	64(%rsp), %r13
	cmpb	$42, (%rbp)
	sete	%al
	movzbl	%al, %eax
	movl	%eax, 36(%rsp)
	cmpl	$1, %eax
	sbbq	$-1, %rbp
	testl	%eax, %eax
	sete	%dl
	movzbl	%dl, %edx
	cmpl	$0, 32(%rsp)
	sete	%al
	movzbl	%al, %eax
	addl	%eax, %edx
	cmpl	%r15d, %edx
	jle	.L444
	cmpb	$37, (%rbp)
	je	.L446
	movl	$.LC38, %edi
	movl	$0, %eax
	call	error
.L444:
	addl	$1, 52(%rsp)
	movzbl	(%rbp), %eax
	addq	$1, %rbp
	cmpb	$102, %al
	je	.L453
	cmpb	$102, %al
	jg	.L456
	cmpb	$40, %al
	je	.L451
	cmpb	$40, %al
	.p2align 4,,5
	jg	.L457
	testb	%al, %al
	.p2align 4,,5
	je	.L449
	cmpb	$37, %al
	.p2align 4,,5
	jne	.L448
	.p2align 4,,7
	jmp	.L450
.L457:
	cmpb	$100, %al
	.p2align 4,,7
	jne	.L448
	.p2align 4,,9
	jmp	.L452
.L456:
	cmpb	$115, %al
	.p2align 4,,7
	je	.L454
	cmpb	$120, %al
	.p2align 4,,7
	jne	.L448
	.p2align 4,,7
	jmp	.L547
.L451:
	movq	%rbp, %r13
	movl	$1, %ebx
	.p2align 4,,5
	jmp	.L548
.L452:
	movzbl	(%r13), %ebx
	testb	%bl, %bl
	.p2align 4,,3
	je	.L459
	call	__ctype_b_loc
	movq	(%rax), %rdx
	.p2align 4,,2
	jmp	.L461
.L453:
	movzbl	(%r13), %ebx
	testb	%bl, %bl
	.p2align 4,,2
	je	.L459
	.p2align 4,,5
	call	__ctype_b_loc
	movq	(%rax), %rdx
	.p2align 4,,2
	jmp	.L463
.L454:
	movl	$.LC39, %edi
	movl	$0, %eax
	call	error
	jmp	.L547
.L464:
	addq	$1, %r13
.L547:
	movzbl	(%r13), %eax
	testb	%al, %al
	je	.L466
	cmpb	$48, %al
	.p2align 4,,3
	jne	.L464
.L466:
	cmpb	$48, %al
	jne	.L467
	movzbl	1(%r13), %eax
	cmpb	$120, %al
	.p2align 4,,3
	je	.L471
	cmpb	$88, %al
	.p2align 4,,3
	jne	.L469
.L471:
	.p2align 4,,5
	call	__ctype_b_loc
	movzbl	2(%r13), %edx
	movq	(%rax), %rax
	testb	$16, 1(%rax,%rdx,2)
	jne	.L459
.L469:
	addq	$2, %r13
.L467:
	cmpb	$0, (%r13)
	jne	.L547
	jmp	.L459
.L472:
	addq	$1, %r13
	movzbl	(%r13), %ebx
	testb	%bl, %bl
	je	.L459
.L461:
	movzbl	%bl, %eax
	testb	$8, 1(%rdx,%rax,2)
	je	.L472
	jmp	.L459
.L473:
	addq	$1, %r13
	movzbl	(%r13), %ebx
	testb	%bl, %bl
	je	.L459
.L463:
	movzbl	%bl, %eax
	testb	$8, 1(%rdx,%rax,2)
	jne	.L459
	cmpb	$46, %bl
	jne	.L473
	movzbl	1(%r13), %eax
	testb	$8, 1(%rdx,%rax,2)
	je	.L473
	.p2align 4,,4
	jmp	.L459
.L534:
	addq	$1, %r13
	movzbl	(%r13), %eax
	testb	%al, %al
	je	.L459
	cmpb	$37, %al
	.p2align 4,,3
	je	.L459
	.p2align 4,,3
	jmp	.L534
.L548:
	movzbl	(%r13), %eax
	cmpb	$41, %al
	.p2align 4,,5
	je	.L481
	cmpb	$41, %al
	.p2align 4,,5
	jg	.L483
	testb	%al, %al
	.p2align 4,,3
	je	.L479
	cmpb	$40, %al
	.p2align 4,,5
	jne	.L478
	.p2align 4,,7
	jmp	.L480
.L483:
	cmpb	$92, %al
	.p2align 4,,7
	jne	.L478
	addq	$1, %r13
	cmpb	$0, (%r13)
	.p2align 4,,5
	je	.L479
	addq	$1, %r13
	.p2align 4,,3
	jmp	.L548
.L479:
	movq	%rbp, %rsi
	movl	$.LC40, %edi
	movl	$0, %eax
	call	error
.L480:
	addl	$1, %ebx
.L478:
	addq	$1, %r13
	jmp	.L548
.L481:
	subl	$1, %ebx
	je	.L485
	addq	$1, %r13
	.p2align 4,,3
	jmp	.L548
.L485:
	movl	%r13d, %ebx
	subl	%ebp, %ebx
	leal	1(%rbx), %edi
	call	xalloc
	movq	%rax, %r12
	movslq	%ebx,%rbx
	movq	%rbx, %rdx
	movq	%rbp, %rsi
	movq	%rax, %rdi
	call	memcpy
	movb	$0, (%r12,%rbx)
	movl	$1, regexp_user(%rip)
	movl	$0, %esi
	movq	%r12, %rdi
	call	regcomp
	movq	%rax, %rbp
	movq	%r12, %rdi
	call	free
	testq	%rbp, %rbp
	jne	.L487
	movq	regexp_error(%rip), %rdi
	movl	$0, %eax
	call	error
.L487:
	movq	64(%rsp), %rsi
	movq	%rbp, %rdi
	call	regexec
	testl	%eax, %eax
	jne	.L489
	cmpl	$0, 32(%rsp)
	jne	.L491
	movw	$4, (%r14)
	movq	64(%rsp), %rdi
	call	int_string_copy
	movq	%rax, 8(%r14)
	movw	$1, 2(%r14)
.L491:
	movq	%rbp, %rdi
	call	free
	jmp	.L393
.L489:
	cmpl	$0, 32(%rsp)
	jne	.L493
	movq	(%rbp), %rbx
	subl	64(%rsp), %ebx
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %r12
	movslq	%ebx,%rbx
	movq	64(%rsp), %rsi
	movq	%rbx, %rdx
	movq	%rax, %rdi
	call	memcpy
	movb	$0, (%r12,%rbx)
	movw	$4, (%r14)
	movq	%r12, 8(%r14)
	movw	$1, 2(%r14)
	subq	$16, %r14
	subl	$1, %r15d
.L493:
	movq	80(%rbp), %rax
	movq	%rax, 64(%rsp)
	cmpl	$0, 36(%rsp)
	jne	.L495
	movq	80(%rbp), %rbx
	subl	(%rbp), %ebx
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %r12
	movslq	%ebx,%rbx
	movq	(%rbp), %rsi
	movq	%rbx, %rdx
	movq	%rax, %rdi
	call	memcpy
	movb	$0, (%r12,%rbx)
	movw	$4, (%r14)
	movq	%r12, 8(%r14)
	movw	$1, 2(%r14)
	subq	$16, %r14
	subl	$1, %r15d
.L495:
	movq	%rbp, %rdi
	call	free
	leaq	1(%r13), %rbp
	jmp	.L417
.L449:
	movl	$.LC41, %edi
	movl	$0, %eax
	call	error
.L448:
	movsbl	-1(%rbp),%esi
	movl	$.LC37, %edi
	movl	$0, %eax
	call	error
.L459:
	cmpl	$0, 32(%rsp)
	jne	.L497
	movl	%r13d, %ebx
	subl	64(%rsp), %ebx
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %r12
	movslq	%ebx,%rbx
	movq	64(%rsp), %rsi
	movq	%rbx, %rdx
	movq	%rax, %rdi
	call	memcpy
	movb	$0, (%r12,%rbx)
	movw	$4, (%r14)
	movq	%r12, 8(%r14)
	movw	$1, 2(%r14)
	subq	$16, %r14
	subl	$1, %r15d
.L497:
	movq	%r13, 64(%rsp)
	cmpb	$0, (%r13)
	je	.L393
	movzbl	-1(%rbp), %eax
	cmpb	$102, %al
	je	.L502
	cmpb	$102, %al
	jg	.L504
	cmpb	$37, %al
	.p2align 4,,3
	je	.L500
	cmpb	$100, %al
	.p2align 4,,5
	jne	.L440
	.p2align 4,,7
	jmp	.L501
.L504:
	movl	$16, %edx
	cmpb	$120, %al
	.p2align 4,,5
	je	.L505
	.p2align 4,,7
	jmp	.L440
.L501:
	movl	$10, %edx
.L505:
	leaq	64(%rsp), %rsi
	movl	$0, %ecx
	movq	%r13, %rdi
	call	__strtol_internal
	cmpl	$0, 36(%rsp)
	jne	.L417
	movw	$2, (%r14)
	movw	$0, 2(%r14)
	cltq
	movq	%rax, 8(%r14)
	subq	$16, %r14
	subl	$1, %r15d
	jmp	.L417
.L502:
	leaq	64(%rsp), %rsi
	movq	%r13, %rdi
	call	_strtof
	movss	%xmm0, 12(%rsp)
	movl	12(%rsp), %eax
	cmpl	$0, 36(%rsp)
	jne	.L417
	movw	$128, (%r14)
	movl	%eax, 8(%r14)
	subq	$16, %r14
	subl	$1, %r15d
	jmp	.L417
.L500:
	leaq	1(%r13), %rax
	movq	%rax, 64(%rsp)
	jmp	.L417
.L440:
	movl	$37, %esi
	movq	%rbp, %rdi
	call	strchr
	movq	%rax, 40(%rsp)
	testq	%rax, %rax
	je	.L508
	movl	40(%rsp), %eax
	addl	$1, %eax
	subl	%ebp, %eax
	jmp	.L510
.L508:
	movq	%rbp, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	subl	$1, %ecx
	movslq	%ecx,%rax
	addq	%rbp, %rax
	movq	%rax, 40(%rsp)
	leal	1(%rcx), %eax
.L510:
	subq	$1, %rbp
	movzbl	(%rbp), %r12d
	movq	64(%rsp), %r13
	movzbl	(%r13), %edx
	testb	%dl, %dl
	je	.L511
	movq	%r13, %rbx
	cltq
	movq	%rax, 16(%rsp)
.L513:
	cmpb	%dl, %r12b
	jne	.L514
	movq	16(%rsp), %rax
	movq	%rax, 24(%rsp)
	movq	%rax, %rdx
	movq	%rbp, %rsi
	movq	%rbx, %rdi
	call	strncmp
	testl	%eax, %eax
	jne	.L514
	cmpl	$0, 32(%rsp)
	jne	.L517
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %r12
	movslq	%ebx,%rbx
	movq	%rbx, %rdx
	movq	%r13, %rsi
	movq	%rax, %rdi
	call	memcpy
	movb	$0, (%r12,%rbx)
	movw	$4, (%r14)
	movq	%r12, 8(%r14)
	movw	$1, 2(%r14)
	subq	$16, %r14
	subl	$1, %r15d
.L517:
	movq	24(%rsp), %rax
	addq	%rax, 64(%rsp)
	movq	40(%rsp), %rbp
	jmp	.L417
.L514:
	addq	$1, %rbx
	movq	%rbx, 64(%rsp)
	movzbl	(%rbx), %edx
	testb	%dl, %dl
	jne	.L513
.L511:
	cmpq	%rbp, 40(%rsp)
	jne	.L393
.L417:
	addl	$1, 52(%rsp)
	testl	%r15d, %r15d
	jns	.L545
	.p2align 4,,3
	jmp	.L393
.L385:
	movl	$0, 52(%rsp)
	.p2align 4,,3
	jmp	.L393
.L395:
	movzbl	(%rbx), %edx
	.p2align 4,,6
	jmp	.L398
.L405:
	addq	$1, %rbp
	.p2align 4,,4
	jmp	.L407
.L446:
	addl	$1, 52(%rsp)
	addq	$1, %rbp
.L450:
	movzbl	(%r13), %eax
	testb	%al, %al
	je	.L459
	cmpb	$37, %al
	jne	.L534
	jmp	.L459
.L393:
	movl	52(%rsp), %eax
	addq	$72, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE129:
	.size	inter_sscanf, .-inter_sscanf
.globl function_exists
	.type	function_exists, @function
function_exists:
.LFB118:
	movq	%rbx, -16(%rsp)
.LCFI109:
	movq	%rbp, -8(%rsp)
.LCFI110:
	subq	$40, %rsp
.LCFI111:
	movq	%rsi, %rbx
	movl	%edx, %ebp
	cmpb	$35, (%rdi)
	je	.L550
	call	findstring
	testq	%rax, %rax
	je	.L550
	leaq	16(%rsp), %rcx
	leaq	20(%rsp), %rdx
	movq	40(%rbx), %rdi
	movq	%rax, %rsi
	call	ffbn_recurse
	movq	%rax, %rcx
	testq	%rax, %rax
	je	.L550
	movq	40(%rbx), %rax
	movslq	16(%rsp),%rdx
	movq	56(%rax), %rax
	movzwl	(%rax,%rdx,2), %eax
	testb	$2, %al
	jne	.L550
	testl	%ebp, %ebp
	jne	.L555
	testb	$7, %ah
	jne	.L550
.L555:
	movq	(%rcx), %rax
	.p2align 4,,5
	jmp	.L557
.L550:
	movl	$0, %eax
.L557:
	movq	24(%rsp), %rbx
	movq	32(%rsp), %rbp
	addq	$40, %rsp
	ret
.LFE118:
	.size	function_exists, .-function_exists
	.section	.rodata.str1.1
.LC42:
	.string	"function"
.LC43:
	.string	"CATCH"
.LC44:
	.string	"<fake>"
.LC45:
	.string	"program"
.LC46:
	.string	"object"
.LC47:
	.string	"file"
.LC48:
	.string	"line"
.LC49:
	.string	"arguments"
.LC50:
	.string	"locals"
.LC51:
	.string	"%d\n"
	.text
.globl get_svalue_trace
	.type	get_svalue_trace, @function
get_svalue_trace:
.LFB127:
	pushq	%r15
.LCFI112:
	pushq	%r14
.LCFI113:
	pushq	%r13
.LCFI114:
	pushq	%r12
.LCFI115:
	pushq	%rbp
.LCFI116:
	pushq	%rbx
.LCFI117:
	subq	$88, %rsp
.LCFI118:
	movl	$-1, 48(%rsp)
	movl	$the_null_array, %r15d
	cmpq	$0, current_prog(%rip)
	je	.L562
	movq	csp(%rip), %rax
	movl	$the_null_array, %r15d
	cmpq	$control_stack, %rax
	jb	.L562
	movq	%rax, %rdi
	subq	$control_stack, %rdi
	sarq	$4, %rdi
	imull	$-858993459, %edi, %edi
	addl	$1, %edi
	call	allocate_empty_array
	movq	%rax, %r15
	movl	$control_stack, %r14d
	cmpq	%r14, csp(%rip)
	jbe	.L567
	movl	$0, %r13d
.L568:
	movl	$6, %edi
	call	allocate_mapping
	movq	%rax, %r12
	movswl	control_stack(%r13),%eax
	andl	$3, %eax
	cmpl	$1, %eax
	je	.L571
	cmpl	$1, %eax
	jg	.L574
	testl	%eax, %eax
	je	.L570
	.p2align 4,,7
	jmp	.L569
.L574:
	cmpl	$2, %eax
	.p2align 4,,7
	je	.L572
	cmpl	$3, %eax
	.p2align 4,,7
	jne	.L569
	.p2align 4,,7
	jmp	.L573
.L570:
	leaq	52(%rsp), %rcx
	leaq	56(%rsp), %rdx
	movl	control_stack+8(%r13), %esi
	movq	control_stack+112(%r13), %rdi
	leaq	48(%rsp), %r8
	call	get_trace_details
	movq	56(%rsp), %rdx
	movl	$.LC42, %esi
	movq	%r12, %rdi
	call	add_mapping_string
	jmp	.L569
.L572:
	movl	$.LC43, %edx
	movl	$.LC42, %esi
	movq	%r12, %rdi
	call	add_mapping_string
	movl	$-1, 52(%rsp)
	jmp	.L569
.L573:
	movl	$.LC44, %edx
	movl	$.LC42, %esi
	movq	%r12, %rdi
	call	add_mapping_string
	movl	$-1, 52(%rsp)
	jmp	.L569
.L571:
	movl	$0, 32(%rsp)
	movq	$0, 40(%rsp)
	movw	$64, 16(%rsp)
	movq	control_stack+8(%r13), %rax
	movq	%rax, 24(%rsp)
	leaq	32(%rsp), %rsi
	leaq	16(%rsp), %rdi
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	call	svalue_to_string
	movq	40(%rsp), %rdx
	movl	$.LC42, %esi
	movq	%r12, %rdi
	call	add_mapping_string
	movq	40(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	40(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	movq	control_stack+8(%r13), %rdx
	movzbl	24(%rdx), %eax
	movl	%eax, 52(%rsp)
	movzbl	25(%rdx), %eax
	movl	%eax, 48(%rsp)
.L569:
	leaq	control_stack+80(%r13), %r14
	movq	control_stack+112(%r13), %rax
	movq	(%rax), %rdi
	call	add_slash
	movq	%rax, %rdx
	movl	$.LC45, %esi
	movq	%r12, %rdi
	call	add_mapping_malloced_string
	movq	control_stack+96(%r13), %rdx
	movl	$.LC46, %esi
	movq	%r12, %rdi
	call	add_mapping_object
	movq	control_stack+112(%r13), %rbx
	leaq	68(%rsp), %rcx
	leaq	72(%rsp), %rdx
	movq	control_stack+128(%r13), %rdi
	movq	%rbx, %rsi
	call	find_line
	cmpq	$0, 72(%rsp)
	jne	.L575
	movq	(%rbx), %rax
	movq	%rax, 72(%rsp)
.L575:
	movq	72(%rsp), %rdi
	call	add_slash
	movq	%rax, %rdx
	movl	$.LC47, %esi
	movq	%r12, %rdi
	call	add_mapping_malloced_string
	movslq	68(%rsp),%rdx
	movl	$.LC48, %esi
	movq	%r12, %rdi
	call	add_mapping_pair
	movl	52(%rsp), %edi
	cmpl	$-1, %edi
	je	.L577
	movq	control_stack+136(%r13), %rbp
	call	allocate_empty_array
	movq	%rax, %rbx
	cmpl	$0, 52(%rsp)
	jle	.L579
	movq	%rbp, %rcx
	movl	$0, %edi
.L581:
	movq	%rdi, %rax
	salq	$4, %rax
	leaq	8(%rbx,%rax), %rdx
	movq	%rcx, %rsi
	cmpw	$16, (%rcx)
	jne	.L582
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.L584
	testb	$16, 2(%rax)
	je	.L582
.L584:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L586
.L582:
	movq	(%rcx), %rax
	movq	%rax, (%rdx)
	movq	8(%rcx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L587
	cmpw	$8192, %ax
	je	.L587
	andb	$223, %ah
	movw	%ax, (%rdx)
.L587:
	movzwl	(%rsi), %eax
	cmpw	$4, %ax
	jne	.L590
	testb	$1, 2(%rsi)
	je	.L586
	movq	8(%rdx), %rsi
	subq	$4, %rsi
	movzwl	2(%rsi), %eax
	testw	%ax, %ax
	je	.L593
	addl	$1, %eax
	movw	%ax, 2(%rsi)
.L593:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L586
.L590:
	testl	$17272, %eax
	je	.L586
	movq	8(%rcx), %rax
	addw	$1, (%rax)
.L586:
	leal	1(%rdi), %eax
	addq	$1, %rdi
	addq	$16, %rcx
	cmpl	%eax, 52(%rsp)
	jg	.L581
.L579:
	movq	%rbx, %rdx
	movl	$.LC49, %esi
	movq	%r12, %rdi
	call	add_mapping_array
	subw	$1, (%rbx)
.L577:
	movl	48(%rsp), %edi
	testl	%edi, %edi
	jle	.L596
	movl	52(%rsp), %eax
	cmpl	$-1, %eax
	je	.L596
	cltq
	salq	$4, %rax
	movq	%rax, %rbp
	addq	control_stack+136(%r13), %rbp
	call	allocate_empty_array
	movq	%rax, %rbx
	cmpl	$0, 48(%rsp)
	jle	.L599
	movq	%rbp, %rcx
	movl	$0, %edi
.L601:
	movq	%rdi, %rax
	salq	$4, %rax
	leaq	8(%rbx,%rax), %rdx
	movq	%rcx, %rsi
	cmpw	$16, (%rcx)
	jne	.L602
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.L604
	testb	$16, 2(%rax)
	je	.L602
.L604:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L606
.L602:
	movq	(%rcx), %rax
	movq	%rax, (%rdx)
	movq	8(%rcx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L607
	cmpw	$8192, %ax
	je	.L607
	andb	$223, %ah
	movw	%ax, (%rdx)
.L607:
	movzwl	(%rsi), %eax
	cmpw	$4, %ax
	jne	.L610
	testb	$1, 2(%rsi)
	je	.L606
	movq	8(%rdx), %rsi
	subq	$4, %rsi
	movzwl	2(%rsi), %eax
	testw	%ax, %ax
	je	.L613
	addl	$1, %eax
	movw	%ax, 2(%rsi)
.L613:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L606
.L610:
	testl	$17272, %eax
	je	.L606
	movq	8(%rcx), %rax
	addw	$1, (%rax)
.L606:
	leal	1(%rdi), %eax
	addq	$1, %rdi
	addq	$16, %rcx
	cmpl	%eax, 48(%rsp)
	jg	.L601
.L599:
	movq	%rbx, %rdx
	movl	$.LC50, %esi
	movq	%r12, %rdi
	call	add_mapping_array
	subw	$1, (%rbx)
.L596:
	movq	%r13, %rax
	sarq	$4, %rax
	movabsq	$-3689348814741910323, %rdx
	imulq	%rdx, %rax
	salq	$4, %rax
	movw	$32, 8(%rax,%r15)
	movq	%r12, 16(%rax,%r15)
	addq	$80, %r13
	cmpq	%r14, csp(%rip)
	ja	.L568
.L567:
	movl	$6, %edi
	call	allocate_mapping
	movq	%rax, %rbp
	movswl	(%r14),%eax
	andl	$3, %eax
	cmpl	$1, %eax
	je	.L618
	cmpl	$1, %eax
	jg	.L621
	testl	%eax, %eax
	je	.L617
	.p2align 4,,7
	jmp	.L616
.L621:
	cmpl	$2, %eax
	.p2align 4,,7
	je	.L619
	cmpl	$3, %eax
	.p2align 4,,7
	jne	.L616
	.p2align 4,,7
	jmp	.L620
.L617:
	leaq	52(%rsp), %rcx
	leaq	56(%rsp), %rdx
	movl	8(%r14), %esi
	leaq	48(%rsp), %r8
	movq	current_prog(%rip), %rdi
	call	get_trace_details
	movq	56(%rsp), %rdx
	movl	$.LC42, %esi
	movq	%rbp, %rdi
	call	add_mapping_string
	jmp	.L616
.L619:
	movl	$.LC43, %edx
	movl	$.LC42, %esi
	movq	%rbp, %rdi
	call	add_mapping_string
	movl	$-1, 52(%rsp)
	jmp	.L616
.L620:
	movl	$.LC44, %edx
	movl	$.LC42, %esi
	movq	%rbp, %rdi
	call	add_mapping_string
	movl	$-1, 52(%rsp)
	jmp	.L616
.L618:
	movl	$0, 16(%rsp)
	movq	$0, 24(%rsp)
	movw	$64, (%rsp)
	movq	8(%r14), %rax
	movq	%rax, 8(%rsp)
	leaq	16(%rsp), %rsi
	movq	%rsp, %rdi
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	call	svalue_to_string
	movq	24(%rsp), %rdx
	movl	$.LC42, %esi
	movq	%rbp, %rdi
	call	add_mapping_string
	movq	24(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	24(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	movq	8(%r14), %rdx
	movzbl	24(%rdx), %eax
	movl	%eax, 52(%rsp)
	movzbl	25(%rdx), %eax
	movl	%eax, 48(%rsp)
.L616:
	movq	current_prog(%rip), %rax
	movq	(%rax), %rdi
	call	add_slash
	movq	%rax, %rdx
	movl	$.LC45, %esi
	movq	%rbp, %rdi
	call	add_mapping_malloced_string
	movq	current_object(%rip), %rdx
	movl	$.LC46, %esi
	movq	%rbp, %rdi
	call	add_mapping_object
	leaq	68(%rsp), %rsi
	leaq	72(%rsp), %rdi
	call	get_line_number_info
	movq	72(%rsp), %rdi
	movl	$0, %eax
	call	printf
	movl	68(%rsp), %esi
	movl	$.LC51, %edi
	movl	$0, %eax
	call	printf
	movq	72(%rsp), %rdi
	call	add_slash
	movq	%rax, %rdx
	movl	$.LC47, %esi
	movq	%rbp, %rdi
	call	add_mapping_malloced_string
	movslq	68(%rsp),%rdx
	movl	$.LC48, %esi
	movq	%rbp, %rdi
	call	add_mapping_pair
	movl	52(%rsp), %edi
	cmpl	$-1, %edi
	je	.L622
	call	allocate_empty_array
	movq	%rax, %rbx
	cmpl	$0, 52(%rsp)
	jle	.L624
	movl	$0, %esi
.L626:
	movq	%rsi, %rax
	salq	$4, %rax
	movq	%rax, %rdx
	addq	fp(%rip), %rdx
	leaq	8(%rbx,%rax), %rcx
	cmpw	$16, (%rdx)
	jne	.L627
	movq	8(%rdx), %rax
	testq	%rax, %rax
	je	.L629
	testb	$16, 2(%rax)
	je	.L627
.L629:
	movq	const0u(%rip), %rax
	movq	%rax, (%rcx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	jmp	.L631
.L627:
	movq	(%rdx), %rax
	movq	%rax, (%rcx)
	movq	8(%rdx), %rax
	movq	%rax, 8(%rcx)
	movzwl	(%rcx), %eax
	testb	$32, %ah
	je	.L632
	cmpw	$8192, %ax
	je	.L632
	andb	$223, %ah
	movw	%ax, (%rcx)
.L632:
	movzwl	(%rdx), %eax
	cmpw	$4, %ax
	jne	.L635
	testb	$1, 2(%rdx)
	je	.L631
	movq	8(%rcx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L638
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L638:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rcx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L631
.L635:
	testl	$17272, %eax
	je	.L631
	movq	8(%rdx), %rax
	addw	$1, (%rax)
.L631:
	leal	1(%rsi), %eax
	addq	$1, %rsi
	cmpl	52(%rsp), %eax
	jl	.L626
.L624:
	movq	%rbx, %rdx
	movl	$.LC49, %esi
	movq	%rbp, %rdi
	call	add_mapping_array
	subw	$1, (%rbx)
.L622:
	movl	48(%rsp), %edi
	testl	%edi, %edi
	jle	.L641
	cmpl	$-1, 52(%rsp)
	je	.L641
	call	allocate_empty_array
	movq	%rax, %rbx
	cmpl	$0, 48(%rsp)
	jle	.L644
	movl	$0, %esi
.L646:
	movl	%esi, %edi
	movl	%esi, %eax
	addl	52(%rsp), %eax
	cltq
	salq	$4, %rax
	movq	%rax, %rdx
	addq	fp(%rip), %rdx
	movq	%rsi, %rax
	salq	$4, %rax
	leaq	8(%rbx,%rax), %rcx
	cmpw	$16, (%rdx)
	jne	.L647
	movq	8(%rdx), %rax
	testq	%rax, %rax
	je	.L649
	testb	$16, 2(%rax)
	je	.L647
.L649:
	movq	const0u(%rip), %rax
	movq	%rax, (%rcx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	jmp	.L651
.L647:
	movq	(%rdx), %rax
	movq	%rax, (%rcx)
	movq	8(%rdx), %rax
	movq	%rax, 8(%rcx)
	movzwl	(%rcx), %eax
	testb	$32, %ah
	je	.L652
	cmpw	$8192, %ax
	je	.L652
	andb	$223, %ah
	movw	%ax, (%rcx)
.L652:
	movzwl	(%rdx), %eax
	cmpw	$4, %ax
	jne	.L655
	testb	$1, 2(%rdx)
	je	.L651
	movq	8(%rcx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L658
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L658:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rcx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L651
.L655:
	testl	$17272, %eax
	je	.L651
	movq	8(%rdx), %rax
	addw	$1, (%rax)
.L651:
	addq	$1, %rsi
	leal	1(%rdi), %eax
	cmpl	%eax, 48(%rsp)
	jg	.L646
.L644:
	movq	%rbx, %rdx
	movl	$.LC50, %esi
	movq	%rbp, %rdi
	call	add_mapping_array
	subw	$1, (%rbx)
.L641:
	movq	csp(%rip), %rax
	subq	$control_stack, %rax
	movabsq	$-3689348814741910323, %rdx
	imulq	%rdx, %rax
	movw	$32, 8(%r15,%rax)
	movq	csp(%rip), %rax
	subq	$control_stack, %rax
	imulq	%rdx, %rax
	movq	%rbp, 16(%r15,%rax)
	subw	$1, (%r15)
.L562:
	movq	%r15, %rax
	addq	$88, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE127:
	.size	get_svalue_trace, .-get_svalue_trace
.globl copy_some_svalues
	.type	copy_some_svalues, @function
copy_some_svalues:
.LFB69:
	movl	%edx, %r8d
	leal	-1(%r8), %eax
	cltq
	salq	$4, %rax
	leaq	(%rax,%rdi), %rdi
	addq	%rax, %rsi
	movl	$-1, %r9d
	jmp	.L668
.L669:
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	cmpw	$16, (%rsi)
	jne	.L670
	movq	8(%rsi), %rax
	testq	%rax, %rax
	je	.L672
	testb	$16, 2(%rax)
	je	.L670
.L672:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L674
.L670:
	movq	(%rcx), %rax
	movq	%rax, (%rdx)
	movq	8(%rcx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L675
	cmpw	$8192, %ax
	je	.L675
	andb	$223, %ah
	movw	%ax, (%rdx)
.L675:
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L678
	testb	$1, 2(%rcx)
	je	.L674
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L681
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L681:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L674
.L678:
	testl	$17272, %eax
	je	.L674
	movq	8(%rcx), %rax
	addw	$1, (%rax)
.L674:
	subq	$16, %rdi
	subq	$16, %rsi
.L668:
	subl	$1, %r8d
	cmpl	%r9d, %r8d
	jne	.L669
	rep ; ret
.LFE69:
	.size	copy_some_svalues, .-copy_some_svalues
.globl push_some_svalues
	.type	push_some_svalues, @function
push_some_svalues:
.LFB68:
	pushq	%rbp
.LCFI119:
	pushq	%rbx
.LCFI120:
	subq	$8, %rsp
.LCFI121:
	movl	%esi, %ebp
	leaq	16(%rdi), %rbx
	jmp	.L687
.L688:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L689
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L689:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	movq	%rbx, %rcx
	cmpw	$16, -16(%rbx)
	jne	.L691
	movq	-8(%rbx), %rax
	testq	%rax, %rax
	je	.L693
	testb	$16, 2(%rax)
	je	.L691
.L693:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L695
.L691:
	movq	-16(%rbx), %rax
	movq	%rax, (%rdx)
	movq	-8(%rbx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L696
	cmpw	$8192, %ax
	je	.L696
	andb	$223, %ah
	movw	%ax, (%rdx)
.L696:
	movzwl	-16(%rcx), %eax
	cmpw	$4, %ax
	jne	.L699
	testb	$1, -14(%rcx)
	je	.L695
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L702
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L702:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L695
.L699:
	testl	$17272, %eax
	je	.L695
	movq	-8(%rbx), %rax
	addw	$1, (%rax)
.L695:
	addq	$16, %rbx
.L687:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L688
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE68:
	.size	push_some_svalues, .-push_some_svalues
	.section	.rodata.str1.1
.LC52:
	.string	"free_svalue"
	.text
.globl kill_ref
	.type	kill_ref, @function
kill_ref:
.LFB48:
	pushq	%rbx
.LCFI122:
	movq	%rdi, %rbx
	cmpw	$32, 40(%rdi)
	jne	.L708
	movq	48(%rdi), %rdi
	cmpl	$0, 20(%rdi)
	jns	.L708
	movq	global_ref_list(%rip), %rax
	testq	%rax, %rax
	je	.L711
	cmpq	48(%rax), %rdi
	jne	.L751
	.p2align 4,,2
	jmp	.L708
.L714:
	cmpq	48(%rax), %rdi
	.p2align 4,,7
	je	.L708
.L751:
	movq	8(%rax), %rax
	testq	%rax, %rax
	.p2align 4,,5
	jne	.L714
.L711:
	.p2align 4,,5
	call	unlock_mapping
.L708:
	cmpq	$0, 32(%rbx)
	.p2align 4,,2
	je	.L715
	leaq	40(%rbx), %rcx
	movzwl	40(%rbx), %eax
	cmpw	$4, %ax
	jne	.L717
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L715
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L720
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L720
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L723
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L715
.L723:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L715
.L720:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L715
.L717:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L725
	testb	$32, %dh
	jne	.L725
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L715
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L732
	cmpw	$64, %ax
	jg	.L736
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L730
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L731
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L715
	.p2align 4,,7
	jmp	.L729
.L736:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L734
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L735
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L715
	.p2align 4,,7
	jmp	.L733
.L730:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L715
.L734:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L715
.L729:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L715
	call	dealloc_array
	jmp	.L715
.L733:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L715
	call	free
	jmp	.L715
.L731:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L715
.L732:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L715
.L735:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L715
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L715
.L725:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L715
	.p2align 4,,9
	call	*8(%rcx)
.L715:
	movq	8(%rbx), %rdx
	testq	%rdx, %rdx
	.p2align 4,,5
	je	.L741
	movq	16(%rbx), %rax
	movq	%rax, 16(%rdx)
.L741:
	movq	16(%rbx), %rdx
	testq	%rdx, %rdx
	je	.L743
	movq	8(%rbx), %rax
	movq	%rax, 8(%rdx)
	jmp	.L745
.L743:
	movq	8(%rbx), %rax
	movq	%rax, global_ref_list(%rip)
	testq	%rax, %rax
	je	.L745
	movq	$0, 16(%rax)
.L745:
	cmpw	$0, (%rbx)
	je	.L747
	movq	$0, 32(%rbx)
	movq	%rbx, 16(%rbx)
	movq	%rbx, 8(%rbx)
	jmp	.L750
.L747:
	movq	%rbx, %rdi
	call	free
.L750:
	popq	%rbx
	ret
.LFE48:
	.size	kill_ref, .-kill_ref
.globl int_free_svalue
	.type	int_free_svalue, @function
int_free_svalue:
.LFB61:
	subq	$8, %rsp
.LCFI123:
	movzwl	(%rdi), %eax
	cmpw	$4, %ax
	jne	.L753
	movq	8(%rdi), %rdx
	testb	$1, 2(%rdi)
	je	.L778
	leaq	-4(%rdx), %rsi
	movzwl	-4(%rdx), %ecx
	movzwl	-2(%rdx), %eax
	testw	%ax, %ax
	je	.L757
	subl	$1, %eax
	movw	%ax, -2(%rdx)
	testw	%ax, %ax
	jne	.L757
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rdi)
	je	.L760
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	movq	%rdx, %rdi
	call	deallocate_string
	jmp	.L778
.L760:
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L778
.L757:
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L778
.L753:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L762
	testb	$32, %dh
	jne	.L762
	movq	8(%rdi), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L778
	movzwl	(%rdi), %eax
	cmpw	$64, %ax
	je	.L769
	cmpw	$64, %ax
	jg	.L773
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L767
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L768
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L778
	.p2align 4,,7
	jmp	.L766
.L773:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L771
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L772
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L778
	.p2align 4,,7
	jmp	.L770
.L767:
	movq	8(%rdi), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L778
.L771:
	movq	8(%rdi), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L778
.L766:
	movq	8(%rdi), %rdi
	cmpq	$the_null_array, %rdi
	je	.L778
	call	dealloc_array
	jmp	.L778
.L770:
	movq	8(%rdi), %rdi
	cmpq	$null_buf, %rdi
	je	.L778
	call	free
	jmp	.L778
.L768:
	movq	8(%rdi), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L778
.L769:
	movq	8(%rdi), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L778
.L772:
	movq	8(%rdi), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L778
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L778
.L762:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L778
	.p2align 4,,9
	call	*8(%rdi)
.L778:
	addq	$8, %rsp
	.p2align 4,,7
	ret
.LFE61:
	.size	int_free_svalue, .-int_free_svalue
.globl pop_n_elems
	.type	pop_n_elems, @function
pop_n_elems:
.LFB76:
	pushq	%rbp
.LCFI124:
	pushq	%rbx
.LCFI125:
	subq	$8, %rsp
.LCFI126:
	movl	%edi, %ebp
	testl	%edi, %edi
	je	.L783
	movl	$0, %ebx
.L782:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L782
.L783:
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE76:
	.size	pop_n_elems, .-pop_n_elems
.globl restore_context
	.type	restore_context, @function
restore_context:
.LFB138:
	pushq	%rbp
.LCFI127:
	pushq	%rbx
.LCFI128:
	subq	$8, %rsp
.LCFI129:
	movq	%rdi, %rbx
	movl	$0, _in_reference_allowed(%rip)
	movq	cgsp(%rip), %rax
	cmpq	%rax, 216(%rdi)
	je	.L786
.L798:
	call	restore_command_giver
	movq	cgsp(%rip), %rax
	cmpq	%rax, 216(%rbx)
	jne	.L798
.L786:
	movq	200(%rbx), %rax
	cmpq	csp(%rip), %rax
	jae	.L788
	addq	$80, %rax
	movq	%rax, csp(%rip)
	call	pop_control_stack
.L788:
	movq	sp(%rip), %rax
	subq	208(%rbx), %rax
	shrq	$4, %rax
	leal	-1(%rax), %ebx
	cmpl	$-1, %ebx
	je	.L790
	movl	$-1, %ebp
.L791:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	subl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L791
.L790:
	movq	global_ref_list(%rip), %rdi
	testq	%rdi, %rdi
	je	.L797
.L799:
	movq	csp(%rip), %rax
	cmpq	%rax, 24(%rdi)
	jb	.L794
	movq	8(%rdi), %rbx
	call	kill_ref
	movq	%rbx, %rdi
	jmp	.L796
.L794:
	movq	8(%rdi), %rdi
.L796:
	testq	%rdi, %rdi
	jne	.L799
.L797:
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	.p2align 4,,1
	ret
.LFE138:
	.size	restore_context, .-restore_context
	.section	.rodata.str1.1
.LC53:
	.string	"--- trace ---\n"
.LC54:
	.string	"heart_beat"
.LC55:
	.string	"<catch>"
.LC56:
	.string	"arguments were ("
.LC57:
	.string	","
.LC58:
	.string	"%s"
.LC59:
	.string	")\n"
.LC60:
	.string	"locals were: "
	.section	.rodata.str1.8
	.align 8
.LC61:
	.string	"'%15s' in '/%20s' ('/%20s') %s\n"
	.section	.rodata.str1.1
.LC62:
	.string	"'%s' in '/%20s' ('/%20s') %s\n"
	.section	.rodata.str1.8
	.align 8
.LC63:
	.string	"'     <fake>' in '/%20s' ('/%20s') %s\n"
	.align 8
.LC64:
	.string	"'          CATCH' in '/%20s' ('/%20s') %s\n"
	.section	.rodata.str1.1
.LC65:
	.string	"--- end trace ---\n"
	.text
.globl dump_trace
	.type	dump_trace, @function
dump_trace:
.LFB126:
	pushq	%r15
.LCFI130:
	pushq	%r14
.LCFI131:
	pushq	%r13
.LCFI132:
	pushq	%r12
.LCFI133:
	pushq	%rbp
.LCFI134:
	pushq	%rbx
.LCFI135:
	subq	$328, %rsp
.LCFI136:
	movl	%edi, 12(%rsp)
	movl	$-1, 308(%rsp)
	movl	$-1, 304(%rsp)
	cmpq	$0, current_prog(%rip)
	je	.L804
	cmpq	$control_stack, csp(%rip)
	jb	.L804
	cmpl	$0, max_eval_error(%rip)
	jne	.L807
	cmpl	$0, too_deep_error(%rip)
	jne	.L807
	leaq	64(%rsp), %rbx
	movq	%rbx, %rdi
	call	save_context
	testl	%eax, %eax
	je	.L804
	movq	%rbx, %rdi
	call	_setjmp
	movl	$1, %r14d
	testl	%eax, %eax
	je	.L813
	leaq	64(%rsp), %rbx
	movq	%rbx, %rdi
	call	restore_context
	movq	%rbx, %rdi
	call	pop_context
	movl	$0, %r15d
	jmp	.L814
.L807:
	movl	$0, %r14d
.L813:
	cmpl	$0, 12(%rsp)
	je	.L815
	movl	$0, %eax
	call	last_instructions
.L815:
	movl	$.LC53, %edi
	movl	$0, %eax
	call	debug_message
	movl	$control_stack, %ebx
	movl	$0, %r15d
	cmpq	%rbx, csp(%rip)
	jbe	.L819
	movl	$control_stack+80, %r13d
	movl	$0, %r15d
.L820:
	movswl	-80(%r13),%eax
	andl	$3, %eax
	cmpl	$1, %eax
	je	.L823
	cmpl	$1, %eax
	jg	.L826
	testl	%eax, %eax
	je	.L822
	.p2align 4,,7
	jmp	.L821
.L826:
	cmpl	$2, %eax
	.p2align 4,,7
	je	.L824
	cmpl	$3, %eax
	.p2align 4,,7
	jne	.L821
	.p2align 4,,7
	jmp	.L825
.L822:
	leaq	308(%rsp), %rcx
	leaq	312(%rsp), %rdx
	movl	-72(%r13), %esi
	movq	32(%r13), %rdi
	leaq	304(%rsp), %r8
	call	get_trace_details
	movq	32(%r13), %rsi
	movq	48(%r13), %rdi
	call	get_line_number
	movq	%rax, %rcx
	movq	16(%r13), %rax
	movq	8(%rax), %rdx
	movq	32(%r13), %rax
	movq	312(%rsp), %rdi
	movq	(%rax), %rsi
	call	dump_trace_line
	movq	312(%rsp), %rsi
	movl	$.LC54, %edi
	movl	$11, %ecx
	cld
	repz
	cmpsb
	seta	%dl
	setb	%al
	cmpb	%al, %dl
	jne	.L821
	movq	-64(%r13), %rax
	movl	$0, %r15d
	testq	%rax, %rax
	je	.L821
	movq	8(%rax), %r15
	jmp	.L821
.L823:
	movl	$0, 48(%rsp)
	movq	$0, 56(%rsp)
	movw	$64, 32(%rsp)
	movq	-72(%r13), %rax
	movq	%rax, 40(%rsp)
	leaq	48(%rsp), %rsi
	leaq	32(%rsp), %rdi
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	call	svalue_to_string
	movq	32(%r13), %rsi
	movq	48(%r13), %rdi
	call	get_line_number
	movq	%rax, %rcx
	movq	16(%r13), %rax
	movq	8(%rax), %rdx
	movq	32(%r13), %rax
	movq	56(%rsp), %rdi
	movq	(%rax), %rsi
	call	dump_trace_line
	movq	56(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	56(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	movq	-72(%r13), %rdx
	movzbl	24(%rdx), %eax
	movl	%eax, 308(%rsp)
	movzbl	25(%rdx), %eax
	movl	%eax, 304(%rsp)
	jmp	.L821
.L825:
	movq	32(%r13), %rsi
	movq	48(%r13), %rdi
	call	get_line_number
	movq	%rax, %rcx
	movq	16(%r13), %rax
	movq	8(%rax), %rdx
	movq	32(%r13), %rax
	movq	(%rax), %rsi
	movl	$.LC44, %edi
	call	dump_trace_line
	movl	$-1, 308(%rsp)
	jmp	.L830
.L824:
	movq	32(%r13), %rsi
	movq	48(%r13), %rdi
	call	get_line_number
	movq	%rax, %rcx
	movq	16(%r13), %rax
	movq	8(%rax), %rdx
	movq	32(%r13), %rax
	movq	(%rax), %rsi
	movl	$.LC55, %edi
	call	dump_trace_line
	movl	$-1, 308(%rsp)
	jmp	.L830
.L821:
	cmpl	$-1, 308(%rsp)
	je	.L830
	movq	56(%r13), %rbx
	movl	$.LC56, %edi
	movl	$0, %eax
	call	debug_message
	movq	%rbx, %rbp
	movl	$0, %r12d
	cmpl	$0, 308(%rsp)
	jg	.L837
	jmp	.L833
.L834:
	testl	%r12d, %r12d
	je	.L835
	movl	$.LC57, %edi
	movl	$0, %eax
	call	debug_message
.L835:
	addq	$16, %rbp
.L837:
	leaq	32(%rsp), %rbx
	movq	%rbx, %rdi
	call	outbuf_zero
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	movq	%rbx, %rsi
	movq	%rbp, %rdi
	call	svalue_to_string
	movq	40(%rsp), %rsi
	movl	$.LC58, %edi
	movl	$0, %eax
	call	debug_message
	movq	40(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	40(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	addl	$1, %r12d
	cmpl	%r12d, 308(%rsp)
	jg	.L834
.L833:
	movl	$.LC59, %edi
	movl	$0, %eax
	call	debug_message
.L830:
	cmpl	$0, 304(%rsp)
	jle	.L838
	movl	308(%rsp), %eax
	cmpl	$-1, %eax
	je	.L838
	movslq	%eax,%rbx
	salq	$4, %rbx
	addq	56(%r13), %rbx
	movl	$.LC60, %edi
	movl	$0, %eax
	call	debug_message
	movq	%rbx, %rbp
	movl	$0, %r12d
	cmpl	$0, 304(%rsp)
	jg	.L846
	jmp	.L842
.L843:
	testl	%r12d, %r12d
	je	.L844
	movl	$.LC57, %edi
	movl	$0, %eax
	call	debug_message
.L844:
	addq	$16, %rbp
.L846:
	leaq	32(%rsp), %rbx
	movq	%rbx, %rdi
	call	outbuf_zero
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	movq	%rbx, %rsi
	movq	%rbp, %rdi
	call	svalue_to_string
	movq	40(%rsp), %rsi
	movl	$.LC58, %edi
	movl	$0, %eax
	call	debug_message
	movq	40(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	40(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	addl	$1, %r12d
	cmpl	%r12d, 304(%rsp)
	jg	.L843
.L842:
	movl	$.LC24, %edi
	movl	$0, %eax
	call	debug_message
.L838:
	movq	%r13, %rbx
	leaq	80(%r13), %r13
	cmpq	csp(%rip), %rbx
	jb	.L820
.L819:
	movswl	(%rbx),%eax
	andl	$3, %eax
	cmpl	$1, %eax
	je	.L849
	cmpl	$1, %eax
	jg	.L852
	testl	%eax, %eax
	je	.L848
	.p2align 4,,7
	jmp	.L847
.L852:
	cmpl	$2, %eax
	.p2align 4,,7
	je	.L850
	cmpl	$3, %eax
	.p2align 4,,7
	jne	.L847
	.p2align 4,,7
	jmp	.L851
.L848:
	leaq	308(%rsp), %rcx
	leaq	312(%rsp), %rdx
	movl	8(%rbx), %esi
	leaq	304(%rsp), %r8
	movq	current_prog(%rip), %rdi
	call	get_trace_details
	movq	current_prog(%rip), %rsi
	movq	pc(%rip), %rdi
	call	get_line_number
	movq	current_object(%rip), %rdx
	movq	8(%rdx), %rcx
	movq	312(%rsp), %rsi
	movq	%rax, %r8
	movq	current_prog(%rip), %rax
	movq	(%rax), %rdx
	movl	$.LC61, %edi
	movl	$0, %eax
	call	debug_message
	jmp	.L847
.L849:
	movl	$0, 32(%rsp)
	movq	$0, 40(%rsp)
	movw	$64, 16(%rsp)
	movq	8(%rbx), %rax
	movq	%rax, 24(%rsp)
	leaq	32(%rsp), %rsi
	leaq	16(%rsp), %rdi
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	call	svalue_to_string
	movq	current_prog(%rip), %rsi
	movq	pc(%rip), %rdi
	call	get_line_number
	movq	current_object(%rip), %rdx
	movq	8(%rdx), %rcx
	movq	40(%rsp), %rsi
	movq	%rax, %r8
	movq	current_prog(%rip), %rax
	movq	(%rax), %rdx
	movl	$.LC62, %edi
	movl	$0, %eax
	call	debug_message
	movq	40(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	40(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	movq	8(%rbx), %rdx
	movzbl	24(%rdx), %eax
	movl	%eax, 308(%rsp)
	movzbl	25(%rdx), %eax
	movl	%eax, 304(%rsp)
	jmp	.L847
.L851:
	movq	current_prog(%rip), %rsi
	movq	pc(%rip), %rdi
	call	get_line_number
	movq	%rax, %rcx
	movq	current_object(%rip), %rax
	movq	8(%rax), %rdx
	movq	current_prog(%rip), %rax
	movq	(%rax), %rsi
	movl	$.LC63, %edi
	movl	$0, %eax
	call	debug_message
	movl	$-1, 308(%rsp)
	jmp	.L853
.L850:
	movq	current_prog(%rip), %rsi
	movq	pc(%rip), %rdi
	call	get_line_number
	movq	%rax, %rcx
	movq	current_object(%rip), %rax
	movq	8(%rax), %rdx
	movq	current_prog(%rip), %rax
	movq	(%rax), %rsi
	movl	$.LC64, %edi
	movl	$0, %eax
	call	debug_message
	movl	$-1, 308(%rsp)
	jmp	.L853
.L847:
	cmpl	$-1, 308(%rsp)
	je	.L853
	movl	$.LC56, %edi
	movl	$0, %eax
	call	debug_message
	cmpl	$0, 308(%rsp)
	jle	.L855
	movl	$0, %ebx
	movl	$0, %ebp
	leaq	16(%rsp), %r12
	jmp	.L857
.L858:
	testl	%ebx, %ebx
	je	.L859
	movl	$.LC57, %edi
	movl	$0, %eax
	call	debug_message
.L859:
	addq	$16, %rbp
.L857:
	movq	%r12, %rdi
	call	outbuf_zero
	movq	%rbp, %rdi
	addq	fp(%rip), %rdi
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	movq	%r12, %rsi
	call	svalue_to_string
	movq	24(%rsp), %rsi
	movl	$.LC58, %edi
	movl	$0, %eax
	call	debug_message
	movq	24(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	24(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	addl	$1, %ebx
	cmpl	%ebx, 308(%rsp)
	jg	.L858
.L855:
	movl	$.LC59, %edi
	movl	$0, %eax
	call	debug_message
.L853:
	cmpl	$0, 304(%rsp)
	jle	.L861
	movl	308(%rsp), %eax
	cmpl	$-1, %eax
	je	.L861
	movslq	%eax,%rbx
	salq	$4, %rbx
	addq	fp(%rip), %rbx
	movl	$.LC60, %edi
	movl	$0, %eax
	call	debug_message
	movq	%rbx, %rbp
	movl	$0, %r12d
	cmpl	$0, 304(%rsp)
	jg	.L869
	jmp	.L865
.L866:
	testl	%r12d, %r12d
	je	.L867
	movl	$.LC57, %edi
	movl	$0, %eax
	call	debug_message
.L867:
	addq	$16, %rbp
.L869:
	leaq	16(%rsp), %rbx
	movq	%rbx, %rdi
	call	outbuf_zero
	movl	$0, %r8d
	movl	$0, %ecx
	movl	$0, %edx
	movq	%rbx, %rsi
	movq	%rbp, %rdi
	call	svalue_to_string
	movq	24(%rsp), %rsi
	movl	$.LC58, %edi
	movl	$0, %eax
	call	debug_message
	movq	24(%rsp), %rax
	movzwl	-4(%rax), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	24(%rsp), %rdi
	subq	$4, %rdi
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	addl	$1, %r12d
	cmpl	%r12d, 304(%rsp)
	jg	.L866
.L865:
	movl	$.LC24, %edi
	movl	$0, %eax
	call	debug_message
.L861:
	movl	$.LC65, %edi
	movl	$0, %eax
	call	debug_message
	testl	%r14d, %r14d
	je	.L814
	leaq	64(%rsp), %rdi
	call	pop_context
	.p2align 4,,3
	jmp	.L814
.L804:
	movl	$0, %r15d
.L814:
	movq	%r15, %rax
	addq	$328, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE126:
	.size	dump_trace, .-dump_trace
.globl reset_machine
	.type	reset_machine, @function
reset_machine:
.LFB130:
	pushq	%rbp
.LCFI137:
	pushq	%rbx
.LCFI138:
	subq	$8, %rsp
.LCFI139:
	movq	$control_stack-80, csp(%rip)
	testl	%edi, %edi
	je	.L874
	movq	$start_of_stack-16, sp(%rip)
	jmp	.L878
.L874:
	movq	sp(%rip), %rax
	subq	$start_of_stack, %rax
	movq	%rax, %rbx
	shrq	$4, %rbx
	cmpl	$-1, %ebx
	je	.L878
	movl	$-1, %ebp
.L877:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	subl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L877
.L878:
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE130:
	.size	reset_machine, .-reset_machine
.globl setup_variables
	.type	setup_variables, @function
setup_variables:
.LFB94:
	pushq	%r13
.LCFI140:
	pushq	%r12
.LCFI141:
	pushq	%rbp
.LCFI142:
	pushq	%rbx
.LCFI143:
	subq	$8, %rsp
.LCFI144:
	movl	%esi, %r12d
	movl	%edx, %r13d
	movl	%edi, %ebp
	subl	%edx, %ebp
	testl	%ebp, %ebp
	jle	.L881
	movl	$0, %ebx
.L885:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L885
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L886
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L886:
	testl	%r12d, %r12d
	je	.L888
	movl	$0, %ecx
.L890:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	addl	$1, %ecx
	cmpl	%r12d, %ecx
	je	.L888
	jmp	.L890
.L881:
	movl	%esi, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L891
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L891:
	testl	%ebx, %ebx
	je	.L888
	movl	$0, %ecx
.L894:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	addl	$1, %ecx
	cmpl	%ebx, %ecx
	jne	.L894
.L888:
	movq	sp(%rip), %rax
	leal	(%r13,%r12), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	ret
.LFE94:
	.size	setup_variables, .-setup_variables
.globl setup_new_frame
	.type	setup_new_frame, @function
setup_new_frame:
.LFB96:
	pushq	%r14
.LCFI145:
	pushq	%r13
.LCFI146:
	pushq	%r12
.LCFI147:
	pushq	%rbp
.LCFI148:
	pushq	%rbx
.LCFI149:
	movl	$0, variable_index_offset(%rip)
	movl	$0, function_index_offset(%rip)
	movq	current_prog(%rip), %r9
	movq	56(%r9), %rcx
	movslq	%edi,%rax
	movzwl	(%rcx,%rax,2), %eax
	movl	%eax, %edx
	andl	$32767, %edx
	cmpw	$-1, %ax
	cmovle	%edx, %edi
	movslq	%edi,%rax
	movzwl	(%rcx,%rax,2), %ecx
	testb	$1, %cl
	je	.L903
.L943:
	movzwl	150(%r9), %eax
	leal	-1(%rax), %esi
	movl	$0, %edx
	testl	%esi, %esi
	jle	.L907
	movq	104(%r9), %r8
	movl	$0, %edx
.L908:
	leal	1(%rdx,%rsi), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movzwl	8(%rax,%r8), %eax
	cmpl	%edi, %eax
	jg	.L909
	movl	%ecx, %edx
	jmp	.L911
.L909:
	leal	-1(%rcx), %esi
.L911:
	cmpl	%edx, %esi
	jg	.L908
.L907:
	movslq	%edx,%rdx
	salq	$4, %rdx
	movq	104(%r9), %rax
	movzwl	8(%rax,%rdx), %eax
	subl	%eax, %edi
	addl	%eax, function_index_offset(%rip)
	movq	104(%r9), %rax
	movzwl	10(%rdx,%rax), %eax
	addl	%eax, variable_index_offset(%rip)
	movq	104(%r9), %rax
	movq	(%rdx,%rax), %r9
	movq	%r9, current_prog(%rip)
	movslq	%edi,%rdx
	movq	56(%r9), %rax
	movzwl	(%rax,%rdx,2), %ecx
	testb	$1, %cl
	jne	.L943
.L903:
	movzwl	10(%r9), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	movslq	%edx,%rax
	movq	%rax, %r14
	salq	$4, %r14
	movq	%r14, %rsi
	addq	48(%r9), %rsi
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	testb	$16, %cl
	je	.L912
	movzbl	10(%rsi), %r13d
	movzbl	11(%rsi), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r13d
	jg	.L914
	leal	1(%rax), %ebx
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L916
	movl	$-1, %edi
.L917:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L916
	jmp	.L917
.L914:
	movl	%r13d, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L918
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L918:
	leal	-1(%rbx), %ecx
	cmpl	$-1, %ecx
	je	.L920
	movl	$-1, %esi
.L921:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	subl	$1, %ecx
	cmpl	%esi, %ecx
	jne	.L921
.L920:
	movl	$the_null_array, %ebp
.L916:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L922
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L922:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$8, (%rax)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L924
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L924:
	leal	-1(%r12), %ecx
	cmpl	$-1, %ecx
	je	.L926
	movl	$-1, %esi
.L927:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	subl	$1, %ecx
	cmpl	%esi, %ecx
	jne	.L927
.L926:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L928
.L912:
	movzbl	10(%rsi), %r13d
	movzbl	11(%rsi), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r13d, %ebp
	testl	%ebp, %ebp
	jle	.L929
	movl	$0, %ebx
.L933:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L933
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L934
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L934:
	leal	-1(%r12), %ecx
	cmpl	$-1, %ecx
	je	.L936
	movl	$-1, %esi
.L937:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	subl	$1, %ecx
	cmpl	%esi, %ecx
	je	.L936
	jmp	.L937
.L929:
	movl	%r12d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L938
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L938:
	testl	%ebx, %ebx
	je	.L936
	movl	$0, %ecx
.L941:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	addl	$1, %ecx
	cmpl	%ebx, %ecx
	jne	.L941
.L936:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L928:
	movq	current_prog(%rip), %rax
	addq	48(%rax), %r14
	movq	%r14, %rax
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret
.LFE96:
	.size	setup_new_frame, .-setup_new_frame
.globl setup_inherited_frame
	.type	setup_inherited_frame, @function
setup_inherited_frame:
.LFB97:
	pushq	%r14
.LCFI150:
	pushq	%r13
.LCFI151:
	pushq	%r12
.LCFI152:
	pushq	%rbp
.LCFI153:
	pushq	%rbx
.LCFI154:
	movq	current_prog(%rip), %r9
	movq	56(%r9), %rcx
	movslq	%edi,%rax
	movzwl	(%rcx,%rax,2), %eax
	movl	%eax, %edx
	andl	$32767, %edx
	cmpw	$-1, %ax
	cmovle	%edx, %edi
	movslq	%edi,%rax
	movzwl	(%rcx,%rax,2), %ecx
	testb	$1, %cl
	je	.L956
.L996:
	movzwl	150(%r9), %eax
	leal	-1(%rax), %esi
	movl	$0, %edx
	testl	%esi, %esi
	jle	.L960
	movq	104(%r9), %r8
	movl	$0, %edx
.L961:
	leal	1(%rdx,%rsi), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movzwl	8(%rax,%r8), %eax
	cmpl	%edi, %eax
	jg	.L962
	movl	%ecx, %edx
	jmp	.L964
.L962:
	leal	-1(%rcx), %esi
.L964:
	cmpl	%edx, %esi
	jg	.L961
.L960:
	movslq	%edx,%rdx
	salq	$4, %rdx
	movq	104(%r9), %rax
	movzwl	8(%rax,%rdx), %eax
	subl	%eax, %edi
	addl	%eax, function_index_offset(%rip)
	movq	104(%r9), %rax
	movzwl	10(%rdx,%rax), %eax
	addl	%eax, variable_index_offset(%rip)
	movq	104(%r9), %rax
	movq	(%rdx,%rax), %r9
	movq	%r9, current_prog(%rip)
	movslq	%edi,%rdx
	movq	56(%r9), %rax
	movzwl	(%rax,%rdx,2), %ecx
	testb	$1, %cl
	jne	.L996
.L956:
	movzwl	10(%r9), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	movslq	%edx,%rax
	movq	%rax, %r14
	salq	$4, %r14
	movq	%r14, %rsi
	addq	48(%r9), %rsi
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	testb	$16, %cl
	je	.L965
	movzbl	10(%rsi), %r13d
	movzbl	11(%rsi), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r13d
	jg	.L967
	leal	1(%rax), %ebx
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L969
	movl	$-1, %edi
.L970:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L969
	jmp	.L970
.L967:
	movl	%r13d, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L971
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L971:
	leal	-1(%rbx), %ecx
	cmpl	$-1, %ecx
	je	.L973
	movl	$-1, %esi
.L974:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	subl	$1, %ecx
	cmpl	%esi, %ecx
	jne	.L974
.L973:
	movl	$the_null_array, %ebp
.L969:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L975
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L975:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$8, (%rax)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L977
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L977:
	leal	-1(%r12), %ecx
	cmpl	$-1, %ecx
	je	.L979
	movl	$-1, %esi
.L980:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	subl	$1, %ecx
	cmpl	%esi, %ecx
	jne	.L980
.L979:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L981
.L965:
	movzbl	10(%rsi), %r13d
	movzbl	11(%rsi), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r13d, %ebp
	testl	%ebp, %ebp
	jle	.L982
	movl	$0, %ebx
.L986:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L986
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L987
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L987:
	leal	-1(%r12), %ecx
	cmpl	$-1, %ecx
	je	.L989
	movl	$-1, %esi
.L990:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	subl	$1, %ecx
	cmpl	%esi, %ecx
	je	.L989
	jmp	.L990
.L982:
	movl	%r12d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L991
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L991:
	testl	%ebx, %ebx
	je	.L989
	movl	$0, %ecx
.L994:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movq	const0u(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	addl	$1, %ecx
	cmpl	%ebx, %ecx
	jne	.L994
.L989:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L981:
	movq	current_prog(%rip), %rax
	addq	48(%rax), %r14
	movq	%r14, %rax
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret
.LFE97:
	.size	setup_inherited_frame, .-setup_inherited_frame
	.section	.rodata.str1.8
	.align 8
.LC66:
	.string	"Illegal rhs to array range lvalue\n"
	.align 8
.LC67:
	.string	"Illegal rhs to string range lvalue.\n"
	.align 8
.LC68:
	.string	"Illegal rhs to buffer range lvalue.\n"
	.text
.globl assign_lvalue_range
	.type	assign_lvalue_range, @function
assign_lvalue_range:
.LFB75:
	pushq	%r15
.LCFI155:
	pushq	%r14
.LCFI156:
	pushq	%r13
.LCFI157:
	pushq	%r12
.LCFI158:
	pushq	%rbp
.LCFI159:
	pushq	%rbx
.LCFI160:
	subq	$56, %rsp
.LCFI161:
	movq	%rdi, %rbp
	movl	global_lvalue_range(%rip), %ebx
	movl	global_lvalue_range+4(%rip), %eax
	movl	%eax, 4(%rsp)
	movl	global_lvalue_range+8(%rip), %r14d
	movq	global_lvalue_range+16(%rip), %r15
	movzwl	(%r15), %eax
	cmpw	$8, %ax
	je	.L1009
	cmpw	$256, %ax
	je	.L1010
	cmpw	$4, %ax
	jne	.L1102
	.p2align 4,,7
	jmp	.L1008
.L1009:
	cmpw	$8, (%rdi)
	.p2align 4,,7
	je	.L1011
	movl	$.LC66, %edi
	movl	$0, %eax
	call	error
.L1011:
	movq	8(%rbp), %rdx
	leaq	8(%rdx), %rax
	movq	%rax, 8(%rsp)
	movzwl	2(%rdx), %ebp
	movl	4(%rsp), %eax
	subl	%ebx, %eax
	cmpl	%eax, %ebp
	jne	.L1013
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	8(%r15), %rax
	leaq	8(%rax), %rbx
	leaq	24(%rdx), %r12
	jmp	.L1015
.L1016:
	movq	%rbx, %rdi
	call	int_free_svalue
	movq	%r12, %rdx
	cmpw	$16, -16(%r12)
	jne	.L1017
	movq	-8(%r12), %rax
	testq	%rax, %rax
	je	.L1019
	testb	$16, 2(%rax)
	je	.L1017
.L1019:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L1021
.L1017:
	movq	-16(%r12), %rax
	movq	%rax, (%rbx)
	movq	-8(%r12), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L1022
	cmpw	$8192, %ax
	je	.L1022
	andb	$223, %ah
	movw	%ax, (%rbx)
.L1022:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1025
	testb	$1, -14(%rdx)
	je	.L1021
	movq	%rbx, %rcx
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1028
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1028:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rcx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1021
.L1025:
	testl	$17272, %eax
	je	.L1021
	movq	-8(%r12), %rax
	addw	$1, (%rax)
.L1021:
	addq	$16, %rbx
	addq	$16, %r12
.L1015:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L1016
	jmp	.L1102
.L1013:
	movq	8(%r15), %rdx
	movq	%rdx, 16(%rsp)
	addq	$8, %rdx
	movq	%rdx, 24(%rsp)
	movl	%r14d, %r12d
	subl	4(%rsp), %r12d
	leal	(%rbp,%r12), %edi
	addl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %r13
	leaq	8(%rax), %rdi
	movq	16(%rsp), %rcx
	addq	$24, %rcx
	leaq	24(%rax), %rsi
	movl	%ebx, %r8d
	movl	$-1, %r10d
	jmp	.L1031
.L1032:
	movq	%rcx, %rdx
	cmpw	$16, -16(%rcx)
	jne	.L1033
	movq	-8(%rcx), %rax
	testq	%rax, %rax
	je	.L1035
	testb	$16, 2(%rax)
	je	.L1033
.L1035:
	movq	const0u(%rip), %rax
	movq	%rax, -16(%rsi)
	movq	const0u+8(%rip), %rax
	movq	%rax, -8(%rsi)
	jmp	.L1037
.L1033:
	movq	-16(%rcx), %rax
	movq	%rax, -16(%rsi)
	movq	-8(%rcx), %rax
	movq	%rax, -8(%rsi)
	movzwl	-16(%rsi), %eax
	testb	$32, %ah
	je	.L1038
	cmpw	$8192, %ax
	je	.L1038
	andb	$223, %ah
	movw	%ax, -16(%rsi)
.L1038:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1041
	testb	$1, -14(%rdx)
	je	.L1037
	movq	%rsi, %r9
	movq	-8(%rsi), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1044
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1044:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	-8(%r9), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1037
.L1041:
	testl	$17272, %eax
	je	.L1037
	movq	-8(%rcx), %rax
	addw	$1, (%rax)
.L1037:
	addq	$16, %rcx
	addq	$16, %rsi
.L1031:
	subl	$1, %r8d
	cmpl	%r10d, %r8d
	jne	.L1032
	movq	8(%rsp), %rcx
	addq	$16, %rcx
	mov	%ebx, %eax
	salq	$4, %rax
	leaq	16(%rdi,%rax), %rdi
	movl	$-1, %r8d
	jmp	.L1048
.L1049:
	movq	%rcx, %rdx
	cmpw	$16, -16(%rcx)
	jne	.L1050
	movq	-8(%rcx), %rax
	testq	%rax, %rax
	je	.L1052
	testb	$16, 2(%rax)
	je	.L1050
.L1052:
	movq	const0u(%rip), %rax
	movq	%rax, -16(%rdi)
	movq	const0u+8(%rip), %rax
	movq	%rax, -8(%rdi)
	jmp	.L1054
.L1050:
	movq	-16(%rcx), %rax
	movq	%rax, -16(%rdi)
	movq	-8(%rcx), %rax
	movq	%rax, -8(%rdi)
	movzwl	-16(%rdi), %eax
	testb	$32, %ah
	je	.L1055
	cmpw	$8192, %ax
	je	.L1055
	andb	$223, %ah
	movw	%ax, -16(%rdi)
.L1055:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1058
	testb	$1, -14(%rdx)
	je	.L1054
	movq	%rdi, %rsi
	movq	-8(%rdi), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1061
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1061:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	-8(%rsi), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1054
.L1058:
	testl	$17272, %eax
	je	.L1054
	movq	-8(%rcx), %rax
	addw	$1, (%rax)
.L1054:
	addq	$16, %rcx
	addq	$16, %rdi
.L1048:
	subl	$1, %ebp
	cmpl	%r8d, %ebp
	jne	.L1049
	movslq	4(%rsp),%rax
	salq	$4, %rax
	movq	24(%rsp), %rdx
	leaq	16(%rdx,%rax), %rcx
	movq	%rdi, %rsi
	movl	$-1, %r8d
	jmp	.L1065
.L1066:
	movq	%rcx, %rdx
	cmpw	$16, -16(%rcx)
	jne	.L1067
	movq	-8(%rcx), %rax
	testq	%rax, %rax
	je	.L1069
	testb	$16, 2(%rax)
	je	.L1067
.L1069:
	movq	const0u(%rip), %rax
	movq	%rax, -16(%rsi)
	movq	const0u+8(%rip), %rax
	movq	%rax, -8(%rsi)
	jmp	.L1071
.L1067:
	movq	-16(%rcx), %rax
	movq	%rax, -16(%rsi)
	movq	-8(%rcx), %rax
	movq	%rax, -8(%rsi)
	movzwl	-16(%rsi), %eax
	testb	$32, %ah
	je	.L1072
	cmpw	$8192, %ax
	je	.L1072
	andb	$223, %ah
	movw	%ax, -16(%rsi)
.L1072:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1075
	testb	$1, -14(%rdx)
	je	.L1071
	movq	%rsi, %rdi
	movq	-8(%rsi), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1078
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1078:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	-8(%rdi), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1071
.L1075:
	testl	$17272, %eax
	je	.L1071
	movq	-8(%rcx), %rax
	addw	$1, (%rax)
.L1071:
	addq	$16, %rcx
	addq	$16, %rsi
.L1065:
	subl	$1, %r12d
	cmpl	%r8d, %r12d
	jne	.L1066
	movq	16(%rsp), %rdi
	call	free_array
	movq	%r13, 8(%r15)
	jmp	.L1102
.L1008:
	cmpw	$4, (%rdi)
	je	.L1082
	movl	$.LC67, %edi
	movl	$0, %eax
	call	error
.L1082:
	testb	$1, 2(%rbp)
	je	.L1084
	movq	8(%rbp), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %r12d
	movl	%r12d, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L1086
	movq	8(%rbp), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %r12d
	jmp	.L1086
.L1084:
	movq	8(%rbp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %r12d
.L1086:
	movl	4(%rsp), %eax
	subl	%ebx, %eax
	cmpl	%eax, %r12d
	jne	.L1088
	movslq	%r12d,%rdx
	movq	8(%rbp), %rsi
	movslq	%ebx,%rdi
	addq	8(%r15), %rdi
	call	strncpy
	jmp	.L1102
.L1088:
	movq	8(%r15), %rax
	movq	%rax, 32(%rsp)
	subl	4(%rsp), %r14d
	leal	(%r14,%rbx), %edi
	addl	%r12d, %edi
	call	int_new_string
	movq	%rax, %r13
	movq	%rax, 8(%r15)
	testl	%ebx, %ebx
	jle	.L1090
	movslq	%ebx,%rbx
	movq	%rbx, %rdx
	movq	32(%rsp), %rsi
	movq	%rax, %rdi
	call	strncpy
	addq	%rbx, %r13
.L1090:
	movq	8(%rbp), %rsi
	movq	%r13, %rdi
	call	strcpy
	testl	%r14d, %r14d
	jle	.L1092
	movslq	%r12d,%rbx
	addq	%r13, %rbx
	movslq	%r14d,%r12
	movslq	4(%rsp),%rsi
	addq	32(%rsp), %rsi
	movq	%r12, %rdx
	movq	%rbx, %rdi
	call	strncpy
	movb	$0, (%r12,%rbx)
.L1092:
	movq	32(%rsp), %rdi
	subq	$4, %rdi
	movq	32(%rsp), %rdx
	movzwl	-4(%rdx), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1102
.L1010:
	cmpw	$256, (%rdi)
	je	.L1094
	movl	$.LC68, %edi
	movl	$0, %eax
	call	error
.L1094:
	movq	8(%rbp), %rcx
	movl	4(%rcx), %r13d
	movl	4(%rsp), %eax
	subl	%ebx, %eax
	cmpl	%eax, %r13d
	jne	.L1096
	movq	8(%r15), %rdi
	addq	$8, %rdi
	movslq	%ebx,%rax
	addq	%rax, %rdi
	movslq	%r13d,%rdx
	leaq	8(%rcx), %rsi
	call	memcpy
	jmp	.L1102
.L1096:
	movq	8(%r15), %rax
	addq	$8, %rax
	movq	%rax, 48(%rsp)
	subl	4(%rsp), %r14d
	leal	(%r13,%r14), %edi
	addl	%ebx, %edi
	call	allocate_buffer
	movq	%rax, 40(%rsp)
	movq	%rax, %r12
	addq	$8, %r12
	testl	%ebx, %ebx
	jle	.L1098
	movslq	%ebx,%rbx
	movq	%rbx, %rdx
	movq	48(%rsp), %rsi
	movq	%r12, %rdi
	call	memcpy
	addq	%rbx, %r12
.L1098:
	movslq	%r13d,%rbx
	movq	8(%rbp), %rsi
	movq	%rbx, %rdx
	movq	%r12, %rdi
	call	memcpy
	testl	%r14d, %r14d
	jle	.L1100
	leaq	(%rbx,%r12), %rdi
	movslq	%r14d,%rdx
	movslq	4(%rsp),%rsi
	addq	48(%rsp), %rsi
	call	memcpy
.L1100:
	movq	8(%r15), %rdi
	call	free_buffer
	movq	40(%rsp), %rdx
	movq	%rdx, 8(%r15)
.L1102:
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE75:
	.size	assign_lvalue_range, .-assign_lvalue_range
.globl check_for_destr
	.type	check_for_destr, @function
check_for_destr:
.LFB100:
	pushq	%r12
.LCFI162:
	pushq	%rbp
.LCFI163:
	pushq	%rbx
.LCFI164:
	movq	%rdi, %r12
	movzwl	2(%rdi), %ebx
	leal	-1(%rbx), %eax
	movslq	%eax,%rbp
	jmp	.L1104
.L1105:
	movslq	%ebx,%rax
	salq	$4, %rax
	cmpw	$16, 8(%rax,%r12)
	jne	.L1106
	movq	16(%rax,%r12), %rax
	testb	$16, 2(%rax)
	je	.L1106
	movq	%rbp, %rax
	salq	$4, %rax
	leaq	(%r12,%rax), %rax
	leaq	8(%rax), %rcx
	movzwl	8(%rax), %eax
	cmpw	$4, %ax
	jne	.L1109
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1111
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1113
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1113
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1116
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1111
.L1116:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1111
.L1113:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1111
.L1109:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1118
	testb	$32, %dh
	jne	.L1118
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1111
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1125
	cmpw	$64, %ax
	jg	.L1129
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1123
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1124
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1111
	.p2align 4,,7
	jmp	.L1122
.L1129:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1127
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1128
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1111
	.p2align 4,,7
	jmp	.L1126
.L1123:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1111
.L1127:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1111
.L1122:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1111
	call	dealloc_array
	jmp	.L1111
.L1126:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1111
	call	free
	jmp	.L1111
.L1124:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1111
.L1125:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1111
.L1128:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1111
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1111
.L1118:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1111
	.p2align 4,,9
	call	*8(%rcx)
.L1111:
	movslq	%ebx,%rax
	salq	$4, %rax
	movq	const0u(%rip), %rdx
	movq	%rdx, 8(%rax,%r12)
	movq	const0u+8(%rip), %rdx
	movq	%rdx, 16(%rax,%r12)
.L1106:
	subq	$1, %rbp
.L1104:
	subl	$1, %ebx
	cmpl	$-1, %ebx
	jne	.L1105
	popq	%rbx
	popq	%rbp
	popq	%r12
	ret
.LFE100:
	.size	check_for_destr, .-check_for_destr
.globl pop_stack
	.type	pop_stack, @function
pop_stack:
.LFB71:
	subq	$8, %rsp
.LCFI165:
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1137
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1162
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1141
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1141
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1144
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1162
.L1144:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1162
.L1141:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1162
.L1137:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1146
	testb	$32, %dh
	jne	.L1146
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1162
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1153
	cmpw	$64, %ax
	jg	.L1157
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1151
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1152
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1162
	.p2align 4,,7
	jmp	.L1150
.L1157:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1155
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1156
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1162
	.p2align 4,,7
	jmp	.L1154
.L1151:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1162
.L1155:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1162
.L1150:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1162
	call	dealloc_array
	jmp	.L1162
.L1154:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1162
	call	free
	jmp	.L1162
.L1152:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1162
.L1153:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1162
.L1156:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1162
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1162
.L1146:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1162
	.p2align 4,,9
	call	*8(%rcx)
.L1162:
	addq	$8, %rsp
	.p2align 4,,7
	ret
.LFE71:
	.size	pop_stack, .-pop_stack
.globl assign_svalue
	.type	assign_svalue, @function
assign_svalue:
.LFB67:
	pushq	%rbp
.LCFI166:
	pushq	%rbx
.LCFI167:
	subq	$8, %rsp
.LCFI168:
	movq	%rdi, %rbx
	movq	%rsi, %rbp
	movzwl	(%rdi), %eax
	cmpw	$4, %ax
	jne	.L1164
	movq	8(%rdi), %rdi
	testb	$1, 2(%rbx)
	je	.L1166
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1168
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1168
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rbx)
	je	.L1171
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1166
.L1171:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L1166
.L1168:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1166
.L1164:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1173
	testb	$32, %dh
	jne	.L1173
	movq	8(%rdi), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1166
	movzwl	(%rdi), %eax
	cmpw	$64, %ax
	je	.L1180
	cmpw	$64, %ax
	jg	.L1184
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1178
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1179
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1166
	.p2align 4,,7
	jmp	.L1177
.L1184:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1182
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1183
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1166
	.p2align 4,,7
	jmp	.L1181
.L1178:
	movq	8(%rdi), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1166
.L1182:
	movq	8(%rdi), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1166
.L1177:
	movq	8(%rdi), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1166
	call	dealloc_array
	jmp	.L1166
.L1181:
	movq	8(%rdi), %rdi
	cmpq	$null_buf, %rdi
	je	.L1166
	call	free
	jmp	.L1166
.L1179:
	movq	8(%rdi), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1166
.L1180:
	movq	8(%rdi), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1166
.L1183:
	movq	8(%rdi), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1166
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1166
.L1173:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1166
	.p2align 4,,9
	call	*8(%rbx)
.L1166:
	cmpw	$16, (%rbp)
	.p2align 4,,7
	jne	.L1189
	movq	8(%rbp), %rax
	testq	%rax, %rax
	.p2align 4,,5
	je	.L1191
	testb	$16, 2(%rax)
	.p2align 4,,3
	je	.L1189
.L1191:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L1203
.L1189:
	movq	(%rbp), %rax
	movq	%rax, (%rbx)
	movq	8(%rbp), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L1194
	cmpw	$8192, %ax
	je	.L1194
	andb	$223, %ah
	movw	%ax, (%rbx)
.L1194:
	movzwl	(%rbp), %eax
	cmpw	$4, %ax
	jne	.L1197
	testb	$1, 2(%rbp)
	je	.L1203
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1200
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1200:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1203
.L1197:
	testl	$17272, %eax
	je	.L1203
	movq	8(%rbp), %rax
	addw	$1, (%rax)
.L1203:
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	ret
.LFE67:
	.size	assign_svalue, .-assign_svalue
.globl free_some_svalues
	.type	free_some_svalues, @function
free_some_svalues:
.LFB64:
	pushq	%r12
.LCFI169:
	pushq	%rbp
.LCFI170:
	pushq	%rbx
.LCFI171:
	movq	%rdi, %r12
	movl	%esi, %ebp
	leal	-1(%rbp), %eax
	cltq
	salq	$4, %rax
	leaq	(%rax,%rdi), %rbx
	jmp	.L1205
.L1206:
	movq	%rbx, %rcx
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L1207
	movq	8(%rbx), %rdi
	testb	$1, 2(%rbx)
	je	.L1209
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1211
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1211
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rbx)
	je	.L1214
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1209
.L1214:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L1209
.L1211:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1209
.L1207:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1216
	testb	$32, %dh
	jne	.L1216
	movq	8(%rbx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1209
	movzwl	(%rbx), %eax
	cmpw	$64, %ax
	je	.L1223
	cmpw	$64, %ax
	jg	.L1227
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1221
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1222
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1209
	.p2align 4,,7
	jmp	.L1220
.L1227:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1225
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1226
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1209
	.p2align 4,,7
	jmp	.L1224
.L1221:
	movq	8(%rbx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1209
.L1225:
	movq	8(%rbx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1209
.L1220:
	movq	8(%rbx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1209
	call	dealloc_array
	jmp	.L1209
.L1224:
	movq	8(%rbx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1209
	call	free
	jmp	.L1209
.L1222:
	movq	8(%rbx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1209
.L1223:
	movq	8(%rbx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1209
.L1226:
	movq	8(%rbx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1209
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1209
.L1216:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1209
	.p2align 4,,9
	call	*8(%rcx)
.L1209:
	subq	$16, %rbx
.L1205:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	.p2align 4,,3
	jne	.L1206
	movq	%r12, %rdi
	call	free
	popq	%rbx
	popq	%rbp
	popq	%r12
	ret
.LFE64:
	.size	free_some_svalues, .-free_some_svalues
.globl pop_3_elems
	.type	pop_3_elems, @function
pop_3_elems:
.LFB78:
	subq	$8, %rsp
.LCFI172:
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1235
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1237
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1239
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1239
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1242
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1237
.L1242:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1237
.L1239:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1237
.L1235:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1244
	testb	$32, %dh
	jne	.L1244
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1237
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1251
	cmpw	$64, %ax
	jg	.L1255
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1249
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1250
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1237
	.p2align 4,,7
	jmp	.L1248
.L1255:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1253
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1254
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1237
	.p2align 4,,7
	jmp	.L1252
.L1249:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1237
.L1253:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1237
.L1248:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1237
	call	dealloc_array
	jmp	.L1237
.L1252:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1237
	call	free
	jmp	.L1237
.L1250:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1237
.L1251:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1237
.L1254:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1237
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1237
.L1244:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1237
	.p2align 4,,9
	call	*8(%rcx)
.L1237:
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1260
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1262
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1264
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1264
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1267
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1262
.L1267:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1262
.L1264:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1262
.L1260:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1269
	testb	$32, %dh
	jne	.L1269
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1262
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1276
	cmpw	$64, %ax
	jg	.L1280
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1274
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1275
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1262
	.p2align 4,,7
	jmp	.L1273
.L1280:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1278
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1279
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1262
	.p2align 4,,7
	jmp	.L1277
.L1274:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1262
.L1278:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1262
.L1273:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1262
	call	dealloc_array
	jmp	.L1262
.L1277:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1262
	call	free
	jmp	.L1262
.L1275:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1262
.L1276:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1262
.L1279:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1262
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1262
.L1269:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1262
	.p2align 4,,9
	call	*8(%rcx)
.L1262:
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1285
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1310
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1289
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1289
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1292
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1310
.L1292:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1310
.L1289:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1310
.L1285:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1294
	testb	$32, %dh
	jne	.L1294
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1310
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1301
	cmpw	$64, %ax
	jg	.L1305
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1299
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1300
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1310
	.p2align 4,,7
	jmp	.L1298
.L1305:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1303
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1304
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1310
	.p2align 4,,7
	jmp	.L1302
.L1299:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1310
.L1303:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1310
.L1298:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1310
	call	dealloc_array
	jmp	.L1310
.L1302:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1310
	call	free
	jmp	.L1310
.L1300:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1310
.L1301:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1310
.L1304:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1310
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1310
.L1294:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1310
	.p2align 4,,9
	call	*8(%rcx)
.L1310:
	addq	$8, %rsp
	.p2align 4,,7
	ret
.LFE78:
	.size	pop_3_elems, .-pop_3_elems
.globl pop_2_elems
	.type	pop_2_elems, @function
pop_2_elems:
.LFB77:
	subq	$8, %rsp
.LCFI173:
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1312
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1314
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1316
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1316
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1319
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1314
.L1319:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1314
.L1316:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1314
.L1312:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1321
	testb	$32, %dh
	jne	.L1321
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1314
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1328
	cmpw	$64, %ax
	jg	.L1332
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1326
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1327
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1314
	.p2align 4,,7
	jmp	.L1325
.L1332:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1330
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1331
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1314
	.p2align 4,,7
	jmp	.L1329
.L1326:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1314
.L1330:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1314
.L1325:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1314
	call	dealloc_array
	jmp	.L1314
.L1329:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1314
	call	free
	jmp	.L1314
.L1327:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1314
.L1328:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1314
.L1331:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1314
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1314
.L1321:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1314
	.p2align 4,,9
	call	*8(%rcx)
.L1314:
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1337
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1362
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1341
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1341
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1344
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1362
.L1344:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1362
.L1341:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1362
.L1337:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1346
	testb	$32, %dh
	jne	.L1346
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1362
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1353
	cmpw	$64, %ax
	jg	.L1357
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1351
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1352
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1362
	.p2align 4,,7
	jmp	.L1350
.L1357:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1355
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1356
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1362
	.p2align 4,,7
	jmp	.L1354
.L1351:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1362
.L1355:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1362
.L1350:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1362
	call	dealloc_array
	jmp	.L1362
.L1354:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1362
	call	free
	jmp	.L1362
.L1352:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1362
.L1353:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1362
.L1356:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1362
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1362
.L1346:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1362
	.p2align 4,,9
	call	*8(%rcx)
.L1362:
	addq	$8, %rsp
	.p2align 4,,7
	ret
.LFE77:
	.size	pop_2_elems, .-pop_2_elems
.globl copy_lvalue_range
	.type	copy_lvalue_range, @function
copy_lvalue_range:
.LFB74:
	pushq	%r15
.LCFI174:
	pushq	%r14
.LCFI175:
	pushq	%r13
.LCFI176:
	pushq	%r12
.LCFI177:
	pushq	%rbp
.LCFI178:
	pushq	%rbx
.LCFI179:
	subq	$56, %rsp
.LCFI180:
	movq	%rdi, %rbp
	movl	global_lvalue_range(%rip), %ebx
	movl	global_lvalue_range+4(%rip), %eax
	movl	%eax, 4(%rsp)
	movl	global_lvalue_range+8(%rip), %r14d
	movq	global_lvalue_range+16(%rip), %r15
	movzwl	(%r15), %eax
	cmpw	$8, %ax
	je	.L1366
	cmpw	$256, %ax
	je	.L1367
	cmpw	$4, %ax
	jne	.L1504
	.p2align 4,,7
	jmp	.L1365
.L1366:
	cmpw	$8, (%rdi)
	.p2align 4,,7
	je	.L1368
	movl	$.LC66, %edi
	movl	$0, %eax
	call	error
.L1368:
	movq	8(%rbp), %rbp
	movq	%rbp, 8(%rsp)
	movq	%rbp, %r13
	addq	$8, %r13
	movq	%rbp, %rdx
	movzwl	2(%rbp), %ebp
	movl	4(%rsp), %eax
	subl	%ebx, %eax
	cmpl	%eax, %ebp
	jne	.L1370
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	8(%r15), %rax
	leaq	8(%rax), %rbx
	cmpw	$1, (%rdx)
	je	.L1372
	movq	%rdx, %r12
	addq	$24, %r12
	jmp	.L1374
.L1372:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	je	.L1375
	addq	$16, %rbx
.L1377:
	movzwl	-16(%rbx), %eax
	cmpw	$4, %ax
	jne	.L1378
	movq	-8(%rbx), %rdi
	testb	$1, -14(%rbx)
	je	.L1380
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1382
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1382
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, -14(%rbx)
	je	.L1385
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1380
.L1385:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L1380
.L1382:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1380
.L1378:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1387
	testb	$32, %dh
	jne	.L1387
	movq	-8(%rbx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1380
	movzwl	-16(%rbx), %eax
	cmpw	$64, %ax
	je	.L1394
	cmpw	$64, %ax
	jg	.L1398
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1392
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1393
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1380
	.p2align 4,,7
	jmp	.L1391
.L1398:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1396
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1397
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1380
	.p2align 4,,7
	jmp	.L1395
.L1392:
	movq	-8(%rbx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1380
.L1396:
	movq	-8(%rbx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1380
.L1391:
	movq	-8(%rbx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1380
	call	dealloc_array
	jmp	.L1380
.L1395:
	movq	-8(%rbx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1380
	call	free
	jmp	.L1380
.L1393:
	movq	-8(%rbx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1380
.L1394:
	movq	-8(%rbx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1380
.L1397:
	movq	-8(%rbx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1380
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1380
.L1387:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1380
	.p2align 4,,9
	call	*-8(%rbx)
.L1380:
	movq	(%r13), %rax
	movq	%rax, -16(%rbx)
	movq	8(%r13), %rax
	movq	%rax, -8(%rbx)
	subl	$1, %ebp
	addq	$16, %rbx
	cmpl	$-1, %ebp
	je	.L1375
	addq	$16, %r13
	jmp	.L1377
.L1375:
	movq	8(%rsp), %rdi
	call	free_empty_array
	.p2align 4,,3
	jmp	.L1504
.L1404:
	movq	%rbx, %rdi
	call	int_free_svalue
	movq	%r12, %rdx
	cmpw	$16, -16(%r12)
	.p2align 4,,2
	jne	.L1405
	movq	-8(%r12), %rax
	testq	%rax, %rax
	je	.L1407
	testb	$16, 2(%rax)
	je	.L1405
.L1407:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L1409
.L1405:
	movq	-16(%r12), %rax
	movq	%rax, (%rbx)
	movq	-8(%r12), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L1410
	cmpw	$8192, %ax
	je	.L1410
	andb	$223, %ah
	movw	%ax, (%rbx)
.L1410:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1413
	testb	$1, -14(%rdx)
	je	.L1409
	movq	%rbx, %rcx
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1416
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1416:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rcx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1409
.L1413:
	testl	$17272, %eax
	je	.L1409
	movq	-8(%r12), %rax
	addw	$1, (%rax)
.L1409:
	addq	$16, %rbx
	addq	$16, %r12
.L1374:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L1404
	movq	8(%rsp), %rax
	subw	$1, (%rax)
	jmp	.L1504
.L1370:
	movq	8(%r15), %rdx
	movq	%rdx, 16(%rsp)
	addq	$8, %rdx
	movq	%rdx, 24(%rsp)
	movl	%r14d, %r12d
	subl	4(%rsp), %r12d
	leal	(%rbp,%r12), %edi
	addl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %r14
	leaq	8(%rax), %r10
	movq	16(%rsp), %rcx
	addq	$24, %rcx
	leaq	24(%rax), %rsi
	movl	%ebx, %edi
	movl	$-1, %r9d
	jmp	.L1420
.L1421:
	movq	%rcx, %rdx
	cmpw	$16, -16(%rcx)
	jne	.L1422
	movq	-8(%rcx), %rax
	testq	%rax, %rax
	je	.L1424
	testb	$16, 2(%rax)
	je	.L1422
.L1424:
	movq	const0u(%rip), %rax
	movq	%rax, -16(%rsi)
	movq	const0u+8(%rip), %rax
	movq	%rax, -8(%rsi)
	jmp	.L1426
.L1422:
	movq	-16(%rcx), %rax
	movq	%rax, -16(%rsi)
	movq	-8(%rcx), %rax
	movq	%rax, -8(%rsi)
	movzwl	-16(%rsi), %eax
	testb	$32, %ah
	je	.L1427
	cmpw	$8192, %ax
	je	.L1427
	andb	$223, %ah
	movw	%ax, -16(%rsi)
.L1427:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1430
	testb	$1, -14(%rdx)
	je	.L1426
	movq	%rsi, %r8
	movq	-8(%rsi), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1433
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1433:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	-8(%r8), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1426
.L1430:
	testl	$17272, %eax
	je	.L1426
	movq	-8(%rcx), %rax
	addw	$1, (%rax)
.L1426:
	addq	$16, %rcx
	addq	$16, %rsi
.L1420:
	subl	$1, %edi
	cmpl	%r9d, %edi
	jne	.L1421
	mov	%ebx, %eax
	salq	$4, %rax
	leaq	(%r10,%rax), %rbx
	movq	8(%rsp), %rax
	leaq	16(%r13), %rcx
	leaq	16(%rbx), %rsi
	cmpw	$1, (%rax)
	jne	.L1439
	leal	-1(%rbp), %edx
	cmpl	$-1, %edx
	je	.L1440
	movl	$-1, %ecx
.L1441:
	movq	(%r13), %rax
	movq	%rax, (%rbx)
	movq	8(%r13), %rax
	movq	%rax, 8(%rbx)
	addq	$16, %rbx
	addq	$16, %r13
	subl	$1, %edx
	cmpl	%ecx, %edx
	jne	.L1441
.L1440:
	movq	8(%rsp), %rdi
	call	free_empty_array
	jmp	.L1442
.L1443:
	movq	%rcx, %rdx
	cmpw	$16, -16(%rcx)
	jne	.L1444
	movq	-8(%rcx), %rax
	testq	%rax, %rax
	.p2align 4,,3
	je	.L1446
	testb	$16, 2(%rax)
	je	.L1444
.L1446:
	movq	const0u(%rip), %rax
	movq	%rax, -16(%rsi)
	movq	const0u+8(%rip), %rax
	movq	%rax, -8(%rsi)
	jmp	.L1448
.L1444:
	movq	-16(%rcx), %rax
	movq	%rax, -16(%rsi)
	movq	-8(%rcx), %rax
	movq	%rax, -8(%rsi)
	movzwl	-16(%rsi), %eax
	testb	$32, %ah
	je	.L1449
	cmpw	$8192, %ax
	je	.L1449
	andb	$223, %ah
	movw	%ax, -16(%rsi)
.L1449:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1452
	testb	$1, -14(%rdx)
	je	.L1448
	movq	%rsi, %rdi
	movq	-8(%rsi), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1455
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1455:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	-8(%rdi), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1448
.L1452:
	testl	$17272, %eax
	je	.L1448
	movq	-8(%rcx), %rax
	addw	$1, (%rax)
.L1448:
	addq	$16, %rcx
	addq	$16, %rsi
.L1439:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L1443
	leaq	-16(%rsi), %rbx
	movq	8(%rsp), %rdx
	subw	$1, (%rdx)
.L1442:
	movslq	4(%rsp),%rax
	salq	$4, %rax
	movq	24(%rsp), %rdx
	leaq	16(%rdx,%rax), %rcx
	leaq	16(%rbx), %rsi
	movl	$-1, %r8d
	jmp	.L1459
.L1460:
	movq	%rcx, %rdx
	cmpw	$16, -16(%rcx)
	jne	.L1461
	movq	-8(%rcx), %rax
	testq	%rax, %rax
	je	.L1463
	testb	$16, 2(%rax)
	je	.L1461
.L1463:
	movq	const0u(%rip), %rax
	movq	%rax, -16(%rsi)
	movq	const0u+8(%rip), %rax
	movq	%rax, -8(%rsi)
	jmp	.L1465
.L1461:
	movq	-16(%rcx), %rax
	movq	%rax, -16(%rsi)
	movq	-8(%rcx), %rax
	movq	%rax, -8(%rsi)
	movzwl	-16(%rsi), %eax
	testb	$32, %ah
	je	.L1466
	cmpw	$8192, %ax
	je	.L1466
	andb	$223, %ah
	movw	%ax, -16(%rsi)
.L1466:
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	jne	.L1469
	testb	$1, -14(%rdx)
	je	.L1465
	movq	%rsi, %rdi
	movq	-8(%rsi), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1472
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1472:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	-8(%rdi), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1465
.L1469:
	testl	$17272, %eax
	je	.L1465
	movq	-8(%rcx), %rax
	addw	$1, (%rax)
.L1465:
	addq	$16, %rcx
	addq	$16, %rsi
.L1459:
	subl	$1, %r12d
	cmpl	%r8d, %r12d
	jne	.L1460
	movq	16(%rsp), %rdi
	call	free_array
	movq	%r14, 8(%r15)
	jmp	.L1504
.L1365:
	cmpw	$4, (%rdi)
	je	.L1476
	movl	$.LC67, %edi
	movl	$0, %eax
	call	error
.L1476:
	testb	$1, 2(%rbp)
	je	.L1478
	movq	8(%rbp), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %r12d
	movl	%r12d, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L1480
	movq	8(%rbp), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %r12d
	jmp	.L1480
.L1478:
	movq	8(%rbp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %r12d
.L1480:
	movl	4(%rsp), %eax
	subl	%ebx, %eax
	cmpl	%eax, %r12d
	jne	.L1482
	movslq	%r12d,%rdx
	movq	8(%rbp), %rsi
	movslq	%ebx,%rdi
	addq	8(%r15), %rdi
	call	strncpy
	jmp	.L1484
.L1482:
	movq	8(%r15), %rax
	movq	%rax, 32(%rsp)
	subl	4(%rsp), %r14d
	leal	(%r14,%rbx), %edi
	addl	%r12d, %edi
	call	int_new_string
	movq	%rax, %r13
	movq	%rax, 8(%r15)
	testl	%ebx, %ebx
	jle	.L1485
	movslq	%ebx,%rbx
	movq	%rbx, %rdx
	movq	32(%rsp), %rsi
	movq	%rax, %rdi
	call	strncpy
	addq	%rbx, %r13
.L1485:
	movq	8(%rbp), %rsi
	movq	%r13, %rdi
	call	strcpy
	testl	%r14d, %r14d
	jle	.L1487
	movslq	%r12d,%rbx
	addq	%r13, %rbx
	movslq	%r14d,%r12
	movslq	4(%rsp),%rsi
	addq	32(%rsp), %rsi
	movq	%r12, %rdx
	movq	%rbx, %rdi
	call	strncpy
	movb	$0, (%r12,%rbx)
.L1487:
	movq	32(%rsp), %rdi
	subq	$4, %rdi
	movq	32(%rsp), %rdx
	movzwl	-4(%rdx), %eax
	movl	%eax, svalue_strlen_size(%rip)
	subl	$1, num_distinct_strings(%rip)
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	call	free
	subl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	subl	svalue_strlen_size(%rip), %eax
	subl	$1, %eax
	movl	%eax, allocd_bytes(%rip)
.L1484:
	movq	8(%rbp), %rdi
	testb	$1, 2(%rbp)
	je	.L1504
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1490
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1490
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rbp)
	je	.L1493
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1504
.L1493:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L1504
.L1490:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1504
.L1367:
	cmpw	$256, (%rdi)
	je	.L1495
	movl	$.LC68, %edi
	movl	$0, %eax
	call	error
.L1495:
	movq	8(%rbp), %rcx
	movl	4(%rcx), %r13d
	movl	4(%rsp), %eax
	subl	%ebx, %eax
	cmpl	%eax, %r13d
	jne	.L1497
	movq	8(%r15), %rdi
	addq	$8, %rdi
	movslq	%ebx,%rax
	addq	%rax, %rdi
	movslq	%r13d,%rdx
	leaq	8(%rcx), %rsi
	call	memcpy
	jmp	.L1499
.L1497:
	movq	8(%r15), %rax
	addq	$8, %rax
	movq	%rax, 48(%rsp)
	subl	4(%rsp), %r14d
	leal	(%r13,%r14), %edi
	addl	%ebx, %edi
	call	allocate_buffer
	movq	%rax, 40(%rsp)
	movq	%rax, %r12
	addq	$8, %r12
	testl	%ebx, %ebx
	jle	.L1500
	movslq	%ebx,%rbx
	movq	%rbx, %rdx
	movq	48(%rsp), %rsi
	movq	%r12, %rdi
	call	memcpy
	addq	%rbx, %r12
.L1500:
	movslq	%r13d,%rbx
	movq	8(%rbp), %rsi
	movq	%rbx, %rdx
	movq	%r12, %rdi
	call	memcpy
	testl	%r14d, %r14d
	jle	.L1502
	leaq	(%rbx,%r12), %rdi
	movslq	%r14d,%rdx
	movslq	4(%rsp),%rsi
	addq	48(%rsp), %rsi
	call	memcpy
.L1502:
	movq	8(%r15), %rdi
	call	free_buffer
	movq	40(%rsp), %rdx
	movq	%rdx, 8(%r15)
.L1499:
	movq	8(%rbp), %rdi
	call	free_buffer
.L1504:
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE74:
	.size	copy_lvalue_range, .-copy_lvalue_range
	.section	.rodata.str1.8
	.align 8
.LC69:
	.string	"Mapping exceeded maximum allowed size of %d.\n"
	.section	.rodata.str1.1
.LC70:
	.string	"Illegal type of index\n"
	.section	.rodata.str1.8
	.align 8
.LC71:
	.string	"Index out of bounds in string index lvalue.\n"
	.section	.rodata.str1.1
.LC72:
	.string	"Buffer index out of bounds.\n"
.LC73:
	.string	"Array index out of bounds\n"
.LC74:
	.string	"Value being indexed is zero.\n"
	.section	.rodata.str1.8
	.align 8
.LC75:
	.string	"Cannot index value of type '%s'.\n"
	.align 8
.LC76:
	.string	"Illegal to make char lvalue from assigned string\n"
	.section	.rodata.str1.1
.LC77:
	.string	"Array index out of bounds.\n"
	.text
.globl push_indexed_lvalue
	.type	push_indexed_lvalue, @function
push_indexed_lvalue:
.LFB72:
	movq	%rbx, -24(%rsp)
.LCFI181:
	movq	%rbp, -16(%rsp)
.LCFI182:
	movq	%r12, -8(%rsp)
.LCFI183:
	subq	$24, %rsp
.LCFI184:
	movl	%edi, %ebx
	movq	sp(%rip), %rdx
	movzwl	(%rdx), %eax
	cmpw	$1, %ax
	jne	.L1507
	movq	8(%rdx), %rbp
	testl	%edi, %edi
	jne	.L1509
	cmpw	$32, (%rbp)
	jne	.L1509
	leaq	-16(%rdx), %rsi
	movq	%rsi, sp(%rip)
	movq	8(%rbp), %rdi
	movl	$0, %edx
	call	find_for_insert
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L1512
	movl	config_int+52(%rip), %esi
	movl	$.LC69, %edi
	movl	$0, %eax
	call	error
.L1512:
	movq	sp(%rip), %rcx
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1514
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1516
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1518
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1518
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1521
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1516
.L1521:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1516
.L1518:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1516
.L1514:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1523
	testb	$32, %dh
	jne	.L1523
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1516
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1530
	cmpw	$64, %ax
	jg	.L1534
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1528
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1529
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1516
	.p2align 4,,7
	jmp	.L1527
.L1534:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1532
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1533
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1516
	.p2align 4,,7
	jmp	.L1531
.L1528:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1516
.L1532:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1516
.L1527:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1516
	call	dealloc_array
	jmp	.L1516
.L1531:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1516
	call	free
	jmp	.L1516
.L1529:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1516
.L1530:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1516
.L1533:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1516
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1516
.L1523:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1516
	.p2align 4,,9
	call	*8(%rcx)
.L1516:
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movl	$32, lv_owner_type(%rip)
	movq	8(%rbx), %rax
	movq	%rax, lv_owner(%rip)
	jmp	.L1617
.L1509:
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L1540
	movl	$.LC70, %edi
	movl	$0, %eax
	call	error
.L1540:
	movq	sp(%rip), %rax
	movl	8(%rax), %r12d
	movzwl	(%rbp), %eax
	cmpw	$8, %ax
	je	.L1544
	cmpw	$256, %ax
	je	.L1545
	cmpw	$4, %ax
	jne	.L1542
	testb	$1, 2(%rbp)
	.p2align 4,,5
	je	.L1546
	movq	8(%rbp), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %edx
	movl	%edx, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L1548
	movq	8(%rbp), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %edx
	jmp	.L1548
.L1546:
	movq	8(%rbp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %edx
.L1548:
	movl	%edx, %eax
	subl	%r12d, %eax
	testl	%ebx, %ebx
	cmovne	%eax, %r12d
	cmpl	%r12d, %edx
	jle	.L1554
	testl	%r12d, %r12d
	jns	.L1552
.L1554:
	movl	$.LC71, %edi
	movl	$0, %eax
	call	error
.L1552:
	movq	%rbp, %rdi
	call	unlink_string_svalue
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	$global_lvalue_byte, 8(%rax)
	movw	$0, global_lvalue_byte+2(%rip)
	movslq	%r12d,%rax
	addq	8(%rbp), %rax
	movq	%rax, global_lvalue_byte+8(%rip)
	movl	$4, lv_owner_type(%rip)
	movq	8(%rbp), %rax
	movq	%rax, lv_owner(%rip)
	jmp	.L1617
.L1545:
	testl	%ebx, %ebx
	je	.L1555
	movq	8(%rbp), %rax
	movl	4(%rax), %eax
	subl	%r12d, %eax
	movl	%eax, %r12d
.L1555:
	movq	8(%rbp), %rax
	cmpl	4(%rax), %r12d
	jae	.L1559
	testl	%r12d, %r12d
	jns	.L1557
.L1559:
	movl	$.LC72, %edi
	movl	$0, %eax
	call	error
.L1557:
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	$global_lvalue_byte, 8(%rax)
	movw	$1, global_lvalue_byte+2(%rip)
	movq	8(%rbp), %rax
	addq	$8, %rax
	movslq	%r12d,%rdx
	addq	%rdx, %rax
	movq	%rax, global_lvalue_byte+8(%rip)
	movl	$256, lv_owner_type(%rip)
	movq	8(%rbp), %rax
	movq	%rax, lv_owner(%rip)
	jmp	.L1617
.L1544:
	testl	%ebx, %ebx
	je	.L1560
	movq	8(%rbp), %rax
	movzwl	2(%rax), %eax
	subl	%r12d, %eax
	movl	%eax, %r12d
.L1560:
	movq	8(%rbp), %rax
	movzwl	2(%rax), %eax
	cmpl	%r12d, %eax
	jle	.L1564
	testl	%r12d, %r12d
	jns	.L1562
.L1564:
	movl	$.LC73, %edi
	movl	$0, %eax
	call	error
.L1562:
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	8(%rbp), %rax
	addq	$8, %rax
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	movl	$8, lv_owner_type(%rip)
	movq	8(%rbp), %rax
	movq	%rax, lv_owner(%rip)
	jmp	.L1617
.L1542:
	cmpw	$2, %ax
	jne	.L1565
	cmpq	$0, 8(%rbp)
	jne	.L1565
	movl	$.LC74, %edi
	movl	$0, %eax
	call	error
.L1565:
	movswl	(%rbp),%edi
	call	type_name
	movq	%rax, %rsi
	movl	$.LC75, %edi
	movl	$0, %eax
	call	error
	jmp	.L1617
.L1507:
	testl	%edi, %edi
	jne	.L1568
	cmpw	$32, %ax
	.p2align 4,,7
	jne	.L1568
	leaq	-16(%rdx), %rsi
	movq	8(%rdx), %rdi
	movl	$0, %edx
	call	find_for_insert
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L1571
	movl	config_int+52(%rip), %esi
	movl	$.LC69, %edi
	movl	$0, %eax
	call	error
.L1571:
	movq	sp(%rip), %rax
	movq	8(%rax), %rax
	subw	$1, (%rax)
	movl	$32, lv_owner_type(%rip)
	movq	sp(%rip), %rax
	movq	8(%rax), %rax
	movq	%rax, lv_owner(%rip)
	movq	sp(%rip), %rcx
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1573
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1575
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1577
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1577
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1580
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1575
.L1580:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1575
.L1577:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1575
.L1573:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1582
	testb	$32, %dh
	jne	.L1582
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1575
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1589
	cmpw	$64, %ax
	jg	.L1593
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1587
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1588
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1575
	.p2align 4,,7
	jmp	.L1586
.L1593:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1591
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1592
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1575
	.p2align 4,,7
	jmp	.L1590
.L1587:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1575
.L1591:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1575
.L1586:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1575
	call	dealloc_array
	jmp	.L1575
.L1590:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1575
	call	free
	jmp	.L1575
.L1588:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1575
.L1589:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1575
.L1592:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1575
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1575
.L1582:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1575
	.p2align 4,,9
	call	*8(%rcx)
.L1575:
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L1617
.L1568:
	cmpw	$2, -16(%rdx)
	je	.L1598
	movl	$.LC70, %edi
	movl	$0, %eax
	call	error
.L1598:
	movq	sp(%rip), %rdx
	movl	-8(%rdx), %ebp
	movzwl	(%rdx), %eax
	cmpw	$8, %ax
	je	.L1602
	cmpw	$256, %ax
	je	.L1603
	cmpw	$4, %ax
	jne	.L1600
	movl	$.LC76, %edi
	movl	$0, %eax
	call	error
	jmp	.L1617
.L1603:
	testl	%ebx, %ebx
	je	.L1604
	movq	8(%rdx), %rax
	movl	4(%rax), %eax
	subl	%ebp, %eax
	movl	%eax, %ebp
.L1604:
	movq	8(%rdx), %rax
	cmpl	4(%rax), %ebp
	jae	.L1608
	testl	%ebp, %ebp
	jns	.L1606
.L1608:
	movl	$.LC72, %edi
	movl	$0, %eax
	call	error
.L1606:
	movq	sp(%rip), %rax
	movq	8(%rax), %rax
	subw	$1, (%rax)
	movl	$256, lv_owner_type(%rip)
	movq	sp(%rip), %rdx
	movq	8(%rdx), %rax
	movq	%rax, lv_owner(%rip)
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, -16(%rdx)
	movq	sp(%rip), %rax
	movq	$global_lvalue_byte, 8(%rax)
	movw	$1, global_lvalue_byte+2(%rip)
	movq	sp(%rip), %rax
	movq	24(%rax), %rax
	addq	$8, %rax
	movslq	%ebp,%rdx
	addq	%rdx, %rax
	movq	%rax, global_lvalue_byte+8(%rip)
	jmp	.L1617
.L1602:
	testl	%ebx, %ebx
	je	.L1609
	movq	8(%rdx), %rax
	movzwl	2(%rax), %eax
	subl	%ebp, %eax
	movl	%eax, %ebp
.L1609:
	movq	8(%rdx), %rax
	movzwl	2(%rax), %eax
	cmpl	%ebp, %eax
	jle	.L1613
	testl	%ebp, %ebp
	jns	.L1611
.L1613:
	movl	$.LC77, %edi
	movl	$0, %eax
	call	error
.L1611:
	movq	sp(%rip), %rax
	movq	8(%rax), %rax
	subw	$1, (%rax)
	movl	$8, lv_owner_type(%rip)
	movq	sp(%rip), %rdx
	movq	8(%rdx), %rax
	movq	%rax, lv_owner(%rip)
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, -16(%rdx)
	movq	sp(%rip), %rdx
	movslq	%ebp,%rax
	salq	$4, %rax
	addq	24(%rdx), %rax
	addq	$8, %rax
	movq	%rax, 8(%rdx)
	jmp	.L1617
.L1600:
	cmpw	$2, %ax
	jne	.L1614
	cmpq	$0, 8(%rdx)
	jne	.L1614
	movl	$.LC74, %edi
	movl	$0, %eax
	call	error
.L1614:
	movq	sp(%rip), %rax
	movswl	(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movl	$.LC75, %edi
	movl	$0, %eax
	call	error
.L1617:
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	16(%rsp), %r12
	addq	$24, %rsp
	ret
.LFE72:
	.size	push_indexed_lvalue, .-push_indexed_lvalue
	.section	.rodata.str1.8
	.align 8
.LC78:
	.string	"object /%s: eval_cost too big %d\n"
	.align 8
.LC79:
	.string	"Too long evaluation. Execution aborted.\n"
	.align 8
.LC81:
	.string	"Strings cannot contain 0 bytes.\n"
	.section	.rodata.str1.1
.LC82:
	.string	"++ of non-numeric argument\n"
.LC83:
	.string	"-- of non-numeric argument\n"
.LC84:
	.string	"Reference is invalid.\n"
	.section	.rodata.str1.8
	.align 8
.LC85:
	.string	"Non-reference value passed as reference argument.\n"
	.section	.rodata.str1.1
.LC86:
	.string	"do_loop_cond:1"
.LC87:
	.string	"do_loop_cond:2"
	.section	.rodata.str1.8
	.align 8
.LC88:
	.string	"2nd argument to < is not numeric when the 1st is.\n"
	.align 8
.LC89:
	.string	"2nd argument to < is not string when the 1st is.\n"
	.section	.rodata.str1.1
.LC90:
	.string	"Bad 1st argument to <.\n"
	.section	.rodata.str1.8
	.align 8
.LC91:
	.string	"Right side of < is a number, left side is not.\n"
	.align 8
.LC92:
	.string	"Bad type argument to +. Had %s and %s.\n"
	.section	.rodata.str1.1
.LC93:
	.string	"%ld"
	.section	.rodata.str1.8
	.align 8
.LC94:
	.string	"Maximum string length exceeded in concatenation.\n"
	.section	.rodata.str1.1
.LC95:
	.string	"Out of memory!\n"
	.section	.rodata.str1.8
	.align 8
.LC96:
	.string	"Bad type argument to +.  Had %s and %s.\n"
	.section	.rodata.str1.1
.LC97:
	.string	"%f"
	.section	.rodata.str1.8
	.align 8
.LC98:
	.string	"Bad type argument to +. Had %s and %s\n"
	.align 8
.LC99:
	.string	"Left hand side of += is a number (or zero); right side is not a number.\n"
	.align 8
.LC100:
	.string	"Bad right type to += of char lvalue.\n"
	.align 8
.LC101:
	.string	"Item being expanded with ... is not an array\n"
	.section	.rodata.str1.1
.LC102:
	.string	"Illegal rhs to char lvalue\n"
	.section	.rodata.str1.8
	.align 8
.LC103:
	.string	"Undefined function called: %s\n"
	.section	.rodata.str1.1
.LC104:
	.string	"Bad argument to ~\n"
.LC105:
	.string	"Division by zero\n"
.LC107:
	.string	"Division by 0.0\n"
	.section	.rodata.str1.8
	.align 8
.LC108:
	.string	"Tried to take a member of something that isn't a class.\n"
	.align 8
.LC109:
	.string	"Class has no corresponding member.\n"
	.align 8
.LC110:
	.string	"Buffer indexes must be integers.\n"
	.align 8
.LC111:
	.string	"String indexes must be integers.\n"
	.section	.rodata.str1.1
.LC112:
	.string	"String index out of bounds.\n"
	.section	.rodata.str1.8
	.align 8
.LC113:
	.string	"Array indexes must be integers.\n"
	.align 8
.LC114:
	.string	"Array index must be positive or zero.\n"
	.align 8
.LC115:
	.string	"Indexing a buffer with an illegal type.\n"
	.align 8
.LC116:
	.string	"Indexing a string with an illegal type.\n"
	.align 8
.LC117:
	.string	"Indexing an array with an illegal type\n"
	.section	.rodata.str1.1
.LC118:
	.string	"Modulus by zero.\n"
	.section	.rodata.str1.8
	.align 8
.LC119:
	.string	"Args to * are not compatible.\n"
	.section	.rodata.str1.1
.LC121:
	.string	"Bad argument to unary minus\n"
.LC122:
	.string	"Range lvalue on illegal type\n"
	.section	.rodata.str1.8
	.align 8
.LC123:
	.string	"Illegal 2nd index type to range lvalue\n"
	.align 8
.LC124:
	.string	"The 2nd index to range lvalue must be >= -1 and < sizeof(indexed value)\n"
	.align 8
.LC125:
	.string	"Illegal 1st index type to range lvalue\n"
	.align 8
.LC126:
	.string	"The 1st index to range lvalue must be >= 0 and <= sizeof(indexed value)\n"
	.section	.rodata.str1.1
.LC127:
	.string	"Bad left type to -.\n"
.LC128:
	.string	"Bad right type to -.\n"
	.section	.rodata.str1.8
	.align 8
.LC129:
	.string	"Arguments to - do not have compatible types.\n"
	.section	.rodata.str1.1
.LC130:
	.string	"Trying to put %s in %s\n"
	.section	.rodata.cst4
	.align 4
.LC80:
	.long	1065353216
	.align 4
.LC106:
	.long	0
	.text
.globl eval_instruction
	.type	eval_instruction, @function
eval_instruction:
.LFB103:
	pushq	%r15
.LCFI185:
	pushq	%r14
.LCFI186:
	pushq	%r13
.LCFI187:
	pushq	%r12
.LCFI188:
	pushq	%rbp
.LCFI189:
	pushq	%rbx
.LCFI190:
	subq	$136, %rsp
.LCFI191:
	movq	csp(%rip), %rax
	orw	$8, (%rax)
	movq	%rdi, pc(%rip)
.L3363:
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebp
	movzbl	%bpl, %ebx
	leaq	1(%rax), %rdx
	movq	%rdx, pc(%rip)
	leal	-3(%rbx), %eax
	movl	%ebx, %edi
	cmpl	$4, %eax
	ja	.L1622
	movzbl	(%rdx), %eax
	leal	232(%rax), %edi
.L1622:
	movl	last(%rip), %ecx
	movslq	%ecx,%rsi
	movl	%edi, previous_instruction(,%rsi,4)
	movq	pc(%rip), %rax
	subq	$1, %rax
	movq	%rax, previous_pc(,%rsi,8)
	movq	sp(%rip), %rax
	subq	fp(%rip), %rax
	sarq	$4, %rax
	movq	csp(%rip), %rdx
	subl	40(%rdx), %eax
	movl	%eax, stack_size(,%rsi,4)
	addl	$1, %ecx
	movslq	%ecx,%rcx
	movabsq	$-8608480567731124087, %rdx
	movq	%rcx, %rax
	mulq	%rdx
	shrq	$5, %rdx
	leaq	0(,%rdx,4), %rax
	salq	$6, %rdx
	subq	%rax, %rdx
	subq	%rdx, %rcx
	movl	%ecx, last(%rip)
	cmpl	$0, outoftime(%rip)
	je	.L1623
	movq	current_object(%rip), %rax
	movq	8(%rax), %rsi
	movl	max_cost(%rip), %edx
	movl	$.LC78, %edi
	movl	$0, %eax
	call	debug_message
	movl	max_cost(%rip), %edi
	call	set_eval
	movl	$1, max_eval_error(%rip)
	movl	$.LC79, %edi
	movl	$0, %eax
	call	error
.L1623:
	cmpb	$119, %bpl
	ja	.L1625
	movzbl	%bpl, %eax
	jmp	*.L1744(,%rax,8)
	.section	.rodata
	.align 8
	.align 4
.L1744:
	.quad	.L1625
	.quad	.L1626
	.quad	.L1627
	.quad	.L1628
	.quad	.L1629
	.quad	.L1630
	.quad	.L1631
	.quad	.L1632
	.quad	.L1633
	.quad	.L1634
	.quad	.L1635
	.quad	.L1636
	.quad	.L1637
	.quad	.L1638
	.quad	.L1639
	.quad	.L1640
	.quad	.L1641
	.quad	.L1642
	.quad	.L1643
	.quad	.L1644
	.quad	.L1645
	.quad	.L1646
	.quad	.L1647
	.quad	.L1648
	.quad	.L1649
	.quad	.L1650
	.quad	.L1651
	.quad	.L1652
	.quad	.L1653
	.quad	.L1654
	.quad	.L1655
	.quad	.L1656
	.quad	.L1657
	.quad	.L1658
	.quad	.L1659
	.quad	.L1660
	.quad	.L1661
	.quad	.L1662
	.quad	.L1663
	.quad	.L1664
	.quad	.L1665
	.quad	.L1666
	.quad	.L1667
	.quad	.L1668
	.quad	.L1669
	.quad	.L1670
	.quad	.L1671
	.quad	.L1672
	.quad	.L1673
	.quad	.L1674
	.quad	.L1675
	.quad	.L1676
	.quad	.L1677
	.quad	.L1678
	.quad	.L1679
	.quad	.L1680
	.quad	.L1681
	.quad	.L1682
	.quad	.L1683
	.quad	.L1684
	.quad	.L1685
	.quad	.L1686
	.quad	.L1687
	.quad	.L1688
	.quad	.L1689
	.quad	.L1690
	.quad	.L1691
	.quad	.L1692
	.quad	.L1693
	.quad	.L1694
	.quad	.L1695
	.quad	.L1696
	.quad	.L1697
	.quad	.L1698
	.quad	.L1699
	.quad	.L1700
	.quad	.L1701
	.quad	.L1702
	.quad	.L1703
	.quad	.L1704
	.quad	.L1705
	.quad	.L1706
	.quad	.L1707
	.quad	.L1708
	.quad	.L1709
	.quad	.L1710
	.quad	.L1711
	.quad	.L1712
	.quad	.L1713
	.quad	.L1714
	.quad	.L1715
	.quad	.L1716
	.quad	.L1717
	.quad	.L1718
	.quad	.L1719
	.quad	.L1720
	.quad	.L1710
	.quad	.L1721
	.quad	.L1722
	.quad	.L1723
	.quad	.L1724
	.quad	.L1725
	.quad	.L1726
	.quad	.L1727
	.quad	.L1728
	.quad	.L1729
	.quad	.L1730
	.quad	.L1731
	.quad	.L1732
	.quad	.L1733
	.quad	.L1734
	.quad	.L1735
	.quad	.L1736
	.quad	.L1737
	.quad	.L1738
	.quad	.L1739
	.quad	.L1740
	.quad	.L1741
	.quad	.L1742
	.quad	.L1743
	.text
.L1627:
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebp
	addq	$1, %rax
	movq	%rax, pc(%rip)
	jmp	.L3365
.L1746:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	movq	%rdx, 120(%rsp)
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	%rdx, %rbx
	movq	%rdx, %rax
	andl	$192, %eax
	cmpq	$64, %rax
	je	.L1748
	cmpq	$64, %rax
	jg	.L1751
	testq	%rax, %rax
	je	.L1747
	.p2align 4,,7
	jmp	.L3365
.L1751:
	cmpq	$128, %rax
	.p2align 4,,7
	je	.L1749
	cmpq	$192, %rax
	.p2align 4,,7
	jne	.L3365
	.p2align 4,,7
	jmp	.L1750
.L1747:
	andl	$63, %edx
	movq	current_prog(%rip), %rax
	movq	80(%rax), %rax
	movq	(%rax,%rdx,8), %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1752
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1752:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$4, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	movw	$3, 2(%rax)
	movq	%rbx, %rdi
	call	ref_string
	jmp	.L3365
.L1750:
	movq	%rdx, %rax
	andl	$63, %eax
	salq	$4, %rax
	movq	%rax, %rbx
	addq	fp(%rip), %rbx
	cmpw	$16, (%rbx)
	jne	.L1754
	movq	8(%rbx), %rax
	testb	$16, 2(%rax)
	je	.L1754
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L1757
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L1759
	testb	$16, 2(%rax)
	je	.L1757
.L1759:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L1754
.L1757:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L1761
	cmpw	$8192, %ax
	je	.L1761
	andb	$223, %ah
	movw	%ax, (%rbx)
.L1761:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L1764
	testb	$1, const0u+2(%rip)
	je	.L1754
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1767
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1767:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1754
.L1764:
	testl	$17272, %eax
	je	.L1754
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L1754:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1770
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1770:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	cmpw	$16, (%rbx)
	jne	.L1772
	movq	8(%rbx), %rax
	testq	%rax, %rax
	je	.L1774
	testb	$16, 2(%rax)
	je	.L1772
.L1774:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3365
.L1772:
	movq	(%rbx), %rax
	movq	%rax, (%rdx)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L1776
	cmpw	$8192, %ax
	je	.L1776
	andb	$223, %ah
	movw	%ax, (%rdx)
.L1776:
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L1779
	testb	$1, 2(%rbx)
	je	.L3365
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L1782
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L1782:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3365
.L1779:
	testl	$17272, %eax
	je	.L3365
	movq	8(%rbx), %rax
	addw	$1, (%rax)
	jmp	.L3365
.L1749:
	movl	%edx, %eax
	andl	$63, %eax
	addl	variable_index_offset(%rip), %eax
	cltq
	salq	$4, %rax
	addq	$112, %rax
	addq	current_object(%rip), %rax
	leaq	8(%rax), %rbx
	cmpw	$16, 8(%rax)
	jne	.L1785
	movq	8(%rbx), %rax
	testb	$16, 2(%rax)
	je	.L1785
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L1788
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L1790
	testb	$16, 2(%rax)
	je	.L1788
.L1790:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L1785
.L1788:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L1792
	cmpw	$8192, %ax
	je	.L1792
	andb	$223, %ah
	movw	%ax, (%rbx)
.L1792:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L1795
	testb	$1, const0u+2(%rip)
	je	.L1785
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1798
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1798:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1785
.L1795:
	testl	$17272, %eax
	je	.L1785
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L1785:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1801
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1801:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	cmpw	$16, (%rbx)
	jne	.L1803
	movq	8(%rbx), %rax
	testq	%rax, %rax
	je	.L1805
	testb	$16, 2(%rax)
	je	.L1803
.L1805:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3365
.L1803:
	movq	(%rbx), %rax
	movq	%rax, (%rdx)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L1807
	cmpw	$8192, %ax
	je	.L1807
	andb	$223, %ah
	movw	%ax, (%rdx)
.L1807:
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L1810
	testb	$1, 2(%rbx)
	je	.L3365
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L1813
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L1813:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3365
.L1810:
	testl	$17272, %eax
	je	.L3365
	movq	8(%rbx), %rax
	addw	$1, (%rax)
	jmp	.L3365
.L1748:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1816
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1816:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	%rbx, %rdx
	andl	$63, %edx
	movq	sp(%rip), %rax
	movq	%rdx, 8(%rax)
.L3365:
	subq	$1, %rbp
	cmpq	$-1, %rbp
	jne	.L1746
	jmp	.L3363
.L1679:
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	movzwl	(%rdx), %eax
	cmpw	$128, %ax
	je	.L1820
	cmpw	$1024, %ax
	je	.L1821
	cmpw	$2, %ax
	jne	.L1818
	addq	$1, 8(%rdx)
	.p2align 4,,5
	jmp	.L3363
.L1820:
	movss	.LC80(%rip), %xmm0
	addss	8(%rdx), %xmm0
	movss	%xmm0, 8(%rdx)
	jmp	.L3363
.L1821:
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L1822
	movq	global_lvalue_byte+8(%rip), %rax
	cmpb	$-1, (%rax)
	jne	.L1822
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L1822:
	movq	global_lvalue_byte+8(%rip), %rax
	addb	$1, (%rax)
	jmp	.L3363
.L1818:
	movl	$.LC82, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1661:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rcx
	addq	fp(%rip), %rcx
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	movzwl	(%rcx), %eax
	cmpw	$2, %ax
	jne	.L1825
	movq	8(%rcx), %rax
	movq	%rax, 120(%rsp)
	subq	$1, %rax
	movq	%rax, 8(%rcx)
	jmp	.L1827
.L1825:
	cmpw	$128, %ax
	jne	.L1828
	movss	8(%rcx), %xmm0
	cvttss2siq	%xmm0, %rax
	movq	%rax, 120(%rsp)
	subss	.LC80(%rip), %xmm0
	movss	%xmm0, 8(%rcx)
	jmp	.L1827
.L1828:
	movl	$.LC83, %edi
	movl	$0, %eax
	call	error
.L1827:
	cmpq	$0, 120(%rsp)
	je	.L1830
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1830:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1689:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1832
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1832:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, 16(%rdx)
	movq	pc(%rip), %rcx
	movzbl	(%rcx), %eax
	salq	$4, %rax
	addq	fp(%rip), %rax
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	addq	$1, %rcx
	movq	%rcx, pc(%rip)
	jmp	.L3363
.L1686:
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebp
	movzbl	%bpl, %ebx
	addq	$1, %rax
	movq	%rax, pc(%rip)
	call	make_ref
	movq	%rax, %rcx
	movq	sp(%rip), %rax
	movq	8(%rax), %rax
	movq	%rax, 32(%rcx)
	cmpl	$68, %ebx
	je	.L1834
	cmpl	$64, %ebx
	je	.L1834
	cmpb	$66, %bpl
	je	.L1834
	movl	lv_owner_type(%rip), %eax
	movw	%ax, 40(%rcx)
	movw	$1, 42(%rcx)
	cmpl	$4, lv_owner_type(%rip)
	jne	.L1837
	movq	lv_owner(%rip), %rax
	movq	%rax, 48(%rcx)
	movq	lv_owner(%rip), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1839
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1839:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	lv_owner(%rip), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1841
.L1837:
	movq	lv_owner(%rip), %rax
	movq	%rax, 48(%rcx)
	movq	lv_owner(%rip), %rax
	addw	$1, (%rax)
	cmpl	$32, lv_owner_type(%rip)
	jne	.L1841
	movq	lv_owner(%rip), %rax
	orl	$-2147483648, 20(%rax)
	jmp	.L1841
.L1834:
	movw	$2, 40(%rcx)
.L1841:
	movq	sp(%rip), %rax
	movw	$16384, (%rax)
	movq	sp(%rip), %rax
	movq	%rcx, 8(%rax)
	jmp	.L3363
.L1687:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	addq	$1, %rax
	movq	%rax, pc(%rip)
	leal	-1(%rdx), %ebx
	cmpl	$-1, %ebx
	je	.L3363
	movl	$-1, %ebp
.L1843:
	movq	global_ref_list(%rip), %rdi
	call	kill_ref
	subl	$1, %ebx
	cmpl	%ebp, %ebx
	je	.L3363
	jmp	.L1843
.L1690:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	addq	fp(%rip), %rax
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	cmpw	$16384, (%rax)
	jne	.L1844
	movq	8(%rax), %rax
	movq	32(%rax), %rax
	movq	%rax, 8(%rsp)
	testq	%rax, %rax
	jne	.L1846
	movl	$.LC84, %edi
	call	error
.L1846:
	movq	8(%rsp), %rdx
	cmpw	$1024, (%rdx)
	jne	.L1848
	movq	global_lvalue_byte+8(%rip), %rax
	movzbl	(%rax), %ebx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1850
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1850:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movzbl	%bl, %edx
	movq	sp(%rip), %rax
	movq	%rdx, 8(%rax)
	jmp	.L3363
.L1844:
	movl	$.LC85, %edi
	movl	$0, %eax
	call	error
.L1848:
	movq	8(%rsp), %rcx
	cmpw	$16, (%rcx)
	jne	.L1852
	movq	8(%rcx), %rax
	testb	$16, 2(%rax)
	je	.L1852
	movq	%rcx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L1855
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L1857
	testb	$16, 2(%rax)
	je	.L1855
.L1857:
	movq	const0u(%rip), %rax
	movq	8(%rsp), %rdi
	movq	%rax, (%rdi)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdi)
	jmp	.L1852
.L1855:
	movq	const0u(%rip), %rax
	movq	8(%rsp), %rdx
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L1859
	cmpw	$8192, %ax
	je	.L1859
	andb	$223, %ah
	movw	%ax, (%rdx)
.L1859:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L1862
	testb	$1, const0u+2(%rip)
	je	.L1852
	movq	8(%rsp), %rcx
	movq	8(%rcx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L1865
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L1865:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rsp), %rdi
	movq	8(%rdi), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L1852
.L1862:
	testl	$17272, %eax
	je	.L1852
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L1852:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1868
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1868:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	movq	8(%rsp), %rax
	cmpw	$16, (%rax)
	jne	.L1870
	movq	8(%rax), %rax
	testq	%rax, %rax
	je	.L1872
	testb	$16, 2(%rax)
	je	.L1870
.L1872:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3363
.L1870:
	movq	8(%rsp), %rdi
	movq	(%rdi), %rax
	movq	%rax, (%rdx)
	movq	8(%rdi), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L1874
	cmpw	$8192, %ax
	je	.L1874
	andb	$223, %ah
	movw	%ax, (%rdx)
.L1874:
	movq	8(%rsp), %rcx
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1877
	testb	$1, 2(%rcx)
	je	.L3363
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L1880
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L1880:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3363
.L1877:
	testl	$17272, %eax
	je	.L3363
	movq	8(%rsp), %rdi
	movq	8(%rdi), %rax
	addw	$1, (%rax)
	jmp	.L3363
.L1691:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbx
	addq	fp(%rip), %rbx
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	cmpw	$16384, (%rbx)
	jne	.L1883
	movq	8(%rbx), %rax
	cmpq	$0, 32(%rax)
	je	.L1885
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1887
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1887:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, 16(%rdx)
	movq	8(%rbx), %rax
	movq	32(%rax), %rdx
	movq	sp(%rip), %rax
	movq	%rdx, 8(%rax)
	jmp	.L3363
.L1885:
	movl	$.LC84, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1883:
	movl	$.LC85, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1633:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 104(%rsp)
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	movb	%al, 105(%rsp)
	addq	$2, %rdx
	movq	%rdx, pc(%rip)
	movzwl	104(%rsp), %ebx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1889
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1889:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movswq	%bx,%rdx
	movq	sp(%rip), %rax
	movq	%rdx, 8(%rax)
	jmp	.L3363
.L1634:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	movb	%dl, 120(%rsp)
	leaq	1(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	1(%rax), %edx
	movb	%dl, 121(%rsp)
	leaq	2(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	2(%rax), %edx
	movb	%dl, 122(%rsp)
	leaq	3(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	3(%rax), %edx
	movb	%dl, 123(%rsp)
	leaq	4(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	4(%rax), %edx
	movb	%dl, 124(%rsp)
	leaq	5(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	5(%rax), %edx
	movb	%dl, 125(%rsp)
	leaq	6(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	6(%rax), %edx
	movb	%dl, 126(%rsp)
	leaq	7(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	7(%rax), %edx
	movb	%dl, 127(%rsp)
	addq	$8, %rax
	movq	%rax, pc(%rip)
	movq	120(%rsp), %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1891
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1891:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L1635:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	movb	%dl, 116(%rsp)
	leaq	1(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	1(%rax), %edx
	movb	%dl, 117(%rsp)
	leaq	2(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	2(%rax), %edx
	movb	%dl, 118(%rsp)
	leaq	3(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	3(%rax), %edx
	movb	%dl, 119(%rsp)
	addq	$4, %rax
	movq	%rax, pc(%rip)
	movl	116(%rsp), %ebx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1893
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1893:
	movq	sp(%rip), %rax
	leaq	16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movw	$128, 16(%rax)
	movq	sp(%rip), %rax
	movl	%ebx, 8(%rax)
	jmp	.L3363
.L1636:
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebx
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1895
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1895:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movzbl	%bl, %edx
	movq	sp(%rip), %rax
	movq	%rdx, 8(%rax)
	jmp	.L3363
.L1637:
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebx
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L1897
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L1897:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movzbl	%bl, %eax
	negl	%eax
	cltq
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	jmp	.L3363
.L1646:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1649:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1650:
	call	f_ne
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	je	.L1899
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1899:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1651:
	call	f_ge
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	je	.L1901
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1901:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1652:
	call	f_le
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	je	.L1903
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1903:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1653:
	call	f_eq
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	je	.L1905
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1905:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1654:
	call	f_lt
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	je	.L1907
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1907:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1644:
	movq	sp(%rip), %rcx
	cmpw	$2, (%rcx)
	jne	.L1909
	movq	8(%rcx), %rdx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	jne	.L1911
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1909:
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L1913
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L1911
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L1916
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L1916
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L1919
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L1911
.L1919:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L1911
.L1916:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L1911
.L1913:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L1921
	testb	$32, %dh
	jne	.L1921
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L1911
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L1928
	cmpw	$64, %ax
	jg	.L1932
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L1926
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L1927
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L1911
	.p2align 4,,7
	jmp	.L1925
.L1932:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L1930
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L1931
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L1911
	.p2align 4,,7
	jmp	.L1929
.L1926:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L1911
.L1930:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L1911
.L1925:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L1911
	call	dealloc_array
	jmp	.L1911
.L1929:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L1911
	call	free
	jmp	.L1911
.L1927:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L1911
.L1928:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L1911
.L1931:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L1911
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L1911
.L1921:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L1911
	.p2align 4,,9
	call	*8(%rcx)
.L1911:
	addq	$2, pc(%rip)
	.p2align 4,,4
	jmp	.L3363
.L1645:
	movq	sp(%rip), %rdi
	cmpw	$2, (%rdi)
	jne	.L1937
	movq	8(%rdi), %rdx
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	jne	.L1939
	addq	$2, pc(%rip)
	jmp	.L3363
.L1937:
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
.L1939:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1647:
	movq	sp(%rip), %rdi
	cmpw	$2, (%rdi)
	jne	.L1941
	movq	8(%rdi), %rdx
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	jne	.L1943
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1941:
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
.L1943:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1648:
	movq	sp(%rip), %rdi
	cmpw	$2, (%rdi)
	jne	.L1945
	movq	8(%rdi), %rdx
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	jne	.L1947
	addq	$2, pc(%rip)
	jmp	.L3363
.L1945:
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
.L1947:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1662:
	movq	sp(%rip), %rax
	cmpw	$2, (%rax)
	jne	.L1949
	cmpq	$0, 8(%rax)
	jne	.L1949
	addq	$2, pc(%rip)
	subq	$16, sp(%rip)
	jmp	.L3363
.L1949:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1663:
	movq	sp(%rip), %rdi
	cmpw	$2, (%rdi)
	jne	.L1952
	cmpq	$0, 8(%rdi)
	jne	.L1954
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rax, pc(%rip)
	jmp	.L3363
.L1954:
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	jmp	.L1956
.L1952:
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
.L1956:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1660:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	addq	fp(%rip), %rax
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	movzwl	(%rax), %edx
	cmpw	$2, %dx
	jne	.L1957
	addq	$1, 8(%rax)
	jmp	.L1959
.L1957:
	cmpw	$128, %dx
	jne	.L1960
	movss	.LC80(%rip), %xmm0
	addss	8(%rax), %xmm0
	movss	%xmm0, 8(%rax)
	jmp	.L1959
.L1960:
	movl	$.LC82, %edi
	movl	$0, %eax
	call	error
.L1959:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	cmpb	$33, %al
	jne	.L1962
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbp
	addq	fp(%rip), %rbp
	leaq	2(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	2(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbx
	addq	fp(%rip), %rbx
	leaq	3(%rdx), %rax
	movq	%rax, pc(%rip)
	movzwl	(%rbp), %edx
	movl	%edx, %eax
	orw	(%rbx), %ax
	cmpw	$4, %ax
	je	.L1966
	cmpw	$4, %ax
	jg	.L1969
	cmpw	$2, %ax
	jne	.L1964
	.p2align 4,,7
	jmp	.L1965
.L1969:
	cmpw	$128, %ax
	.p2align 4,,7
	je	.L1967
	cmpw	$130, %ax
	.p2align 4,,7
	jne	.L1964
	.p2align 4,,7
	jmp	.L1968
.L1965:
	movq	8(%rbx), %rax
	cmpq	%rax, 8(%rbp)
	setl	%al
	movzbl	%al, %eax
	jmp	.L1970
.L1967:
	movss	8(%rbx), %xmm0
	ucomiss	8(%rbp), %xmm0
	seta	%al
	movzbl	%al, %eax
	jmp	.L1970
.L1966:
	movq	8(%rbp), %rdi
	movq	8(%rbx), %rsi
	call	strcmp
	shrl	$31, %eax
	jmp	.L1970
.L1968:
	cmpw	$2, %dx
	jne	.L1971
	cvtsi2ssq	8(%rbp), %xmm0
	movss	8(%rbx), %xmm1
	ucomiss	%xmm0, %xmm1
	seta	%al
	movzbl	%al, %eax
	jmp	.L1970
.L1971:
	cvtsi2ssq	8(%rbx), %xmm0
	ucomiss	8(%rbp), %xmm0
	seta	%al
	movzbl	%al, %eax
	jmp	.L1970
.L1964:
	cmpw	$16, %dx
	jne	.L1973
	movq	8(%rbp), %rdi
	testb	$16, 2(%rdi)
	je	.L1973
	movl	$.LC86, %esi
	call	free_object
	movq	const0u(%rip), %rax
	movq	%rax, (%rbp)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbp)
.L1973:
	cmpw	$16, (%rbx)
	jne	.L1976
	movq	8(%rbx), %rdi
	testb	$16, 2(%rdi)
	je	.L1976
	movl	$.LC87, %esi
	call	free_object
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
.L1976:
	movzwl	(%rbp), %eax
	cmpw	$2, %ax
	jne	.L1979
	cmpw	$2, (%rbx)
	jne	.L1981
	movq	8(%rbx), %rdx
	cmpq	%rdx, 8(%rbp)
	setl	%al
	movzbl	%al, %eax
	jmp	.L1970
.L1979:
	cmpw	$4, %ax
	je	.L1984
	cmpw	$128, %ax
	je	.L1981
	cmpw	$2, %ax
	.p2align 4,,5
	jne	.L1983
.L1981:
	movl	$.LC88, %edi
	movl	$0, %eax
	call	error
.L1984:
	movl	$.LC89, %edi
	movl	$0, %eax
	call	error
.L1983:
	movl	$.LC90, %edi
	movl	$0, %eax
	call	error
	jmp	.L1985
.L1970:
	testl	%eax, %eax
	je	.L1985
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 104(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 105(%rsp)
	movzwl	104(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1985:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1962:
	cmpb	$34, %al
	jne	.L3363
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rsi
	addq	fp(%rip), %rsi
	leaq	2(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	2(%rdx), %eax
	movb	%al, 96(%rsp)
	leaq	3(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	3(%rdx), %eax
	movb	%al, 97(%rsp)
	leaq	4(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	4(%rdx), %eax
	movb	%al, 98(%rsp)
	leaq	5(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	5(%rdx), %eax
	movb	%al, 99(%rsp)
	leaq	6(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	6(%rdx), %eax
	movb	%al, 100(%rsp)
	leaq	7(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	7(%rdx), %eax
	movb	%al, 101(%rsp)
	leaq	8(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	8(%rdx), %eax
	movb	%al, 102(%rsp)
	leaq	9(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	9(%rdx), %eax
	movb	%al, 103(%rsp)
	leaq	10(%rdx), %rcx
	movq	%rcx, pc(%rip)
	movzwl	(%rsi), %eax
	cmpw	$2, %ax
	jne	.L1988
	movq	96(%rsp), %rdi
	cmpq	%rdi, 8(%rsi)
	jge	.L1990
	movzbl	10(%rdx), %eax
	movb	%al, 104(%rsp)
	movzbl	1(%rcx), %eax
	movb	%al, 105(%rsp)
	movzwl	104(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1990:
	leaq	2(%rcx), %rax
	movq	%rax, pc(%rip)
	jmp	.L3363
.L1988:
	cmpw	$128, %ax
	jne	.L1992
	cvtsi2ssq	96(%rsp), %xmm0
	ucomiss	8(%rsi), %xmm0
	jbe	.L1994
	movzbl	(%rcx), %eax
	movb	%al, 104(%rsp)
	movq	pc(%rip), %rax
	movzbl	1(%rax), %eax
	movb	%al, 105(%rsp)
	movzwl	104(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L1994:
	leaq	2(%rcx), %rax
	movq	%rax, pc(%rip)
	jmp	.L3363
.L1992:
	movl	$.LC91, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1658:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbp
	addq	fp(%rip), %rbp
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbx
	addq	fp(%rip), %rbx
	addq	$2, %rdx
	movq	%rdx, pc(%rip)
	movzwl	(%rbp), %edx
	movl	%edx, %eax
	orw	(%rbx), %ax
	cmpw	$4, %ax
	je	.L1999
	cmpw	$4, %ax
	jg	.L2002
	cmpw	$2, %ax
	jne	.L1997
	.p2align 4,,7
	jmp	.L1998
.L2002:
	cmpw	$128, %ax
	.p2align 4,,7
	je	.L2000
	cmpw	$130, %ax
	.p2align 4,,7
	jne	.L1997
	.p2align 4,,7
	jmp	.L2001
.L1998:
	movq	8(%rbx), %rax
	cmpq	%rax, 8(%rbp)
	setl	%al
	movzbl	%al, %eax
	jmp	.L2003
.L2000:
	movss	8(%rbx), %xmm0
	ucomiss	8(%rbp), %xmm0
	seta	%al
	movzbl	%al, %eax
	jmp	.L2003
.L1999:
	movq	8(%rbp), %rdi
	movq	8(%rbx), %rsi
	call	strcmp
	shrl	$31, %eax
	jmp	.L2003
.L2001:
	cmpw	$2, %dx
	jne	.L2004
	cvtsi2ssq	8(%rbp), %xmm0
	movss	8(%rbx), %xmm1
	ucomiss	%xmm0, %xmm1
	seta	%al
	movzbl	%al, %eax
	jmp	.L2003
.L2004:
	cvtsi2ssq	8(%rbx), %xmm0
	ucomiss	8(%rbp), %xmm0
	seta	%al
	movzbl	%al, %eax
	jmp	.L2003
.L1997:
	cmpw	$16, %dx
	jne	.L2006
	movq	8(%rbp), %rdi
	testb	$16, 2(%rdi)
	je	.L2006
	movl	$.LC86, %esi
	call	free_object
	movq	const0u(%rip), %rax
	movq	%rax, (%rbp)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbp)
.L2006:
	cmpw	$16, (%rbx)
	jne	.L2009
	movq	8(%rbx), %rdi
	testb	$16, 2(%rdi)
	je	.L2009
	movl	$.LC87, %esi
	call	free_object
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
.L2009:
	movzwl	(%rbp), %eax
	cmpw	$2, %ax
	jne	.L2012
	cmpw	$2, (%rbx)
	jne	.L2014
	movq	8(%rbx), %rdx
	cmpq	%rdx, 8(%rbp)
	setl	%al
	movzbl	%al, %eax
	jmp	.L2003
.L2012:
	cmpw	$4, %ax
	je	.L2017
	cmpw	$128, %ax
	je	.L2014
	cmpw	$2, %ax
	.p2align 4,,5
	jne	.L2016
.L2014:
	movl	$.LC88, %edi
	movl	$0, %eax
	call	error
.L2017:
	movl	$.LC89, %edi
	movl	$0, %eax
	call	error
.L2016:
	movl	$.LC90, %edi
	movl	$0, %eax
	call	error
	jmp	.L2018
.L2003:
	testl	%eax, %eax
	je	.L2018
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 104(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 105(%rsp)
	movzwl	104(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L2018:
	addq	$2, pc(%rip)
	jmp	.L3363
.L1659:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rsi
	addq	fp(%rip), %rsi
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	movb	%al, 96(%rsp)
	leaq	2(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	2(%rdx), %eax
	movb	%al, 97(%rsp)
	leaq	3(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	3(%rdx), %eax
	movb	%al, 98(%rsp)
	leaq	4(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	4(%rdx), %eax
	movb	%al, 99(%rsp)
	leaq	5(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	5(%rdx), %eax
	movb	%al, 100(%rsp)
	leaq	6(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	6(%rdx), %eax
	movb	%al, 101(%rsp)
	leaq	7(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	7(%rdx), %eax
	movb	%al, 102(%rsp)
	leaq	8(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	8(%rdx), %eax
	movb	%al, 103(%rsp)
	leaq	9(%rdx), %rcx
	movq	%rcx, pc(%rip)
	movzwl	(%rsi), %eax
	cmpw	$2, %ax
	jne	.L2020
	movq	96(%rsp), %rdi
	cmpq	%rdi, 8(%rsi)
	jge	.L2022
	movzbl	9(%rdx), %eax
	movb	%al, 104(%rsp)
	movzbl	1(%rcx), %eax
	movb	%al, 105(%rsp)
	movzwl	104(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L2022:
	leaq	2(%rcx), %rax
	movq	%rax, pc(%rip)
	jmp	.L3363
.L2020:
	cmpw	$128, %ax
	jne	.L2024
	cvtsi2ssq	96(%rsp), %xmm0
	ucomiss	8(%rsi), %xmm0
	jbe	.L2026
	movzbl	(%rcx), %eax
	movb	%al, 104(%rsp)
	movq	pc(%rip), %rax
	movzbl	1(%rax), %eax
	movb	%al, 105(%rsp)
	movzwl	104(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L2026:
	leaq	2(%rcx), %rax
	movq	%rax, pc(%rip)
	jmp	.L3363
.L2024:
	movl	$.LC91, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1685:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbx
	addq	fp(%rip), %rbx
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	cmpw	$16, (%rbx)
	jne	.L2029
	movq	8(%rbx), %rax
	testb	$16, 2(%rax)
	je	.L2029
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L2032
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L2034
	testb	$16, 2(%rax)
	je	.L2032
.L2034:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2029
.L2032:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2036
	cmpw	$8192, %ax
	je	.L2036
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2036:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L2039
	testb	$1, const0u+2(%rip)
	je	.L2029
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2042
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2042:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2029
.L2039:
	testl	$17272, %eax
	je	.L2029
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L2029:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2045
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2045:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	(%rbx), %rax
	movq	%rax, 16(%rdx)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rcx)
	orw	$8192, (%rbx)
	jmp	.L3363
.L1688:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbx
	addq	fp(%rip), %rbx
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	cmpw	$16, (%rbx)
	jne	.L2047
	movq	8(%rbx), %rax
	testb	$16, 2(%rax)
	je	.L2047
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L2050
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L2052
	testb	$16, 2(%rax)
	je	.L2050
.L2052:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2047
.L2050:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2054
	cmpw	$8192, %ax
	je	.L2054
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2054:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L2057
	testb	$1, const0u+2(%rip)
	je	.L2047
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2060
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2060:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2047
.L2057:
	testl	$17272, %eax
	je	.L2047
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L2047:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2063
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2063:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	cmpw	$16, (%rbx)
	jne	.L2065
	movq	8(%rbx), %rax
	testq	%rax, %rax
	je	.L2067
	testb	$16, 2(%rax)
	je	.L2065
.L2067:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3363
.L2065:
	movq	(%rbx), %rax
	movq	%rax, (%rdx)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L2069
	cmpw	$8192, %ax
	je	.L2069
	andb	$223, %ah
	movw	%ax, (%rdx)
.L2069:
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L2072
	testb	$1, 2(%rbx)
	je	.L3363
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L2075
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L2075:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3363
.L2072:
	testl	$17272, %eax
	je	.L3363
	movq	8(%rbx), %rax
	addw	$1, (%rax)
	jmp	.L3363
.L1676:
	.p2align 4,,5
	call	f_lt
	.p2align 4,,4
	jmp	.L3363
.L1723:
	movq	sp(%rip), %rcx
	movzwl	(%rcx), %eax
	movswl	%ax,%edi
	cmpw	$8, %ax
	je	.L2081
	cmpw	$8, %ax
	jg	.L2085
	cmpw	$2, %ax
	je	.L2079
	cmpw	$4, %ax
	.p2align 4,,5
	jne	.L2078
	.p2align 4,,7
	jmp	.L2080
.L2085:
	cmpw	$128, %ax
	.p2align 4,,7
	je	.L2083
	cmpw	$256, %ax
	.p2align 4,,7
	je	.L2084
	cmpw	$32, %ax
	.p2align 4,,5
	jne	.L2078
	.p2align 4,,7
	jmp	.L2082
.L2084:
	leaq	-16(%rcx), %rax
	cmpw	$256, -16(%rcx)
	.p2align 4,,5
	je	.L2086
	.p2align 4,,7
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	-16(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC92, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L2086:
	movq	8(%rax), %rdx
	movq	8(%rcx), %rax
	movl	4(%rax), %edi
	addl	4(%rdx), %edi
	call	allocate_buffer
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movq	-8(%rax), %rsi
	leaq	8(%rbx), %rbp
	mov	4(%rsi), %edx
	addq	$8, %rsi
	movq	%rbp, %rdi
	call	memcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movq	-8(%rax), %rax
	mov	4(%rax), %eax
	leaq	(%rbp,%rax), %rdi
	mov	4(%rsi), %edx
	addq	$8, %rsi
	call	memcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	subq	$16, %rax
	movq	%rax, sp(%rip)
	call	free_buffer
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	call	free_buffer
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L2079:
	leaq	-16(%rcx), %rdx
	movq	%rdx, sp(%rip)
	movzwl	-16(%rcx), %eax
	cmpw	$4, %ax
	je	.L2090
	cmpw	$128, %ax
	je	.L2091
	cmpw	$2, %ax
	jne	.L2088
	movq	24(%rdx), %rax
	addq	%rax, 8(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L2091:
	cvtsi2ssq	24(%rdx), %xmm0
	addss	8(%rdx), %xmm0
	movss	%xmm0, 8(%rdx)
	jmp	.L3363
.L2090:
	movq	24(%rdx), %rdx
	leaq	32(%rsp), %rdi
	movl	$.LC93, %esi
	movl	$0, %eax
	call	sprintf
	movq	sp(%rip), %rdx
	testb	$1, 2(%rdx)
	je	.L2092
	movq	8(%rdx), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %ebp
	movl	%ebp, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L2094
	movq	8(%rdx), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %ebp
	jmp	.L2094
.L2092:
	movq	8(%rdx), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %ebp
.L2094:
	leaq	32(%rsp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rbp,%rcx), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2096
	movl	$.LC94, %edi
	call	error
.L2096:
	movq	sp(%rip), %rax
	cmpw	$1, 2(%rax)
	jne	.L2098
	movq	8(%rax), %rdi
	cmpw	$1, -2(%rdi)
	jne	.L2098
	movl	%ebx, %esi
	call	extend_string
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L2101
	movl	$.LC95, %edi
	movl	$0, %eax
	call	fatal
.L2101:
	leaq	32(%rsp), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	jmp	.L2103
.L2098:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movq	%rbx, %rdi
	call	strcpy
	leaq	32(%rsp), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2104
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2106
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2106
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2109
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2104
.L2109:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2104
.L2106:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2104:
	movq	sp(%rip), %rax
	movw	$1, 2(%rax)
.L2103:
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L2088:
	movswl	16(%rdx),%edi
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC96, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L2083:
	leaq	-16(%rcx), %rdx
	movq	%rdx, sp(%rip)
	movzwl	-16(%rcx), %eax
	cmpw	$4, %ax
	je	.L2113
	cmpw	$128, %ax
	je	.L2114
	cmpw	$2, %ax
	jne	.L2111
	movw	$128, -16(%rcx)
	movq	sp(%rip), %rax
	cvtsi2ssq	8(%rax), %xmm0
	addss	24(%rax), %xmm0
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L2114:
	movss	8(%rdx), %xmm0
	addss	24(%rdx), %xmm0
	movss	%xmm0, 8(%rdx)
	jmp	.L3363
.L2113:
	leaq	32(%rsp), %rdi
	cvtss2sd	24(%rdx), %xmm0
	movl	$.LC97, %esi
	movl	$1, %eax
	call	sprintf
	movq	sp(%rip), %rdx
	testb	$1, 2(%rdx)
	je	.L2115
	movq	8(%rdx), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %ebp
	movl	%ebp, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L2117
	movq	8(%rdx), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %ebp
	jmp	.L2117
.L2115:
	movq	8(%rdx), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %ebp
.L2117:
	leaq	32(%rsp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rbp,%rcx), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2119
	movl	$.LC94, %edi
	call	error
.L2119:
	movq	sp(%rip), %rax
	cmpw	$1, 2(%rax)
	jne	.L2121
	movq	8(%rax), %rdi
	cmpw	$1, -2(%rdi)
	jne	.L2121
	movl	%ebx, %esi
	call	extend_string
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L2124
	movl	$.LC95, %edi
	movl	$0, %eax
	call	fatal
.L2124:
	leaq	32(%rsp), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	jmp	.L2126
.L2121:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movq	%rbx, %rdi
	call	strcpy
	leaq	32(%rsp), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2127
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2129
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2129
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2132
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2127
.L2132:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2127
.L2129:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2127:
	movq	sp(%rip), %rax
	movw	$1, 2(%rax)
.L2126:
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L2111:
	movswl	16(%rdx),%edi
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC98, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L2081:
	leaq	-16(%rcx), %rbx
	cmpw	$8, -16(%rcx)
	je	.L2134
	.p2align 4,,7
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	-16(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC98, %edi
	movl	$0, %eax
	call	error
	jmp	.L2082
.L2134:
	movq	8(%rcx), %rsi
	movq	8(%rbx), %rdi
	call	add_array
	movq	%rax, 8(%rbx)
	subq	$16, sp(%rip)
	jmp	.L3363
.L2082:
	movq	sp(%rip), %rax
	cmpw	$32, -16(%rax)
	jne	.L2136
	movq	8(%rax), %rsi
	movq	-8(%rax), %rdi
	call	add_mapping
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	subq	$16, %rax
	movq	%rax, sp(%rip)
	call	free_mapping
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	call	free_mapping
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L2136:
	movswl	(%rax),%edi
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	-16(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC98, %edi
	movl	$0, %eax
	call	error
.L2080:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rcx
	movzwl	-16(%rdx), %eax
	cmpw	$4, %ax
	je	.L2140
	cmpw	$128, %ax
	je	.L2141
	cmpw	$2, %ax
	jne	.L2138
	movq	-8(%rdx), %rdx
	leaq	32(%rsp), %rdi
	movl	$.LC93, %esi
	movl	$0, %eax
	call	sprintf
	movq	sp(%rip), %rax
	testb	$1, 2(%rax)
	je	.L2142
	movq	8(%rax), %rax
	movzwl	-4(%rax), %edx
	movzwl	%dx, %eax
	movl	%eax, svalue_strlen_size(%rip)
	movl	%eax, %esi
	cmpw	$-1, %dx
	jne	.L2146
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %esi
	jmp	.L2146
.L2142:
	movq	8(%rax), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %esi
.L2146:
	leaq	32(%rsp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %ebp
	leal	(%rsi,%rbp), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2147
	movl	$.LC94, %edi
	call	error
.L2147:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	leaq	32(%rsp), %rsi
	movq	%rax, %rdi
	call	strcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	movq	8(%rdx), %rdi
	testb	$1, 2(%rdx)
	je	.L2149
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %ecx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2151
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2151
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rdx)
	je	.L2154
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2149
.L2154:
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2149
.L2151:
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2149:
	movq	sp(%rip), %rax
	movw	$4, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	movw	$1, 2(%rax)
	jmp	.L3363
.L2141:
	leaq	32(%rsp), %rdi
	cvtss2sd	8(%rcx), %xmm0
	movl	$.LC97, %esi
	movl	$1, %eax
	call	sprintf
	movq	sp(%rip), %rax
	testb	$1, 2(%rax)
	je	.L2156
	movq	8(%rax), %rax
	movzwl	-4(%rax), %edx
	movzwl	%dx, %eax
	movl	%eax, svalue_strlen_size(%rip)
	movl	%eax, %esi
	cmpw	$-1, %dx
	jne	.L2160
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %esi
	jmp	.L2160
.L2156:
	movq	8(%rax), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %esi
.L2160:
	leaq	32(%rsp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %ebp
	leal	(%rsi,%rbp), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2161
	movl	$.LC94, %edi
	call	error
.L2161:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	leaq	32(%rsp), %rsi
	movq	%rax, %rdi
	call	strcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	movq	8(%rdx), %rdi
	testb	$1, 2(%rdx)
	je	.L2163
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %ecx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2165
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2165
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rdx)
	je	.L2168
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2163
.L2168:
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2163
.L2165:
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2163:
	movq	sp(%rip), %rax
	movw	$4, (%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	movw	$1, 2(%rax)
	jmp	.L3363
.L2140:
	testb	$1, 2(%rcx)
	je	.L2170
	movq	8(%rcx), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %ebp
	movl	%ebp, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L2172
	movq	sp(%rip), %rax
	movq	-8(%rax), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %ebp
	jmp	.L2172
.L2170:
	movq	8(%rcx), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %ebp
.L2172:
	movq	sp(%rip), %rax
	testb	$1, 2(%rax)
	je	.L2174
	movq	8(%rax), %rax
	movzwl	-4(%rax), %edx
	movzwl	%dx, %eax
	movl	%eax, svalue_strlen_size(%rip)
	cmpw	$-1, %dx
	jne	.L2178
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %eax
	jmp	.L2178
.L2174:
	movq	8(%rax), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %eax
.L2178:
	leal	(%rax,%rbp), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2179
	movl	$.LC94, %edi
	movl	$0, %eax
	call	error
.L2179:
	movq	sp(%rip), %rax
	subq	$16, %rax
	cmpw	$1, 2(%rax)
	jne	.L2181
	movq	8(%rax), %rdi
	cmpw	$1, -2(%rdi)
	jne	.L2181
	movl	%ebx, %esi
	call	extend_string
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L2184
	movl	$.LC95, %edi
	movl	$0, %eax
	call	fatal
.L2184:
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2186
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2188
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2188
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2191
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2186
.L2191:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2186
.L2188:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L2186
.L2181:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movq	-8(%rax), %rsi
	movq	%rbx, %rdi
	call	strcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movslq	%ebp,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2193
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2195
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2195
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2198
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2193
.L2198:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2193
.L2195:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2193:
	movq	sp(%rip), %rcx
	subq	$16, %rcx
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2200
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2202
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2202
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2205
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2200
.L2205:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2200
.L2202:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2200:
	movq	sp(%rip), %rax
	movw	$1, -14(%rax)
.L2186:
	movq	sp(%rip), %rax
	movq	%rbx, -8(%rax)
	subq	$16, sp(%rip)
	jmp	.L3363
.L2138:
	movswl	(%rdx),%edi
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	-16(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC98, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L2078:
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	-16(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC96, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1710:
	movq	sp(%rip), %rax
	movq	8(%rax), %r12
	leaq	-16(%rax), %r8
	movq	%r8, sp(%rip)
	movzwl	(%r12), %eax
	cmpw	$32, %ax
	je	.L2211
	cmpw	$32, %ax
	jg	.L2215
	cmpw	$4, %ax
	je	.L2209
	cmpw	$8, %ax
	.p2align 4,,5
	je	.L2210
	cmpw	$2, %ax
	.p2align 4,,5
	jne	.L2207
	.p2align 4,,7
	jmp	.L2208
.L2215:
	cmpw	$256, %ax
	.p2align 4,,7
	je	.L2213
	cmpw	$1024, %ax
	.p2align 4,,7
	je	.L2214
	cmpw	$128, %ax
	.p2align 4,,5
	jne	.L2207
	.p2align 4,,7
	jmp	.L2212
.L2209:
	movzwl	(%r8), %eax
	cmpw	$4, %ax
	.p2align 4,,5
	jne	.L2216
	testb	$1, 2(%r12)
	.p2align 4,,5
	je	.L2218
	movq	8(%r12), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %r13d
	movl	%r13d, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L2220
	movq	8(%r12), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %r13d
	jmp	.L2220
.L2218:
	movq	8(%r12), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %r13d
.L2220:
	movq	sp(%rip), %rax
	testb	$1, 2(%rax)
	je	.L2222
	movq	8(%rax), %rax
	movzwl	-4(%rax), %edx
	movzwl	%dx, %eax
	movl	%eax, svalue_strlen_size(%rip)
	cmpw	$-1, %dx
	jne	.L2226
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %eax
	jmp	.L2226
.L2222:
	movq	8(%rax), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %eax
.L2226:
	leal	(%rax,%r13), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2227
	movl	$.LC94, %edi
	movl	$0, %eax
	call	error
.L2227:
	cmpw	$1, 2(%r12)
	jne	.L2229
	movq	8(%r12), %rdi
	cmpw	$1, -2(%rdi)
	jne	.L2229
	movl	%ebx, %esi
	call	extend_string
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L2232
	movl	$.LC95, %edi
	movl	$0, %eax
	call	fatal
.L2232:
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movslq	%r13d,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2234
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2236
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2236
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2239
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2234
.L2239:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2234
.L2236:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L2234
.L2229:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	movq	8(%r12), %rsi
	movq	%rax, %rdi
	call	strcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movslq	%r13d,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2241
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2243
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2243
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2246
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2241
.L2246:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2241
.L2243:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2241:
	movq	8(%r12), %rdi
	testb	$1, 2(%r12)
	je	.L2248
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2250
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2250
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%r12)
	je	.L2253
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2248
.L2253:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L2248
.L2250:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2248:
	movw	$1, 2(%r12)
.L2234:
	movq	%rbx, 8(%r12)
	jmp	.L2255
.L2216:
	cmpw	$2, %ax
	jne	.L2256
	movq	8(%r8), %rdx
	leaq	32(%rsp), %rdi
	movl	$.LC93, %esi
	movl	$0, %eax
	call	sprintf
	testb	$1, 2(%r12)
	je	.L2258
	movq	8(%r12), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %r13d
	movl	%r13d, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L2260
	movq	8(%r12), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %r13d
	jmp	.L2260
.L2258:
	movq	8(%r12), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %r13d
.L2260:
	leaq	32(%rsp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%r13,%rcx), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2262
	movl	$.LC94, %edi
	call	error
.L2262:
	cmpw	$1, 2(%r12)
	jne	.L2264
	movq	8(%r12), %rdi
	cmpw	$1, -2(%rdi)
	jne	.L2264
	movl	%ebx, %esi
	call	extend_string
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L2267
	movl	$.LC95, %edi
	movl	$0, %eax
	call	fatal
.L2267:
	leaq	32(%rsp), %rsi
	movslq	%r13d,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	jmp	.L2269
.L2264:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	movq	8(%r12), %rsi
	movq	%rax, %rdi
	call	strcpy
	leaq	32(%rsp), %rsi
	movslq	%r13d,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	8(%r12), %rdi
	testb	$1, 2(%r12)
	je	.L2270
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2272
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2272
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%r12)
	je	.L2275
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2270
.L2275:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L2270
.L2272:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2270:
	movw	$1, 2(%r12)
.L2269:
	movq	%rbx, 8(%r12)
	jmp	.L2255
.L2256:
	cmpw	$128, %ax
	jne	.L2277
	leaq	32(%rsp), %rdi
	cvtss2sd	8(%r8), %xmm0
	movl	$.LC97, %esi
	movl	$1, %eax
	call	sprintf
	testb	$1, 2(%r12)
	je	.L2279
	movq	8(%r12), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %r13d
	movl	%r13d, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L2281
	movq	8(%r12), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %r13d
	jmp	.L2281
.L2279:
	movq	8(%r12), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %r13d
.L2281:
	leaq	32(%rsp), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%r13,%rcx), %ebx
	cmpl	config_int+56(%rip), %ebx
	jle	.L2283
	movl	$.LC94, %edi
	call	error
.L2283:
	cmpw	$1, 2(%r12)
	jne	.L2285
	movq	8(%r12), %rdi
	cmpw	$1, -2(%rdi)
	jne	.L2285
	movl	%ebx, %esi
	call	extend_string
	movq	%rax, %rbx
	testq	%rax, %rax
	jne	.L2288
	movl	$.LC95, %edi
	movl	$0, %eax
	call	fatal
.L2288:
	leaq	32(%rsp), %rsi
	movslq	%r13d,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	jmp	.L2290
.L2285:
	movl	%ebx, %edi
	call	int_new_string
	movq	%rax, %rbx
	movq	8(%r12), %rsi
	movq	%rax, %rdi
	call	strcpy
	leaq	32(%rsp), %rsi
	movslq	%r13d,%rdi
	leaq	(%rbx,%rdi), %rdi
	call	strcpy
	movq	8(%r12), %rdi
	testb	$1, 2(%r12)
	je	.L2291
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2293
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2293
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%r12)
	je	.L2296
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2291
.L2296:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L2291
.L2293:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2291:
	movw	$1, 2(%r12)
.L2290:
	movq	%rbx, 8(%r12)
	jmp	.L2255
.L2277:
	movl	%ebx, %ecx
	movl	$2, %edx
	movl	$134, %esi
	movq	%r8, %rdi
	call	bad_argument
	jmp	.L2255
.L2208:
	movzwl	(%r8), %eax
	cmpw	$2, %ax
	jne	.L2298
	movq	8(%r8), %rax
	addq	%rax, 8(%r12)
	movw	$0, 2(%r12)
	jmp	.L2255
.L2298:
	cmpw	$128, %ax
	jne	.L2300
	cvtsi2ssq	8(%r12), %xmm0
	addss	8(%r8), %xmm0
	cvttss2siq	%xmm0, %rax
	movq	%rax, 8(%r12)
	movw	$0, 2(%r12)
	jmp	.L2255
.L2300:
	movl	$.LC99, %edi
	movl	$0, %eax
	call	error
	jmp	.L2255
.L2212:
	movzwl	(%r8), %eax
	cmpw	$2, %ax
	jne	.L2302
	cvtsi2ssq	8(%r8), %xmm0
	addss	8(%r12), %xmm0
	movss	%xmm0, 8(%r12)
	jmp	.L2255
.L2302:
	cmpw	$128, %ax
	jne	.L2304
	movss	8(%r12), %xmm0
	addss	8(%r8), %xmm0
	movss	%xmm0, 8(%r12)
	jmp	.L2255
.L2304:
	movl	$.LC99, %edi
	movl	$0, %eax
	call	error
	jmp	.L2255
.L2213:
	cmpw	$256, (%r8)
	je	.L2306
	movl	%ebx, %ecx
	movl	$2, %edx
	movl	$256, %esi
	movq	%r8, %rdi
	call	bad_argument
	jmp	.L2255
.L2306:
	movq	8(%r8), %rdx
	movq	8(%r12), %rax
	movl	4(%rax), %edi
	addl	4(%rdx), %edi
	call	allocate_buffer
	movq	%rax, %rbx
	movq	8(%r12), %rsi
	leaq	8(%rax), %r13
	mov	4(%rsi), %edx
	addq	$8, %rsi
	movq	%r13, %rdi
	call	memcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rsi
	movq	8(%r12), %rax
	mov	4(%rax), %eax
	leaq	(%r13,%rax), %rdi
	mov	4(%rsi), %edx
	addq	$8, %rsi
	call	memcpy
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	call	free_buffer
	movq	8(%r12), %rdi
	call	free_buffer
	movq	%rbx, 8(%r12)
	jmp	.L2255
.L2210:
	cmpw	$8, (%r8)
	.p2align 4,,2
	je	.L2308
	movl	%ebx, %ecx
	movl	$2, %edx
	movl	$8, %esi
	movq	%r8, %rdi
	call	bad_argument
	jmp	.L2255
.L2308:
	movq	8(%r8), %rsi
	movq	8(%r12), %rdi
	call	add_array
	movq	%rax, 8(%r12)
	.p2align 4,,2
	jmp	.L2255
.L2211:
	cmpw	$32, (%r8)
	je	.L2310
	movl	%ebx, %ecx
	movl	$2, %edx
	movl	$32, %esi
	movq	%r8, %rdi
	call	bad_argument
	jmp	.L2255
.L2310:
	movq	8(%r8), %rsi
	movq	8(%r12), %rdi
	call	absorb_mapping
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	call	free_mapping
	jmp	.L2255
.L2214:
	cmpw	$2, (%r8)
	je	.L2312
	movl	$.LC100, %edi
	movl	$0, %eax
	call	error
.L2312:
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	movq	global_lvalue_byte+8(%rip), %rax
	movzbl	(%rax), %ebx
	addl	%edx, %ebx
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L2314
	testb	%bl, %bl
	jne	.L2314
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L2314:
	movq	global_lvalue_byte+8(%rip), %rax
	movb	%bl, (%rax)
	jmp	.L2255
.L2207:
	movl	%ebx, %esi
	movl	$1, %edi
	call	bad_arg
.L2255:
	cmpb	$85, %bpl
	jne	.L2316
	movq	sp(%rip), %rdx
	cmpw	$16, (%r12)
	jne	.L2318
	movq	8(%r12), %rax
	testq	%rax, %rax
	je	.L2320
	testb	$16, 2(%rax)
	je	.L2318
.L2320:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3363
.L2318:
	movq	(%r12), %rax
	movq	%rax, (%rdx)
	movq	8(%r12), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L2322
	cmpw	$8192, %ax
	je	.L2322
	andb	$223, %ah
	movw	%ax, (%rdx)
.L2322:
	movzwl	(%r12), %eax
	cmpw	$4, %ax
	jne	.L2325
	testb	$1, 2(%r12)
	je	.L3363
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L2328
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L2328:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3363
.L2325:
	testl	$17272, %eax
	je	.L3363
	movq	8(%r12), %rax
	addw	$1, (%rax)
	jmp	.L3363
.L2316:
	subq	$16, sp(%rip)
	jmp	.L3363
.L1728:
	call	f_and
	.p2align 4,,3
	jmp	.L3363
.L1712:
	.p2align 4,,8
	call	f_and_eq
	.p2align 4,,8
	jmp	.L3363
.L1736:
	.p2align 4,,8
	call	f_function_constructor
	.p2align 4,,8
	jmp	.L3363
.L1655:
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebp
	addq	$1, %rax
	movq	%rax, pc(%rip)
	testb	$8, %bpl
	je	.L2331
	movq	sp(%rip), %rdi
	testb	$32, (%rdi)
	jne	.L2333
	movl	$30, %ecx
	movl	$2, %edx
	movl	$32, %esi
	call	bad_argument
.L2333:
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	call	mapping_indices
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2335
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2335:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$8, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2337
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2337:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rdx
	movq	-8(%rdx), %rax
	addq	$8, %rax
	movq	%rax, 8(%rdx)
	movq	sp(%rip), %rdx
	movq	-8(%rdx), %rax
	movzwl	2(%rax), %eax
	movw	%ax, 2(%rdx)
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2339
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2339:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, 16(%rdx)
	testb	$1, %bpl
	je	.L2341
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	addl	variable_index_offset(%rip), %edx
	movslq	%edx,%rdx
	salq	$4, %rdx
	addq	current_object(%rip), %rdx
	leaq	120(%rdx), %rdx
	movq	sp(%rip), %rcx
	movq	%rdx, 8(%rcx)
	addq	$1, %rax
	movq	%rax, pc(%rip)
	jmp	.L2343
.L2341:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	salq	$4, %rdx
	addq	fp(%rip), %rdx
	movq	sp(%rip), %rcx
	movq	%rdx, 8(%rcx)
	addq	$1, %rax
	movq	%rax, pc(%rip)
	jmp	.L2343
.L2331:
	movq	sp(%rip), %rdi
	movzwl	(%rdi), %eax
	cmpw	$4, %ax
	jne	.L2344
	leaq	16(%rdi), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2346
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2346:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rdx
	movq	-8(%rdx), %rax
	movq	%rax, 8(%rdx)
	movq	sp(%rip), %rsi
	leaq	-16(%rsi), %rax
	testb	$1, -14(%rsi)
	je	.L2348
	movq	-8(%rsi), %rax
	movzwl	-4(%rax), %edx
	movzwl	%dx, %eax
	movl	%eax, svalue_strlen_size(%rip)
	cmpw	$-1, %dx
	jne	.L2352
	movq	sp(%rip), %rsi
	movq	-8(%rsi), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-2(%rcx), %eax
	jmp	.L2352
.L2348:
	movq	8(%rax), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %eax
.L2352:
	movw	%ax, 2(%rsi)
	jmp	.L2343
.L2344:
	testb	$8, %al
	jne	.L2353
	movl	$30, %ecx
	movl	$2, %edx
	movl	$8, %esi
	call	bad_argument
.L2353:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2355
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2355:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rdx
	movq	-8(%rdx), %rax
	addq	$8, %rax
	movq	%rax, 8(%rdx)
	movq	sp(%rip), %rdx
	movq	-8(%rdx), %rax
	movzwl	2(%rax), %eax
	movw	%ax, 2(%rdx)
.L2343:
	testb	$2, %bpl
	je	.L2357
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2359
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2359:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, 16(%rdx)
	movq	pc(%rip), %rcx
	movzbl	(%rcx), %eax
	addl	variable_index_offset(%rip), %eax
	cltq
	salq	$4, %rax
	addq	current_object(%rip), %rax
	leaq	120(%rax), %rax
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	addq	$1, %rcx
	movq	%rcx, pc(%rip)
	jmp	.L3363
.L2357:
	testb	$4, %bpl
	je	.L2361
	call	make_ref
	movq	%rax, %rbx
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	salq	$4, %rdx
	movq	%rdx, %rbp
	addq	fp(%rip), %rbp
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	$0, 32(%rbx)
	movw	$2, 40(%rbx)
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2363
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2363:
	movq	sp(%rip), %rax
	leaq	16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movw	$16384, 16(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movw	$16384, (%rbp)
	movq	%rbx, 8(%rbp)
	addw	$1, (%rbx)
	jmp	.L3363
.L2361:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2365
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2365:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, 16(%rdx)
	movq	pc(%rip), %rcx
	movzbl	(%rcx), %eax
	salq	$4, %rax
	addq	fp(%rip), %rax
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	addq	$1, %rcx
	movq	%rcx, pc(%rip)
	jmp	.L3363
.L1656:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rcx
	cmpw	$1, -16(%rdx)
	jne	.L2367
	movzwl	-30(%rdx), %eax
	subl	$1, %eax
	movw	%ax, -30(%rdx)
	cmpw	$-1, %ax
	je	.L2369
	movq	sp(%rip), %rdx
	subq	$32, %rdx
	movq	8(%rdx), %rbp
	leaq	16(%rbp), %rax
	movq	%rax, 8(%rdx)
	movq	sp(%rip), %rax
	movq	-56(%rax), %rdi
	movq	%rbp, %rsi
	call	find_in_mapping
	movq	%rax, %r12
	movq	sp(%rip), %rax
	movq	-8(%rax), %rbx
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, (%rbp)
	jne	.L2371
	movq	8(%rbp), %rax
	testq	%rax, %rax
	je	.L2373
	testb	$16, 2(%rax)
	.p2align 4,,3
	je	.L2371
.L2373:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2375
.L2371:
	movq	(%rbp), %rax
	movq	%rax, (%rbx)
	movq	8(%rbp), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2376
	cmpw	$8192, %ax
	je	.L2376
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2376:
	movzwl	(%rbp), %eax
	cmpw	$4, %ax
	jne	.L2379
	testb	$1, 2(%rbp)
	je	.L2375
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2382
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2382:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2375
.L2379:
	testl	$17272, %eax
	je	.L2375
	movq	8(%rbp), %rax
	addw	$1, (%rax)
.L2375:
	movq	sp(%rip), %rax
	cmpw	$16384, (%rax)
	jne	.L2385
	cmpq	$const0u, %r12
	jne	.L2387
	movq	8(%rax), %rax
	movq	$0, 32(%rax)
	jmp	.L2389
.L2387:
	movq	8(%rax), %rax
	movq	%r12, 32(%rax)
	jmp	.L2389
.L2385:
	movq	8(%rax), %rbx
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, (%r12)
	jne	.L2390
	movq	8(%r12), %rax
	testq	%rax, %rax
	je	.L2392
	testb	$16, 2(%rax)
	.p2align 4,,3
	je	.L2390
.L2392:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2389
.L2390:
	movq	(%r12), %rax
	movq	%rax, (%rbx)
	movq	8(%r12), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2394
	cmpw	$8192, %ax
	je	.L2394
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2394:
	movzwl	(%r12), %eax
	cmpw	$4, %ax
	jne	.L2397
	testb	$1, 2(%r12)
	je	.L2389
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2400
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2400:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2389
.L2397:
	testl	$17272, %eax
	je	.L2389
	movq	8(%r12), %rax
	addw	$1, (%rax)
.L2389:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L2367:
	movzwl	2(%rcx), %eax
	subl	$1, %eax
	movw	%ax, 2(%rcx)
	cmpw	$-1, %ax
	je	.L2369
	movq	sp(%rip), %rcx
	cmpw	$4, -32(%rcx)
	jne	.L2404
	cmpw	$16384, (%rcx)
	jne	.L2406
	movq	8(%rcx), %rax
	movq	$global_lvalue_byte, 32(%rax)
	movq	sp(%rip), %rdx
	subq	$16, %rdx
	movq	8(%rdx), %rax
	movq	%rax, global_lvalue_byte+8(%rip)
	addq	$1, %rax
	movq	%rax, 8(%rdx)
	jmp	.L2408
.L2406:
	movq	8(%rcx), %rcx
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L2409
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2411
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2413
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2413
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2416
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2411
.L2416:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2411
.L2413:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L2411
.L2409:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L2418
	testb	$32, %dh
	jne	.L2418
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L2411
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L2425
	cmpw	$64, %ax
	jg	.L2429
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L2423
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L2424
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L2411
	.p2align 4,,7
	jmp	.L2422
.L2429:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L2427
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L2428
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L2411
	.p2align 4,,7
	jmp	.L2426
.L2423:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L2411
.L2427:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L2411
.L2422:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L2411
	call	dealloc_array
	jmp	.L2411
.L2426:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L2411
	call	free
	jmp	.L2411
.L2424:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L2411
.L2425:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L2411
.L2428:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L2411
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L2411
.L2418:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L2411
	.p2align 4,,9
	call	*8(%rcx)
.L2411:
	movq	sp(%rip), %rax
	movq	8(%rax), %rax
	movw	$2, (%rax)
	movq	sp(%rip), %rax
	movq	8(%rax), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rcx
	movq	-8(%rcx), %rax
	movq	8(%rcx), %rsi
	movzbl	(%rax), %edx
	movq	%rdx, 8(%rsi)
	addq	$1, %rax
	movq	%rax, -8(%rcx)
	jmp	.L2408
.L2404:
	cmpw	$16384, (%rcx)
	jne	.L2434
	movq	-8(%rcx), %rax
	movq	8(%rcx), %rdx
	movq	%rax, 32(%rdx)
	addq	$16, %rax
	movq	%rax, -8(%rcx)
	jmp	.L2408
.L2434:
	movq	-8(%rcx), %rbx
	leaq	16(%rbx), %rax
	movq	%rax, -8(%rcx)
	movq	sp(%rip), %rax
	movq	8(%rax), %rbp
	movq	%rbp, %rdi
	call	int_free_svalue
	cmpw	$16, (%rbx)
	jne	.L2436
	movq	8(%rbx), %rax
	testq	%rax, %rax
	je	.L2438
	testb	$16, 2(%rax)
	.p2align 4,,3
	je	.L2436
.L2438:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbp)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbp)
	jmp	.L2408
.L2436:
	movq	(%rbx), %rax
	movq	%rax, (%rbp)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rbp)
	movzwl	(%rbp), %eax
	testb	$32, %ah
	je	.L2440
	cmpw	$8192, %ax
	je	.L2440
	andb	$223, %ah
	movw	%ax, (%rbp)
.L2440:
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L2443
	testb	$1, 2(%rbx)
	je	.L2408
	movq	8(%rbp), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2446
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2446:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbp), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2408
.L2443:
	testl	$17272, %eax
	je	.L2408
	movq	8(%rbx), %rax
	addw	$1, (%rax)
.L2408:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	subq	%rax, pc(%rip)
	jmp	.L3363
.L2369:
	addq	$2, pc(%rip)
.L1657:
	movq	sp(%rip), %rax
	cmpw	$16384, (%rax)
	jne	.L2449
	movq	8(%rax), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L2449
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	cmpq	$0, 32(%rdi)
	jne	.L2449
	call	free
.L2449:
	movq	sp(%rip), %rdx
	cmpw	$1, -16(%rdx)
	jne	.L2453
	leaq	-48(%rdx), %rax
	movq	%rax, sp(%rip)
	movq	-40(%rdx), %rdi
	leaq	-64(%rdx), %rax
	movq	%rax, sp(%rip)
	call	free_array
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	subq	$16, %rax
	movq	%rax, sp(%rip)
	call	free_mapping
	jmp	.L3363
.L2453:
	leaq	-32(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$4, -32(%rdx)
	jne	.L2455
	leaq	-48(%rdx), %rax
	movq	%rax, sp(%rip)
	movq	-24(%rdx), %rdi
	testb	$1, -30(%rdx)
	je	.L3363
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %ecx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2458
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2458
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, -30(%rdx)
	je	.L2461
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L3363
.L2461:
	subl	$1, num_distinct_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L3363
.L2458:
	subl	$1, allocd_strings(%rip)
	movl	%ecx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L3363
.L2455:
	movq	8(%rax), %rdi
	subq	$16, %rax
	movq	%rax, sp(%rip)
	call	free_array
	jmp	.L3363
.L1742:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	movq	%rdx, 120(%rsp)
	addq	$1, %rax
	movq	%rax, pc(%rip)
	salq	$4, %rdx
	movq	sp(%rip), %rax
	movq	%rax, %rbp
	subq	%rdx, %rbp
	cmpw	$8, (%rbp)
	je	.L2463
	movl	$.LC101, %edi
	movl	$0, %eax
	call	error
.L2463:
	movq	8(%rbp), %r14
	movzwl	2(%r14), %ebx
	movzwl	%bx, %r12d
	movq	%r12, %r13
	salq	$4, %r13
	movq	%r13, (%rsp)
	movq	%r13, %rax
	addq	sp(%rip), %rax
	subq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2465
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2465:
	movl	num_varargs(%rip), %eax
	subl	$1, %eax
	addl	%r12d, %eax
	movl	%eax, num_varargs(%rip)
	testw	%bx, %bx
	jne	.L2467
	movq	sp(%rip), %rax
	cmpq	%rbp, %rax
	jbe	.L2469
	movq	%rbp, %rdx
.L2471:
	addq	$16, %rdx
	movq	(%rdx), %rax
	movq	%rax, -16(%rdx)
	movq	8(%rdx), %rax
	movq	%rax, -8(%rdx)
	movq	sp(%rip), %rax
	cmpq	%rdx, %rax
	ja	.L2471
.L2469:
	subq	$16, %rax
	movq	%rax, sp(%rip)
	jmp	.L2472
.L2467:
	cmpw	$1, %bx
	jne	.L2473
	leaq	8(%r14), %rdx
	cmpw	$16, 8(%r14)
	jne	.L2475
	movq	8(%rdx), %rax
	testq	%rax, %rax
	je	.L2477
	testb	$16, 2(%rax)
	je	.L2475
.L2477:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbp)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbp)
	jmp	.L2472
.L2475:
	movq	(%rdx), %rax
	movq	%rax, (%rbp)
	movq	8(%rdx), %rax
	movq	%rax, 8(%rbp)
	movzwl	(%rbp), %eax
	testb	$32, %ah
	je	.L2479
	cmpw	$8192, %ax
	je	.L2479
	andb	$223, %ah
	movw	%ax, (%rbp)
.L2479:
	movzwl	(%rdx), %eax
	cmpw	$4, %ax
	jne	.L2482
	testb	$1, 2(%rdx)
	je	.L2472
	movq	8(%rbp), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2485
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2485:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbp), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2472
.L2482:
	testl	$17272, %eax
	je	.L2472
	movq	8(%rdx), %rax
	addw	$1, (%rax)
	jmp	.L2472
.L2473:
	movq	sp(%rip), %rbx
	movq	(%rsp), %rax
	leaq	-16(%rax,%rbx), %r15
	cmpq	end_of_stack(%rip), %r15
	jb	.L2488
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2488:
	movq	(%rsp), %rax
	addq	sp(%rip), %rax
	subq	$16, %rax
	movq	%rax, sp(%rip)
	cmpq	%rbx, %rbp
	jae	.L2490
	movq	%r15, %rdx
.L2492:
	movq	(%rbx), %rax
	movq	%rax, (%rdx)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rdx)
	subq	$16, %rbx
	subq	$16, %rdx
	cmpq	%rbx, %rbp
	jb	.L2492
.L2490:
	leaq	-16(%r13,%rbp), %rcx
	cmpw	$1, (%r14)
	jne	.L2495
	leaq	8(%r14), %rsi
	movq	%r13, %rdx
	movq	%rbp, %rdi
	call	memcpy
	movq	%r14, %rdi
	call	free_empty_array
	jmp	.L3363
.L2496:
	movq	%r12, %rax
	salq	$4, %rax
	leaq	(%r14,%rax), %rax
	leaq	8(%rax), %rdx
	cmpw	$16, 8(%rax)
	jne	.L2497
	movq	8(%rdx), %rax
	testq	%rax, %rax
	je	.L2499
	testb	$16, 2(%rax)
	je	.L2497
.L2499:
	movq	const0u(%rip), %rax
	movq	%rax, (%rcx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	jmp	.L2501
.L2497:
	movq	(%rdx), %rax
	movq	%rax, (%rcx)
	movq	8(%rdx), %rax
	movq	%rax, 8(%rcx)
	movzwl	(%rcx), %eax
	testb	$32, %ah
	je	.L2502
	cmpw	$8192, %ax
	je	.L2502
	andb	$223, %ah
	movw	%ax, (%rcx)
.L2502:
	movzwl	(%rdx), %eax
	cmpw	$4, %ax
	jne	.L2505
	testb	$1, 2(%rdx)
	je	.L2501
	movq	8(%rcx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2508
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2508:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rcx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2501
.L2505:
	testl	$17272, %eax
	je	.L2501
	movq	8(%rdx), %rax
	addw	$1, (%rax)
.L2501:
	subq	$16, %rcx
.L2495:
	subq	$1, %r12
	cmpq	$-1, %r12
	jne	.L2496
.L2472:
	movq	%r14, %rdi
	call	free_array
	jmp	.L3363
.L1740:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %edi
	salq	$3, %rdi
	movq	current_prog(%rip), %rax
	addq	64(%rax), %rdi
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	movl	$1, %esi
	call	allocate_class
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2511
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2511:
	movq	sp(%rip), %rax
	leaq	16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movw	$512, 16(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L1741:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %edi
	salq	$3, %rdi
	movq	current_prog(%rip), %rax
	addq	64(%rax), %rdi
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	movl	$0, %esi
	call	allocate_class
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2513
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2513:
	movq	sp(%rip), %rax
	leaq	16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movw	$512, 16(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L1642:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	addq	$2, %rdx
	movq	%rdx, pc(%rip)
	movl	num_varargs(%rip), %eax
	addw	%ax, 114(%rsp)
	movl	$0, num_varargs(%rip)
	movzwl	114(%rsp), %edi
	call	allocate_empty_array
	movq	%rax, %rbx
	movzwl	114(%rsp), %eax
	subl	$1, %eax
	movw	%ax, 114(%rsp)
	cmpw	$-1, %ax
	je	.L2515
.L3318:
	movq	sp(%rip), %rcx
	movzwl	%ax, %eax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbx)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbx)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	movzwl	114(%rsp), %eax
	subl	$1, %eax
	movw	%ax, 114(%rsp)
	cmpw	$-1, %ax
	jne	.L3318
.L2515:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2517
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2517:
	movq	sp(%rip), %rax
	leaq	16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movw	$8, 16(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L1643:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	addq	$2, %rdx
	movq	%rdx, pc(%rip)
	movl	num_varargs(%rip), %eax
	addw	%ax, 114(%rsp)
	movl	$0, num_varargs(%rip)
	movzwl	114(%rsp), %esi
	movzwl	%si, %eax
	salq	$4, %rax
	movq	sp(%rip), %rdi
	subq	%rax, %rdi
	movq	%rdi, sp(%rip)
	movzwl	%si, %esi
	call	load_mapping_from_aggregate
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2519
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2519:
	movq	sp(%rip), %rax
	leaq	16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movw	$32, 16(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L1720:
	movq	sp(%rip), %rbx
	movq	8(%rbx), %rbp
	movzwl	(%rbp), %eax
	cmpw	$1024, %ax
	je	.L2522
	cmpw	$2048, %ax
	jne	.L3350
	jmp	.L2523
.L2522:
	leaq	-16(%rbx), %rax
	cmpw	$2, -16(%rbx)
	.p2align 4,,5
	je	.L2524
	movl	$.LC102, %edi
	movl	$0, %eax
	call	error
	jmp	.L2526
.L2524:
	movzbl	8(%rax), %ebx
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L2527
	testb	%bl, %bl
	.p2align 4,,2
	jne	.L2527
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L2527:
	movq	global_lvalue_byte+8(%rip), %rax
	movb	%bl, (%rax)
	jmp	.L2526
.L3350:
	leaq	-16(%rbx), %r12
	movq	%rbp, %rdi
	call	int_free_svalue
	cmpw	$16, -16(%rbx)
	jne	.L2529
	movq	-8(%rbx), %rax
	testq	%rax, %rax
	je	.L2531
	testb	$16, 2(%rax)
	.p2align 4,,3
	je	.L2529
.L2531:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbp)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbp)
	jmp	.L2526
.L2529:
	movq	(%r12), %rax
	movq	%rax, (%rbp)
	movq	8(%r12), %rax
	movq	%rax, 8(%rbp)
	movzwl	(%rbp), %eax
	testb	$32, %ah
	je	.L2533
	cmpw	$8192, %ax
	je	.L2533
	andb	$223, %ah
	movw	%ax, (%rbp)
.L2533:
	movzwl	(%r12), %eax
	cmpw	$4, %ax
	jne	.L2536
	testb	$1, 2(%r12)
	je	.L2526
	movq	8(%rbp), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2539
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2539:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbp), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2526
.L2536:
	testl	$17272, %eax
	je	.L2526
	movq	8(%r12), %rax
	addw	$1, (%rax)
	jmp	.L2526
.L2523:
	leaq	-16(%rbx), %rdi
	call	assign_lvalue_range
.L2526:
	subq	$16, sp(%rip)
	jmp	.L3363
.L1722:
	movq	sp(%rip), %rax
	cmpw	$0, (%rax)
	je	.L2542
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	salq	$4, %rax
	movq	%rax, %rbx
	addq	fp(%rip), %rbx
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L2544
	movq	8(%rbx), %rdi
	testb	$1, 2(%rbx)
	je	.L2546
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2548
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2548
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rbx)
	je	.L2551
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2546
.L2551:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L2546
.L2548:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L2546
.L2544:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L2553
	testb	$32, %dh
	jne	.L2553
	movq	8(%rbx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L2546
	movzwl	(%rbx), %eax
	cmpw	$64, %ax
	je	.L2560
	cmpw	$64, %ax
	jg	.L2564
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L2558
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L2559
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L2546
	.p2align 4,,7
	jmp	.L2557
.L2564:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L2562
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L2563
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L2546
	.p2align 4,,7
	jmp	.L2561
.L2558:
	movq	8(%rbx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L2546
.L2562:
	movq	8(%rbx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L2546
.L2557:
	movq	8(%rbx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L2546
	call	dealloc_array
	jmp	.L2546
.L2561:
	movq	8(%rbx), %rdi
	cmpq	$null_buf, %rdi
	je	.L2546
	call	free
	jmp	.L2546
.L2559:
	movq	8(%rbx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L2546
.L2560:
	movq	8(%rbx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L2546
.L2563:
	movq	8(%rbx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L2546
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L2546
.L2553:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L2546
	.p2align 4,,9
	call	*8(%rbx)
.L2546:
	movq	sp(%rip), %rax
	movq	(%rax), %rdx
	movq	%rdx, (%rbx)
	movq	8(%rax), %rdx
	movq	%rdx, 8(%rbx)
	subq	$16, %rax
	movq	%rax, sp(%rip)
	jmp	.L3363
.L2542:
	subq	$16, %rax
	movq	%rax, sp(%rip)
	addq	$1, pc(%rip)
	jmp	.L3363
.L1721:
	movq	sp(%rip), %rax
	movq	8(%rax), %rbx
	leaq	-16(%rax), %rdi
	movq	%rdi, sp(%rip)
	movzwl	-16(%rax), %eax
	testw	%ax, %ax
	je	.L2569
	movzwl	(%rbx), %edx
	cmpw	$1024, %dx
	je	.L2572
	cmpw	$2048, %dx
	jne	.L3351
	.p2align 4,,5
	jmp	.L2573
.L2572:
	cmpw	$2, %ax
	.p2align 4,,7
	je	.L2574
	movl	$.LC102, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L2574:
	movzbl	8(%rdi), %ebx
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L2576
	testb	%bl, %bl
	jne	.L2576
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L2576:
	movq	global_lvalue_byte+8(%rip), %rax
	movb	%bl, (%rax)
	jmp	.L3363
.L2573:
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	copy_lvalue_range
	jmp	.L3363
.L3351:
	cmpw	$4, %dx
	jne	.L2578
	movq	8(%rbx), %rdi
	testb	$1, 2(%rbx)
	.p2align 4,,5
	je	.L2580
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2582
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2582
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rbx)
	je	.L2585
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2580
.L2585:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L2580
.L2582:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L2580
.L2578:
	movswl	%dx,%eax
	testl	$17272, %eax
	je	.L2587
	testb	$32, %ah
	jne	.L2587
	movq	8(%rbx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L2580
	movzwl	(%rbx), %eax
	cmpw	$64, %ax
	je	.L2594
	cmpw	$64, %ax
	jg	.L2598
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L2592
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L2593
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L2580
	.p2align 4,,7
	jmp	.L2591
.L2598:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L2596
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L2597
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L2580
	.p2align 4,,7
	jmp	.L2595
.L2592:
	movq	8(%rbx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L2580
.L2596:
	movq	8(%rbx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L2580
.L2591:
	movq	8(%rbx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L2580
	call	dealloc_array
	jmp	.L2580
.L2595:
	movq	8(%rbx), %rdi
	cmpq	$null_buf, %rdi
	je	.L2580
	call	free
	jmp	.L2580
.L2593:
	movq	8(%rbx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L2580
.L2594:
	movq	8(%rbx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L2580
.L2597:
	movq	8(%rbx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L2580
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L2580
.L2587:
	cmpw	$4096, %dx
	.p2align 4,,6
	jne	.L2580
	.p2align 4,,9
	call	*8(%rbx)
.L2580:
	movq	sp(%rip), %rax
	movq	(%rax), %rdx
	movq	%rdx, (%rbx)
	movq	8(%rax), %rdx
	movq	%rdx, 8(%rbx)
	subq	$16, %rax
	movq	%rax, sp(%rip)
	jmp	.L3363
.L2569:
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	jmp	.L3363
.L1669:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	addq	$2, %rdx
	movq	%rdx, pc(%rip)
	movl	function_index_offset(%rip), %eax
	addw	114(%rsp), %ax
	movw	%ax, 114(%rsp)
	movq	current_object(%rip), %rdx
	movq	40(%rdx), %rdx
	movzwl	%ax, %eax
	movq	56(%rdx), %rdx
	movzwl	(%rdx,%rax,2), %eax
	testw	%ax, %ax
	jns	.L2603
	andw	$32767, %ax
	movw	%ax, 114(%rsp)
.L2603:
	movq	current_object(%rip), %rax
	movq	40(%rax), %rdi
	movzwl	114(%rsp), %ecx
	movzwl	%cx, %edx
	movq	56(%rdi), %rax
	movzwl	(%rax,%rdx,2), %eax
	testb	$10, %al
	je	.L2605
	movzwl	%cx, %esi
	call	function_name
	movq	%rax, %rsi
	movl	$.LC103, %edi
	movl	$0, %eax
	call	error
.L2605:
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L2607
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L2607:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	$0, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movq	current_object(%rip), %rax
	movq	40(%rax), %rax
	movq	%rax, current_prog(%rip)
	movw	$2, caller_type(%rip)
	movq	pc(%rip), %rcx
	movzbl	(%rcx), %eax
	addl	num_varargs(%rip), %eax
	movq	csp(%rip), %rdx
	movl	%eax, 40(%rdx)
	addq	$1, %rcx
	movq	%rcx, pc(%rip)
	movl	$0, num_varargs(%rip)
	movzwl	114(%rsp), %r8d
	movl	$0, variable_index_offset(%rip)
	movl	$0, function_index_offset(%rip)
	movq	current_prog(%rip), %r9
	movq	56(%r9), %rcx
	movslq	%r8d,%rax
	movzwl	(%rcx,%rax,2), %eax
	movl	%eax, %edx
	andl	$32767, %edx
	cmpw	$-1, %ax
	cmovle	%edx, %r8d
	movslq	%r8d,%rax
	movzwl	(%rcx,%rax,2), %esi
	testb	$1, %sil
	je	.L2611
.L3319:
	movzwl	150(%r9), %eax
	leal	-1(%rax), %esi
	movl	$0, %edx
	testl	%esi, %esi
	jle	.L2615
	movq	104(%r9), %rdi
	movl	$0, %edx
.L2616:
	leal	1(%rsi,%rdx), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movzwl	8(%rax,%rdi), %eax
	cmpl	%eax, %r8d
	jl	.L2617
	movl	%ecx, %edx
	jmp	.L2619
.L2617:
	leal	-1(%rcx), %esi
.L2619:
	cmpl	%esi, %edx
	jl	.L2616
.L2615:
	movslq	%edx,%rdx
	salq	$4, %rdx
	movq	104(%r9), %rax
	movzwl	8(%rax,%rdx), %eax
	subl	%eax, %r8d
	addl	%eax, function_index_offset(%rip)
	movq	current_prog(%rip), %rcx
	movq	104(%rcx), %rax
	movzwl	10(%rdx,%rax), %eax
	addl	%eax, variable_index_offset(%rip)
	movq	104(%rcx), %rax
	movq	(%rdx,%rax), %r9
	movq	%r9, current_prog(%rip)
	movslq	%r8d,%rdx
	movq	56(%r9), %rax
	movzwl	(%rax,%rdx,2), %esi
	testb	$1, %sil
	jne	.L3319
.L2611:
	movzwl	10(%r9), %eax
	movl	%r8d, %edx
	subl	%eax, %edx
	movslq	%edx,%rax
	movq	%rax, %r14
	salq	$4, %r14
	movq	%r14, %rcx
	addq	48(%r9), %rcx
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	testb	$16, %sil
	je	.L2620
	movzbl	10(%rcx), %r13d
	movzbl	11(%rcx), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r13d
	jg	.L2622
	leal	1(%rax), %ebx
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L2624
	movl	$-1, %edi
.L2625:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L2624
	jmp	.L2625
.L2622:
	movl	%r13d, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2626
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2626:
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L2628
	movl	$-1, %edi
.L2629:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L2629
.L2628:
	movl	$the_null_array, %ebp
.L2624:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2630
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2630:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$8, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2632
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2632:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L2634
	movl	$-1, %edi
.L2635:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L2635
.L2634:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L2636
.L2620:
	movzbl	10(%rcx), %r13d
	movzbl	11(%rcx), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r13d, %ebp
	testl	%ebp, %ebp
	jle	.L2637
	movl	$0, %ebx
.L2641:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L2641
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2642
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2642:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L2644
	movl	$-1, %edi
.L2645:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L2644
	jmp	.L2645
.L2637:
	movl	%r12d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2646
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2646:
	testl	%ebx, %ebx
	je	.L2644
	movl	$0, %esi
.L2649:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	addl	$1, %esi
	cmpl	%ebx, %esi
	jne	.L2649
.L2644:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L2636:
	movq	current_prog(%rip), %rax
	movq	%r14, %rcx
	addq	48(%rax), %rcx
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	mov	12(%rcx), %eax
	movq	current_prog(%rip), %rdx
	addq	16(%rdx), %rax
	movq	%rax, pc(%rip)
	jmp	.L3363
.L1670:
	movq	pc(%rip), %rcx
	movzbl	(%rcx), %eax
	salq	$4, %rax
	movq	current_prog(%rip), %rdx
	movq	%rax, %rbx
	addq	104(%rdx), %rbx
	leaq	1(%rcx), %rax
	movq	%rax, pc(%rip)
	movq	(%rbx), %rbp
	movzbl	1(%rcx), %eax
	movb	%al, 114(%rsp)
	leaq	2(%rcx), %rax
	movq	%rax, pc(%rip)
	movzbl	2(%rcx), %eax
	movb	%al, 115(%rsp)
	addq	$3, %rcx
	movq	%rcx, pc(%rip)
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L2650
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L2650:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	$0, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movq	%rbp, current_prog(%rip)
	movw	$2, caller_type(%rip)
	movq	pc(%rip), %rcx
	movzbl	(%rcx), %eax
	addl	num_varargs(%rip), %eax
	movq	csp(%rip), %rdx
	movl	%eax, 40(%rdx)
	addq	$1, %rcx
	movq	%rcx, pc(%rip)
	movl	$0, num_varargs(%rip)
	movzwl	8(%rbx), %eax
	addl	%eax, function_index_offset(%rip)
	movzwl	10(%rbx), %eax
	addl	%eax, variable_index_offset(%rip)
	movzwl	114(%rsp), %edi
	movq	current_prog(%rip), %r9
	movq	56(%r9), %rcx
	movslq	%edi,%rax
	movzwl	(%rcx,%rax,2), %eax
	movl	%eax, %edx
	andl	$32767, %edx
	cmpw	$-1, %ax
	cmovle	%edx, %edi
	movslq	%edi,%rax
	movzwl	(%rcx,%rax,2), %esi
	testb	$1, %sil
	je	.L2654
.L3320:
	movzwl	150(%r9), %eax
	leal	-1(%rax), %esi
	movl	$0, %edx
	testl	%esi, %esi
	jle	.L2658
	movq	104(%r9), %r8
	movl	$0, %edx
.L2659:
	leal	1(%rdx,%rsi), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movzwl	8(%rax,%r8), %eax
	cmpl	%eax, %edi
	jl	.L2660
	movl	%ecx, %edx
	jmp	.L2662
.L2660:
	leal	-1(%rcx), %esi
.L2662:
	cmpl	%esi, %edx
	jl	.L2659
.L2658:
	movslq	%edx,%rdx
	salq	$4, %rdx
	movq	104(%r9), %rax
	movzwl	8(%rax,%rdx), %eax
	subl	%eax, %edi
	addl	%eax, function_index_offset(%rip)
	movq	current_prog(%rip), %rcx
	movq	104(%rcx), %rax
	movzwl	10(%rdx,%rax), %eax
	addl	%eax, variable_index_offset(%rip)
	movq	104(%rcx), %rax
	movq	(%rdx,%rax), %r9
	movq	%r9, current_prog(%rip)
	movslq	%edi,%rdx
	movq	56(%r9), %rax
	movzwl	(%rax,%rdx,2), %esi
	testb	$1, %sil
	jne	.L3320
.L2654:
	movzwl	10(%r9), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	movslq	%edx,%rax
	movq	%rax, %r14
	salq	$4, %r14
	movq	%r14, %rcx
	addq	48(%r9), %rcx
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	testb	$16, %sil
	je	.L2663
	movzbl	10(%rcx), %r13d
	movzbl	11(%rcx), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r13d
	jg	.L2665
	leal	1(%rax), %ebx
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L2667
	movl	$-1, %edi
.L2668:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L2667
	jmp	.L2668
.L2665:
	movl	%r13d, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2669
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2669:
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L2671
	movl	$-1, %edi
.L2672:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L2672
.L2671:
	movl	$the_null_array, %ebp
.L2667:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2673
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2673:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$8, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2675
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2675:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L2677
	movl	$-1, %edi
.L2678:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L2678
.L2677:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L2679
.L2663:
	movzbl	10(%rcx), %r13d
	movzbl	11(%rcx), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r13d, %ebp
	testl	%ebp, %ebp
	jle	.L2680
	movl	$0, %ebx
.L2684:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L2684
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2685
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2685:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L2687
	movl	$-1, %edi
.L2688:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L2687
	jmp	.L2688
.L2680:
	movl	%r12d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2689
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2689:
	testl	%ebx, %ebx
	je	.L2687
	movl	$0, %esi
.L2692:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	addl	$1, %esi
	cmpl	%ebx, %esi
	jne	.L2692
.L2687:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L2679:
	movq	current_prog(%rip), %rax
	movq	%r14, %rcx
	addq	48(%rax), %rcx
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	mov	12(%rcx), %eax
	movq	current_prog(%rip), %rdx
	addq	16(%rdx), %rax
	movq	%rax, pc(%rip)
	jmp	.L3363
.L1735:
	movq	sp(%rip), %rax
	cmpw	$2, (%rax)
	je	.L2693
	movl	$.LC104, %edi
	movl	$0, %eax
	call	error
.L2693:
	movq	sp(%rip), %rax
	notq	8(%rax)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L1640:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2695
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2695:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	$0, 8(%rax)
	jmp	.L3363
.L1641:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2697
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2697:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	$1, 8(%rax)
	jmp	.L3363
.L1683:
	movq	sp(%rip), %rdx
	movq	8(%rdx), %rcx
	movzwl	(%rcx), %eax
	cmpw	$128, %ax
	je	.L2701
	cmpw	$1024, %ax
	je	.L2702
	cmpw	$2, %ax
	jne	.L2699
	movw	$2, (%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rdx
	movq	8(%rcx), %rax
	subq	$1, %rax
	movq	%rax, 8(%rcx)
	movq	%rax, 8(%rdx)
	jmp	.L3363
.L2701:
	movw	$128, (%rdx)
	movq	sp(%rip), %rax
	movss	8(%rcx), %xmm0
	subss	.LC80(%rip), %xmm0
	movss	%xmm0, 8(%rcx)
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L2702:
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L2703
	movq	global_lvalue_byte+8(%rip), %rax
	cmpb	$1, (%rax)
	jne	.L2703
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L2703:
	movq	sp(%rip), %rax
	movw	$2, (%rax)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rcx
	movq	global_lvalue_byte+8(%rip), %rdx
	movzbl	(%rdx), %eax
	subl	$1, %eax
	movb	%al, (%rdx)
	movzbl	%al, %eax
	movq	%rax, 8(%rcx)
	jmp	.L3363
.L2699:
	movl	$.LC83, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1680:
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	movzwl	(%rdx), %eax
	cmpw	$128, %ax
	je	.L2708
	cmpw	$1024, %ax
	je	.L2709
	cmpw	$2, %ax
	jne	.L2706
	subq	$1, 8(%rdx)
	.p2align 4,,5
	jmp	.L3363
.L2708:
	movss	8(%rdx), %xmm0
	subss	.LC80(%rip), %xmm0
	movss	%xmm0, 8(%rdx)
	jmp	.L3363
.L2709:
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L2710
	movq	global_lvalue_byte+8(%rip), %rax
	cmpb	$1, (%rax)
	jne	.L2710
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L2710:
	movq	global_lvalue_byte+8(%rip), %rax
	subb	$1, (%rax)
	jmp	.L3363
.L2706:
	movl	$.LC83, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1726:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rdi
	movzwl	-16(%rdx), %esi
	movzwl	(%rdx), %ecx
	movl	%ecx, %eax
	orl	%esi, %eax
	cmpw	$128, %ax
	je	.L2715
	cmpw	$130, %ax
	je	.L2716
	cmpw	$2, %ax
	jne	.L2713
	movq	8(%rdx), %rax
	movq	%rdi, sp(%rip)
	testq	%rax, %rax
	jne	.L2717
	movl	$.LC105, %edi
	call	error
.L2717:
	movq	sp(%rip), %rsi
	movq	8(%rsi), %rcx
	movq	%rcx, %rdx
	movq	%rcx, %rax
	sarq	$63, %rdx
	idivq	24(%rsi)
	movq	%rax, 8(%rsi)
	jmp	.L3363
.L2715:
	movss	8(%rdx), %xmm0
	movq	%rdi, sp(%rip)
	ucomiss	.LC106(%rip), %xmm0
	jne	.L2719
	jp	.L2719
	movl	$.LC105, %edi
	movl	$0, %eax
	call	error
.L2719:
	movq	sp(%rip), %rax
	movss	8(%rax), %xmm0
	divss	24(%rax), %xmm0
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L2716:
	movq	%rdi, sp(%rip)
	cmpw	$2, %cx
	jne	.L2722
	cmpq	$0, 24(%rdi)
	jne	.L2724
	movl	$.LC105, %edi
	movl	$0, %eax
	call	error
.L2724:
	movq	sp(%rip), %rax
	cvtsi2ssq	24(%rax), %xmm1
	movss	8(%rax), %xmm0
	divss	%xmm1, %xmm0
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L2722:
	xorps	%xmm0, %xmm0
	ucomiss	24(%rdi), %xmm0
	jne	.L2726
	jp	.L2726
	movl	$.LC107, %edi
	movl	$0, %eax
	call	error
.L2726:
	movq	sp(%rip), %rax
	movw	$128, (%rax)
	movq	sp(%rip), %rax
	cvtsi2ssq	8(%rax), %xmm0
	divss	24(%rax), %xmm0
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L2713:
	testb	$-126, %sil
	jne	.L2729
	movl	%ebx, %ecx
	movl	$1, %edx
	movl	$130, %esi
	call	bad_argument
.L2729:
	movq	sp(%rip), %rdi
	testb	$-126, (%rdi)
	jne	.L3363
	movl	%ebx, %ecx
	movl	$2, %edx
	movl	$130, %esi
	call	bad_argument
	jmp	.L3363
.L1718:
	call	f_div_eq
	.p2align 4,,8
	jmp	.L3363
.L1673:
	.p2align 4,,8
	call	f_eq
	.p2align 4,,8
	jmp	.L3363
.L1677:
	.p2align 4,,8
	call	f_ge
	.p2align 4,,8
	jmp	.L3363
.L1678:
	.p2align 4,,8
	call	f_gt
	.p2align 4,,8
	jmp	.L3363
.L1692:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	addl	variable_index_offset(%rip), %eax
	cltq
	salq	$4, %rax
	addq	$112, %rax
	addq	current_object(%rip), %rax
	leaq	8(%rax), %rbx
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	cmpw	$16, 8(%rax)
	jne	.L2732
	movq	8(%rbx), %rax
	testb	$16, 2(%rax)
	je	.L2732
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L2735
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L2737
	testb	$16, 2(%rax)
	je	.L2735
.L2737:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2732
.L2735:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2739
	cmpw	$8192, %ax
	je	.L2739
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2739:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L2742
	testb	$1, const0u+2(%rip)
	je	.L2732
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2745
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2745:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2732
.L2742:
	testl	$17272, %eax
	je	.L2732
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L2732:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L2748
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L2748:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	cmpw	$16, (%rbx)
	jne	.L2750
	movq	8(%rbx), %rax
	testq	%rax, %rax
	je	.L2752
	testb	$16, 2(%rax)
	je	.L2750
.L2752:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3363
.L2750:
	movq	(%rbx), %rax
	movq	%rax, (%rdx)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L2754
	cmpw	$8192, %ax
	je	.L2754
	andb	$223, %ah
	movw	%ax, (%rdx)
.L2754:
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L2757
	testb	$1, 2(%rbx)
	je	.L3363
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L2760
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L2760:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3363
.L2757:
	testl	$17272, %eax
	je	.L3363
	movq	8(%rbx), %rax
	addw	$1, (%rax)
	jmp	.L3363
.L1681:
	movq	sp(%rip), %rdx
	movq	8(%rdx), %rcx
	movzwl	(%rcx), %eax
	cmpw	$128, %ax
	je	.L2765
	cmpw	$1024, %ax
	je	.L2766
	cmpw	$2, %ax
	jne	.L2763
	movw	$2, (%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rdx
	movq	8(%rcx), %rax
	addq	$1, %rax
	movq	%rax, 8(%rcx)
	movq	%rax, 8(%rdx)
	jmp	.L3363
.L2765:
	movw	$128, (%rdx)
	movq	sp(%rip), %rax
	movss	.LC80(%rip), %xmm0
	addss	8(%rcx), %xmm0
	movss	%xmm0, 8(%rcx)
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L2766:
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L2767
	movq	global_lvalue_byte+8(%rip), %rax
	cmpb	$-1, (%rax)
	jne	.L2767
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L2767:
	movq	sp(%rip), %rax
	movw	$2, (%rax)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rcx
	movq	global_lvalue_byte+8(%rip), %rdx
	movzbl	(%rdx), %eax
	addl	$1, %eax
	movb	%al, (%rdx)
	movzbl	%al, %eax
	movq	%rax, 8(%rcx)
	jmp	.L3363
.L2763:
	movl	$.LC82, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1694:
	movq	sp(%rip), %rax
	cmpw	$512, (%rax)
	je	.L2770
	movl	$.LC108, %edi
	movl	$0, %eax
	call	error
.L2770:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	movq	%rdx, 120(%rsp)
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	sp(%rip), %rax
	movq	8(%rax), %rbp
	movzwl	2(%rbp), %eax
	cmpq	%rdx, %rax
	jg	.L2772
	movl	$.LC109, %edi
	movl	$0, %eax
	call	error
.L2772:
	movq	120(%rsp), %rax
	salq	$4, %rax
	cmpw	$16, 8(%rax,%rbp)
	jne	.L2774
	leaq	(%rax,%rbp), %rdx
	movq	16(%rdx), %rax
	testb	$16, 2(%rax)
	je	.L2774
	leaq	8(%rdx), %rbx
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L2777
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L2779
	testb	$16, 2(%rax)
	je	.L2777
.L2779:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2774
.L2777:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2781
	cmpw	$8192, %ax
	je	.L2781
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2781:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L2784
	testb	$1, const0u+2(%rip)
	je	.L2774
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2787
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2787:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2774
.L2784:
	testl	$17272, %eax
	je	.L2774
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L2774:
	movq	sp(%rip), %rdx
	movq	120(%rsp), %rax
	salq	$4, %rax
	leaq	(%rbp,%rax), %rax
	leaq	8(%rax), %rcx
	cmpw	$16, 8(%rax)
	jne	.L2790
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.L2792
	testb	$16, 2(%rax)
	je	.L2790
.L2792:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L2794
.L2790:
	movq	(%rcx), %rax
	movq	%rax, (%rdx)
	movq	8(%rcx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L2795
	cmpw	$8192, %ax
	je	.L2795
	andb	$223, %ah
	movw	%ax, (%rdx)
.L2795:
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L2798
	testb	$1, 2(%rcx)
	je	.L2794
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L2801
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L2801:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2794
.L2798:
	testl	$17272, %eax
	je	.L2794
	movq	8(%rcx), %rax
	addw	$1, (%rax)
.L2794:
	movq	%rbp, %rdi
	call	free_class
	jmp	.L3363
.L1695:
	movq	sp(%rip), %rax
	cmpw	$512, (%rax)
	je	.L2804
	movl	$.LC108, %edi
	movl	$0, %eax
	call	error
.L2804:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	movq	%rdx, 120(%rsp)
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	sp(%rip), %rax
	movq	8(%rax), %rbx
	movzwl	2(%rbx), %eax
	cmpq	%rdx, %rax
	jg	.L2806
	movl	$.LC109, %edi
	movl	$0, %eax
	call	error
.L2806:
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	120(%rsp), %rax
	salq	$4, %rax
	leaq	8(%rbx,%rax), %rdx
	movq	sp(%rip), %rax
	movq	%rdx, 8(%rax)
	movl	$512, lv_owner_type(%rip)
	movq	%rbx, lv_owner(%rip)
	movq	%rbx, %rdi
	call	free_class
	jmp	.L3363
.L1696:
	movq	sp(%rip), %rdx
	movzwl	(%rdx), %eax
	cmpw	$8, %ax
	je	.L2810
	cmpw	$8, %ax
	jg	.L2813
	cmpw	$4, %ax
	jne	.L2808
	.p2align 4,,7
	jmp	.L2809
.L2813:
	cmpw	$32, %ax
	.p2align 4,,7
	je	.L2811
	cmpw	$256, %ax
	.p2align 4,,7
	jne	.L2808
	.p2align 4,,7
	jmp	.L2812
.L2811:
	movq	8(%rdx), %r12
	leaq	-16(%rdx), %rsi
	movq	%r12, %rdi
	call	find_in_mapping
	movq	%rax, %rbx
	cmpw	$16, (%rax)
	jne	.L2814
	movq	8(%rax), %rax
	testb	$16, 2(%rax)
	je	.L2814
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L2817
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L2819
	testb	$16, 2(%rax)
	je	.L2817
.L2819:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2814
.L2817:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2821
	cmpw	$8192, %ax
	je	.L2821
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2821:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L2824
	testb	$1, const0u+2(%rip)
	je	.L2814
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2827
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2827:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2814
.L2824:
	testl	$17272, %eax
	je	.L2814
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L2814:
	movq	sp(%rip), %rbp
	subq	$16, %rbp
	movq	%rbp, sp(%rip)
	movq	%rbp, %rdi
	call	int_free_svalue
	cmpw	$16, (%rbx)
	jne	.L2830
	movq	8(%rbx), %rax
	testq	%rax, %rax
	je	.L2832
	testb	$16, 2(%rax)
	.p2align 4,,3
	je	.L2830
.L2832:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbp)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbp)
	jmp	.L2834
.L2830:
	movq	(%rbx), %rax
	movq	%rax, (%rbp)
	movq	8(%rbx), %rax
	movq	%rax, 8(%rbp)
	movzwl	(%rbp), %eax
	testb	$32, %ah
	je	.L2835
	cmpw	$8192, %ax
	je	.L2835
	andb	$223, %ah
	movw	%ax, (%rbp)
.L2835:
	movzwl	(%rbx), %eax
	cmpw	$4, %ax
	jne	.L2838
	testb	$1, 2(%rbx)
	je	.L2834
	movq	8(%rbp), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2841
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2841:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbp), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2834
.L2838:
	testl	$17272, %eax
	je	.L2834
	movq	8(%rbx), %rax
	addw	$1, (%rax)
.L2834:
	movq	%r12, %rdi
	call	free_mapping
	jmp	.L3363
.L2812:
	cmpw	$2, -16(%rdx)
	je	.L2844
	movl	$.LC110, %edi
	movl	$0, %eax
	call	error
.L2844:
	movq	sp(%rip), %rax
	movq	-8(%rax), %rdx
	movq	%rdx, 120(%rsp)
	movq	8(%rax), %rax
	mov	4(%rax), %eax
	cmpq	%rax, %rdx
	jg	.L2846
	testq	%rdx, %rdx
	jns	.L2848
.L2846:
	movl	$.LC72, %edi
	movl	$0, %eax
	call	error
.L2848:
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdx
	movq	120(%rsp), %rax
	movzbl	8(%rdx,%rax), %eax
	movq	%rax, 120(%rsp)
	movq	8(%rcx), %rdi
	call	free_buffer
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	movq	120(%rsp), %rax
	movq	%rax, -8(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L2809:
	cmpw	$2, -16(%rdx)
	je	.L2849
	movl	$.LC111, %edi
	movl	$0, %eax
	call	error
.L2849:
	movq	sp(%rip), %rax
	movq	-8(%rax), %rsi
	movq	%rsi, 120(%rsp)
	testb	$1, 2(%rax)
	je	.L2851
	movq	8(%rax), %rax
	movzwl	-4(%rax), %edx
	movzwl	%dx, %eax
	movl	%eax, svalue_strlen_size(%rip)
	cltq
	cmpw	$-1, %dx
	jne	.L2855
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leaq	65534(%rcx), %rax
	jmp	.L2855
.L2851:
	movq	8(%rax), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leaq	-1(%rcx), %rax
.L2855:
	cmpq	%rsi, %rax
	jb	.L2856
	cmpq	$0, 120(%rsp)
	jns	.L2858
.L2856:
	movl	$.LC112, %edi
	movl	$0, %eax
	call	error
.L2858:
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rax
	movq	120(%rsp), %rdx
	movzbl	(%rax,%rdx), %eax
	movq	%rax, 120(%rsp)
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2859
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2861
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2861
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2864
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2859
.L2864:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2859
.L2861:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2859:
	movq	sp(%rip), %rax
	leaq	-16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movq	120(%rsp), %rdx
	movq	%rdx, -8(%rax)
	jmp	.L3363
.L2810:
	cmpw	$2, -16(%rdx)
	je	.L2866
	movl	$.LC113, %edi
	movl	$0, %eax
	call	error
.L2866:
	movq	sp(%rip), %rax
	movq	-8(%rax), %rax
	movq	%rax, 120(%rsp)
	testq	%rax, %rax
	jns	.L2868
	movl	$.LC114, %edi
	movl	$0, %eax
	call	error
.L2868:
	movq	sp(%rip), %rax
	movq	8(%rax), %rbp
	movzwl	2(%rbp), %eax
	cmpq	120(%rsp), %rax
	jg	.L2870
	movl	$.LC77, %edi
	movl	$0, %eax
	call	error
.L2870:
	movq	120(%rsp), %rax
	salq	$4, %rax
	cmpw	$16, 8(%rax,%rbp)
	jne	.L2872
	leaq	(%rax,%rbp), %rdx
	movq	16(%rdx), %rax
	testb	$16, 2(%rax)
	je	.L2872
	leaq	8(%rdx), %rbx
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L2875
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L2877
	testb	$16, 2(%rax)
	je	.L2875
.L2877:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2872
.L2875:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2879
	cmpw	$8192, %ax
	je	.L2879
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2879:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L2882
	testb	$1, const0u+2(%rip)
	je	.L2872
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2885
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2885:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2872
.L2882:
	testl	$17272, %eax
	je	.L2872
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L2872:
	movq	sp(%rip), %rdx
	subq	$16, %rdx
	movq	%rdx, sp(%rip)
	movq	120(%rsp), %rax
	salq	$4, %rax
	leaq	(%rbp,%rax), %rax
	leaq	8(%rax), %rcx
	cmpw	$16, 8(%rax)
	jne	.L2888
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.L2890
	testb	$16, 2(%rax)
	je	.L2888
.L2890:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L2892
.L2888:
	movq	(%rcx), %rax
	movq	%rax, (%rdx)
	movq	8(%rcx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L2893
	cmpw	$8192, %ax
	je	.L2893
	andb	$223, %ah
	movw	%ax, (%rdx)
.L2893:
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L2896
	testb	$1, 2(%rcx)
	je	.L2892
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L2899
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L2899:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2892
.L2896:
	testl	$17272, %eax
	je	.L2892
	movq	8(%rcx), %rax
	addw	$1, (%rax)
.L2892:
	movq	%rbp, %rdi
	call	free_array
	jmp	.L3363
.L2808:
	cmpw	$2, %ax
	jne	.L2902
	cmpq	$0, 8(%rdx)
	.p2align 4,,7
	jne	.L2902
	movl	$.LC74, %edi
	movl	$0, %eax
	call	error
.L2902:
	movq	sp(%rip), %rax
	movswl	(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movl	$.LC75, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1698:
	movq	sp(%rip), %rdx
	movzwl	(%rdx), %eax
	cmpw	$8, %ax
	je	.L2907
	cmpw	$256, %ax
	je	.L2908
	cmpw	$4, %ax
	jne	.L3354
	.p2align 4,,7
	jmp	.L2906
.L2908:
	cmpw	$2, -16(%rdx)
	.p2align 4,,7
	je	.L2909
	movl	$.LC115, %edi
	movl	$0, %eax
	call	error
.L2909:
	movq	sp(%rip), %rdx
	movq	8(%rdx), %rax
	mov	4(%rax), %eax
	movq	%rax, %rcx
	subq	-8(%rdx), %rcx
	movq	%rcx, 120(%rsp)
	movq	8(%rdx), %rax
	mov	4(%rax), %eax
	cmpq	%rax, %rcx
	jg	.L2911
	testq	%rcx, %rcx
	jns	.L2913
.L2911:
	movl	$.LC72, %edi
	movl	$0, %eax
	call	error
.L2913:
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rdx
	movq	120(%rsp), %rax
	movzbl	8(%rdx,%rax), %eax
	movq	%rax, 120(%rsp)
	movq	8(%rcx), %rdi
	call	free_buffer
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	movq	120(%rsp), %rax
	movq	%rax, -8(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L2906:
	testb	$1, 2(%rdx)
	je	.L2914
	movq	8(%rdx), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %ebx
	movl	%ebx, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L2916
	movq	sp(%rip), %rax
	movq	8(%rax), %rdi
	addq	$65535, %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	65534(%rcx), %ebx
	jmp	.L2916
.L2914:
	movq	8(%rdx), %rdi
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	repnz
	scasb
	notq	%rcx
	leal	-1(%rcx), %ebx
.L2916:
	movq	sp(%rip), %rax
	cmpw	$2, -16(%rax)
	je	.L2918
	movl	$.LC116, %edi
	movl	$0, %eax
	call	error
.L2918:
	movslq	%ebx,%rdx
	movq	sp(%rip), %rax
	movq	%rdx, %rcx
	subq	-8(%rax), %rcx
	movq	%rcx, 120(%rsp)
	cmpq	%rcx, %rdx
	jl	.L2920
	testq	%rcx, %rcx
	jns	.L2922
.L2920:
	movl	$.LC112, %edi
	movl	$0, %eax
	call	error
.L2922:
	movq	sp(%rip), %rcx
	movq	8(%rcx), %rax
	movq	120(%rsp), %rdx
	movzbl	(%rax,%rdx), %eax
	movq	%rax, 120(%rsp)
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2923
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2925
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2925
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2928
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2923
.L2928:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2923
.L2925:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
.L2923:
	movq	sp(%rip), %rax
	leaq	-16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movq	120(%rsp), %rdx
	movq	%rdx, -8(%rax)
	jmp	.L3363
.L2907:
	movq	8(%rdx), %rbp
	cmpw	$2, -16(%rdx)
	je	.L2930
	movl	$.LC117, %edi
	movl	$0, %eax
	call	error
.L2930:
	movzwl	2(%rbp), %edx
	movq	sp(%rip), %rax
	subq	-8(%rax), %rdx
	movq	%rdx, 120(%rsp)
	testq	%rdx, %rdx
	js	.L2932
	movzwl	2(%rbp), %eax
	cmpq	%rax, %rdx
	jl	.L2934
.L2932:
	movl	$.LC77, %edi
	movl	$0, %eax
	call	error
.L2934:
	movq	120(%rsp), %rax
	salq	$4, %rax
	cmpw	$16, 8(%rax,%rbp)
	jne	.L2935
	leaq	(%rax,%rbp), %rdx
	movq	16(%rdx), %rax
	testb	$16, 2(%rax)
	je	.L2935
	leaq	8(%rdx), %rbx
	movq	%rbx, %rdi
	call	int_free_svalue
	cmpw	$16, const0u(%rip)
	jne	.L2938
	movq	const0u+8(%rip), %rax
	testq	%rax, %rax
	je	.L2940
	testb	$16, 2(%rax)
	je	.L2938
.L2940:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	jmp	.L2935
.L2938:
	movq	const0u(%rip), %rax
	movq	%rax, (%rbx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rbx)
	movzwl	(%rbx), %eax
	testb	$32, %ah
	je	.L2942
	cmpw	$8192, %ax
	je	.L2942
	andb	$223, %ah
	movw	%ax, (%rbx)
.L2942:
	movzwl	const0u(%rip), %eax
	cmpw	$4, %ax
	jne	.L2945
	testb	$1, const0u+2(%rip)
	je	.L2935
	movq	8(%rbx), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L2948
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L2948:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rbx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2935
.L2945:
	testl	$17272, %eax
	je	.L2935
	movq	const0u+8(%rip), %rax
	addw	$1, (%rax)
.L2935:
	movq	sp(%rip), %rdx
	subq	$16, %rdx
	movq	%rdx, sp(%rip)
	movq	120(%rsp), %rax
	salq	$4, %rax
	leaq	(%rbp,%rax), %rax
	leaq	8(%rax), %rcx
	cmpw	$16, 8(%rax)
	jne	.L2951
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.L2953
	testb	$16, 2(%rax)
	je	.L2951
.L2953:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L2955
.L2951:
	movq	(%rcx), %rax
	movq	%rax, (%rdx)
	movq	8(%rcx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L2956
	cmpw	$8192, %ax
	je	.L2956
	andb	$223, %ah
	movw	%ax, (%rdx)
.L2956:
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L2959
	testb	$1, 2(%rcx)
	je	.L2955
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L2962
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L2962:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L2955
.L2959:
	testl	$17272, %eax
	je	.L2955
	movq	8(%rcx), %rax
	addw	$1, (%rax)
.L2955:
	movq	%rbp, %rdi
	call	free_array
	jmp	.L3363
.L3354:
	cmpw	$2, %ax
	jne	.L2965
	cmpq	$0, 8(%rdx)
	.p2align 4,,7
	jne	.L2965
	movl	$.LC74, %edi
	movl	$0, %eax
	call	error
.L2965:
	movq	sp(%rip), %rax
	movswl	(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movl	$.LC75, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1675:
	call	f_le
	.p2align 4,,8
	jmp	.L3363
.L1731:
	.p2align 4,,8
	call	f_lsh
	.p2align 4,,8
	jmp	.L3363
.L1715:
	.p2align 4,,8
	call	f_lsh_eq
	.p2align 4,,8
	jmp	.L3363
.L1727:
	movq	sp(%rip), %rdi
	subq	$16, %rdi
	testb	$2, (%rdi)
	jne	.L2968
	movl	%ebx, %ecx
	movl	$1, %edx
	movl	$2, %esi
	call	bad_argument
.L2968:
	movq	sp(%rip), %rdi
	testb	$2, (%rdi)
	jne	.L2970
	movl	%ebx, %ecx
	movl	$2, %edx
	movl	$2, %esi
	call	bad_argument
.L2970:
	movq	sp(%rip), %rax
	movq	8(%rax), %rdx
	subq	$16, %rax
	movq	%rax, sp(%rip)
	testq	%rdx, %rdx
	jne	.L2972
	movl	$.LC118, %edi
	movl	$0, %eax
	call	error
.L2972:
	movq	sp(%rip), %rsi
	movq	8(%rsi), %rdx
	movq	%rdx, %rax
	sarq	$63, %rdx
	idivq	24(%rsi)
	movq	%rdx, 8(%rsi)
	jmp	.L3363
.L1719:
	call	f_mod_eq
	jmp	.L3363
.L1725:
	movq	sp(%rip), %rax
	leaq	-16(%rax), %rdi
	movzwl	-16(%rax), %ecx
	movl	%ecx, %edx
	orw	(%rax), %dx
	cmpw	$32, %dx
	je	.L2976
	cmpw	$32, %dx
	jg	.L2979
	cmpw	$2, %dx
	jne	.L2974
	.p2align 4,,7
	jmp	.L2975
.L2979:
	cmpw	$128, %dx
	.p2align 4,,7
	je	.L2977
	cmpw	$130, %dx
	.p2align 4,,7
	jne	.L2974
	.p2align 4,,7
	jmp	.L2978
.L2975:
	movq	%rdi, sp(%rip)
	movq	8(%rdi), %rax
	imulq	24(%rdi), %rax
	movq	%rax, 8(%rdi)
	jmp	.L3363
.L2977:
	movq	%rdi, sp(%rip)
	movss	8(%rdi), %xmm0
	mulss	24(%rdi), %xmm0
	movss	%xmm0, 8(%rdi)
	jmp	.L3363
.L2978:
	movq	%rdi, sp(%rip)
	cmpw	$2, (%rdi)
	jne	.L2980
	movw	$128, (%rdi)
	movq	sp(%rip), %rax
	cvtsi2ssq	8(%rax), %xmm0
	mulss	24(%rax), %xmm0
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L2980:
	cvtsi2ssq	24(%rdi), %xmm0
	mulss	8(%rdi), %xmm0
	movss	%xmm0, 8(%rdi)
	jmp	.L3363
.L2976:
	movq	8(%rax), %rsi
	movq	8(%rdi), %rdi
	movl	$1, %edx
	call	compose_mapping
	movq	%rax, %rbx
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L2982
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L2984
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L2986
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L2986
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L2989
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L2984
.L2989:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L2984
.L2986:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L2984
.L2982:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L2991
	testb	$32, %dh
	jne	.L2991
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L2984
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L2998
	cmpw	$64, %ax
	jg	.L3002
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L2996
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L2997
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L2984
	.p2align 4,,7
	jmp	.L2995
.L3002:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L3000
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L3001
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L2984
	.p2align 4,,7
	jmp	.L2999
.L2996:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L2984
.L3000:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L2984
.L2995:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L2984
	call	dealloc_array
	jmp	.L2984
.L2999:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L2984
	call	free
	jmp	.L2984
.L2997:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L2984
.L2998:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L2984
.L3001:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L2984
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L2984
.L2991:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L2984
	.p2align 4,,9
	call	*8(%rcx)
.L2984:
	movq	sp(%rip), %rcx
	leaq	-16(%rcx), %rax
	movq	%rax, sp(%rip)
	movzwl	(%rcx), %eax
	cmpw	$4, %ax
	jne	.L3007
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L3009
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L3011
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L3011
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L3014
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L3009
.L3014:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L3009
.L3011:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L3009
.L3007:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L3016
	testb	$32, %dh
	jne	.L3016
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L3009
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L3023
	cmpw	$64, %ax
	jg	.L3027
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L3021
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L3022
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L3009
	.p2align 4,,7
	jmp	.L3020
.L3027:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L3025
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L3026
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L3009
	.p2align 4,,7
	jmp	.L3024
.L3021:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L3009
.L3025:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L3009
.L3020:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L3009
	call	dealloc_array
	jmp	.L3009
.L3024:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L3009
	call	free
	jmp	.L3009
.L3022:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L3009
.L3023:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L3009
.L3026:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L3009
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L3009
.L3016:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L3009
	.p2align 4,,9
	call	*8(%rcx)
.L3009:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3032
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3032:
	movq	sp(%rip), %rax
	leaq	16(%rax), %rdx
	movq	%rdx, sp(%rip)
	movw	$32, 16(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L2974:
	testb	$-94, %cl
	jne	.L3034
	movl	%ebx, %ecx
	movl	$1, %edx
	movl	$162, %esi
	call	bad_argument
.L3034:
	movq	sp(%rip), %rdi
	testb	$-94, (%rdi)
	jne	.L3036
	movl	%ebx, %ecx
	movl	$2, %edx
	movl	$162, %esi
	call	bad_argument
.L3036:
	movl	$.LC119, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1717:
	call	f_mult_eq
	.p2align 4,,8
	jmp	.L3363
.L1674:
	.p2align 4,,8
	call	f_ne
	.p2align 4,,8
	jmp	.L3363
.L1734:
	movq	sp(%rip), %rax
	movzwl	(%rax), %edx
	cmpw	$2, %dx
	jne	.L3038
	negq	8(%rax)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L3038:
	cmpw	$128, %dx
	jne	.L3040
	xorb	$-128, 11(%rax)
	jmp	.L3363
.L3040:
	movl	$.LC121, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1733:
	movq	sp(%rip), %rcx
	movzwl	(%rcx), %eax
	cmpw	$2, %ax
	jne	.L3042
	cmpq	$0, 8(%rcx)
	sete	%al
	movzbl	%al, %eax
	movq	%rax, 8(%rcx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L3042:
	cmpw	$4, %ax
	jne	.L3044
	movq	8(%rcx), %rdi
	testb	$1, 2(%rcx)
	je	.L3046
	leaq	-4(%rdi), %rsi
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L3048
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L3048
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, 2(%rcx)
	je	.L3051
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L3046
.L3051:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rsi, %rdi
	call	free
	jmp	.L3046
.L3048:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L3046
.L3044:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L3053
	testb	$32, %dh
	jne	.L3053
	movq	8(%rcx), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L3046
	movzwl	(%rcx), %eax
	cmpw	$64, %ax
	je	.L3060
	cmpw	$64, %ax
	jg	.L3064
	cmpw	$16, %ax
	.p2align 4,,3
	je	.L3058
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L3059
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L3046
	.p2align 4,,7
	jmp	.L3057
.L3064:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L3062
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L3063
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L3046
	.p2align 4,,7
	jmp	.L3061
.L3058:
	movq	8(%rcx), %rdi
	movl	$.LC52, %esi
	call	dealloc_object
	jmp	.L3046
.L3062:
	movq	8(%rcx), %rdi
	call	dealloc_class
	.p2align 4,,6
	jmp	.L3046
.L3057:
	movq	8(%rcx), %rdi
	cmpq	$the_null_array, %rdi
	je	.L3046
	call	dealloc_array
	jmp	.L3046
.L3061:
	movq	8(%rcx), %rdi
	cmpq	$null_buf, %rdi
	je	.L3046
	call	free
	jmp	.L3046
.L3059:
	movq	8(%rcx), %rdi
	.p2align 4,,6
	call	dealloc_mapping
	.p2align 4,,6
	jmp	.L3046
.L3060:
	movq	8(%rcx), %rdi
	call	dealloc_funp
	.p2align 4,,6
	jmp	.L3046
.L3063:
	movq	8(%rcx), %rdi
	cmpq	$0, 32(%rdi)
	.p2align 4,,2
	jne	.L3046
	.p2align 4,,7
	call	kill_ref
	.p2align 4,,4
	jmp	.L3046
.L3053:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L3046
	.p2align 4,,9
	call	*8(%rcx)
.L3046:
	movq	sp(%rip), %rax
	movq	const0(%rip), %rdx
	movq	%rdx, (%rax)
	movq	const0+8(%rip), %rdx
	movq	%rdx, 8(%rax)
	jmp	.L3363
.L1729:
	call	f_or
	jmp	.L3363
.L1713:
	.p2align 4,,8
	call	f_or_eq
	.p2align 4,,8
	jmp	.L3363
.L1739:
	.p2align 4,,8
	call	f_parse_command
	.p2align 4,,8
	jmp	.L3363
.L1626:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	jmp	.L3363
.L1684:
	movq	sp(%rip), %rdx
	movq	8(%rdx), %rcx
	movzwl	(%rcx), %eax
	cmpw	$128, %ax
	je	.L3071
	cmpw	$1024, %ax
	je	.L3072
	cmpw	$2, %ax
	jne	.L3069
	movw	$2, (%rdx)
	movq	8(%rcx), %rax
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	subq	$1, %rax
	movq	%rax, 8(%rcx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L3071:
	movw	$128, (%rdx)
	movss	8(%rcx), %xmm0
	movq	sp(%rip), %rax
	movss	%xmm0, 8(%rax)
	subss	.LC80(%rip), %xmm0
	movss	%xmm0, 8(%rcx)
	jmp	.L3363
.L3072:
	movw	$2, (%rdx)
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L3073
	movq	global_lvalue_byte+8(%rip), %rax
	cmpb	$1, (%rax)
	jne	.L3073
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L3073:
	movq	global_lvalue_byte+8(%rip), %rsi
	movzbl	(%rsi), %eax
	movzbl	%al, %ecx
	movq	sp(%rip), %rdx
	movq	%rcx, 8(%rdx)
	subl	$1, %eax
	movb	%al, (%rsi)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L3069:
	movl	$.LC83, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1682:
	movq	sp(%rip), %rdx
	movq	8(%rdx), %rcx
	movzwl	(%rcx), %eax
	cmpw	$128, %ax
	je	.L3078
	cmpw	$1024, %ax
	je	.L3079
	cmpw	$2, %ax
	jne	.L3076
	movw	$2, (%rdx)
	movq	8(%rcx), %rax
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	addq	$1, %rax
	movq	%rax, 8(%rcx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L3078:
	movw	$128, (%rdx)
	movss	8(%rcx), %xmm0
	movq	sp(%rip), %rax
	movss	%xmm0, 8(%rax)
	addss	.LC80(%rip), %xmm0
	movss	%xmm0, 8(%rcx)
	jmp	.L3363
.L3079:
	cmpw	$0, global_lvalue_byte+2(%rip)
	jne	.L3080
	movq	global_lvalue_byte+8(%rip), %rax
	cmpb	$-1, (%rax)
	jne	.L3080
	movl	$.LC81, %edi
	movl	$0, %eax
	call	error
.L3080:
	movq	sp(%rip), %rax
	movw	$2, (%rax)
	movq	global_lvalue_byte+8(%rip), %rsi
	movzbl	(%rsi), %eax
	movzbl	%al, %ecx
	movq	sp(%rip), %rdx
	movq	%rcx, 8(%rdx)
	addl	$1, %eax
	movb	%al, (%rsi)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	jmp	.L3363
.L3076:
	movl	$.LC82, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1693:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3083
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3083:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$1, 16(%rdx)
	movq	pc(%rip), %rcx
	movzbl	(%rcx), %eax
	addl	variable_index_offset(%rip), %eax
	cltq
	salq	$4, %rax
	addq	current_object(%rip), %rax
	leaq	120(%rax), %rax
	movq	sp(%rip), %rdx
	movq	%rax, 8(%rdx)
	addq	$1, %rcx
	movq	%rcx, pc(%rip)
	jmp	.L3363
.L1697:
	movl	$0, %edi
	call	push_indexed_lvalue
	jmp	.L3363
.L1699:
	movl	$1, %edi
	call	push_indexed_lvalue
	.p2align 4,,6
	jmp	.L3363
.L1701:
	movq	sp(%rip), %rax
	cmpw	$1, (%rax)
	jne	.L3085
	movq	8(%rax), %rsi
	movq	%rsi, global_lvalue_range+16(%rip)
	movzwl	(%rsi), %eax
	cmpw	$8, %ax
	je	.L3089
	cmpw	$256, %ax
	je	.L3090
	cmpw	$4, %ax
	jne	.L3355
	.p2align 4,,7
	jmp	.L3088
.L3089:
	movq	8(%rsi), %rax
	movzwl	2(%rax), %eax
	movl	%eax, 16(%rsp)
	.p2align 4,,2
	jmp	.L3091
.L3088:
	testb	$1, 2(%rsi)
	.p2align 4,,2
	je	.L3092
	movq	8(%rsi), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %edx
	movl	%edx, 16(%rsp)
	movl	%edx, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L3094
	movq	8(%rsi), %rdx
	addq	$65535, %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	addl	$65534, %ecx
	movl	%ecx, 16(%rsp)
	jmp	.L3094
.L3092:
	movq	8(%rsi), %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	subl	$1, %ecx
	movl	%ecx, 16(%rsp)
.L3094:
	movq	%rsi, %rdi
	call	unlink_string_svalue
	jmp	.L3091
.L3090:
	movq	8(%rsi), %rax
	movl	4(%rax), %eax
	movl	%eax, 16(%rsp)
	jmp	.L3091
.L3355:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
	jmp	.L3091
.L3085:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
.L3091:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3096
	movl	$.LC123, %edi
	movl	$0, %eax
	call	error
.L3096:
	movq	sp(%rip), %rax
	movl	8(%rax), %ebp
	addl	$1, %ebp
	js	.L3098
	cmpl	%ebp, 16(%rsp)
	jge	.L3100
.L3098:
	movl	$.LC124, %edi
	movl	$0, %eax
	call	error
.L3100:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3101
	movl	$.LC125, %edi
	movl	$0, %eax
	call	error
.L3101:
	movq	sp(%rip), %rax
	movl	8(%rax), %ebx
	testl	%ebx, %ebx
	js	.L3105
	cmpl	%ebx, 16(%rsp)
	jge	.L3103
.L3105:
	movl	$.LC126, %edi
	movl	$0, %eax
	call	error
.L3103:
	movl	%ebx, global_lvalue_range(%rip)
	movl	%ebp, global_lvalue_range+4(%rip)
	movl	16(%rsp), %eax
	movl	%eax, global_lvalue_range+8(%rip)
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	$global_lvalue_range_sv, 8(%rax)
	jmp	.L3363
.L1703:
	movq	sp(%rip), %rax
	cmpw	$1, (%rax)
	jne	.L3106
	movq	8(%rax), %rsi
	movq	%rsi, global_lvalue_range+16(%rip)
	movzwl	(%rsi), %eax
	cmpw	$8, %ax
	je	.L3110
	cmpw	$256, %ax
	je	.L3111
	cmpw	$4, %ax
	jne	.L3356
	.p2align 4,,7
	jmp	.L3109
.L3110:
	movq	8(%rsi), %rax
	movzwl	2(%rax), %eax
	movl	%eax, 20(%rsp)
	.p2align 4,,2
	jmp	.L3112
.L3109:
	testb	$1, 2(%rsi)
	.p2align 4,,2
	je	.L3113
	movq	8(%rsi), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %edx
	movl	%edx, 20(%rsp)
	movl	%edx, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L3115
	movq	8(%rsi), %rdx
	addq	$65535, %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	addl	$65534, %ecx
	movl	%ecx, 20(%rsp)
	jmp	.L3115
.L3113:
	movq	8(%rsi), %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	subl	$1, %ecx
	movl	%ecx, 20(%rsp)
.L3115:
	movq	%rsi, %rdi
	call	unlink_string_svalue
	jmp	.L3112
.L3111:
	movq	8(%rsi), %rax
	movl	4(%rax), %eax
	movl	%eax, 20(%rsp)
	jmp	.L3112
.L3356:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
	jmp	.L3112
.L3106:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
.L3112:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3117
	movl	$.LC123, %edi
	movl	$0, %eax
	call	error
.L3117:
	movq	sp(%rip), %rax
	movl	8(%rax), %ebp
	addl	$1, %ebp
	js	.L3119
	cmpl	%ebp, 20(%rsp)
	jge	.L3121
.L3119:
	movl	$.LC124, %edi
	movl	$0, %eax
	call	error
.L3121:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3122
	movl	$.LC125, %edi
	movl	$0, %eax
	call	error
.L3122:
	movq	sp(%rip), %rax
	movl	20(%rsp), %ebx
	subl	8(%rax), %ebx
	js	.L3126
	cmpl	%ebx, 20(%rsp)
	jge	.L3124
.L3126:
	movl	$.LC126, %edi
	movl	$0, %eax
	call	error
.L3124:
	movl	%ebx, global_lvalue_range(%rip)
	movl	%ebp, global_lvalue_range+4(%rip)
	movl	20(%rsp), %eax
	movl	%eax, global_lvalue_range+8(%rip)
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	$global_lvalue_range_sv, 8(%rax)
	jmp	.L3363
.L1705:
	movq	sp(%rip), %rax
	cmpw	$1, (%rax)
	jne	.L3127
	movq	8(%rax), %rsi
	movq	%rsi, global_lvalue_range+16(%rip)
	movzwl	(%rsi), %eax
	cmpw	$8, %ax
	je	.L3131
	cmpw	$256, %ax
	je	.L3132
	cmpw	$4, %ax
	jne	.L3357
	.p2align 4,,7
	jmp	.L3130
.L3131:
	movq	8(%rsi), %rax
	movzwl	2(%rax), %eax
	movl	%eax, 24(%rsp)
	.p2align 4,,2
	jmp	.L3133
.L3130:
	testb	$1, 2(%rsi)
	.p2align 4,,2
	je	.L3134
	movq	8(%rsi), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %edx
	movl	%edx, 24(%rsp)
	movl	%edx, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L3136
	movq	8(%rsi), %rdx
	addq	$65535, %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	addl	$65534, %ecx
	movl	%ecx, 24(%rsp)
	jmp	.L3136
.L3134:
	movq	8(%rsi), %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	subl	$1, %ecx
	movl	%ecx, 24(%rsp)
.L3136:
	movq	%rsi, %rdi
	call	unlink_string_svalue
	jmp	.L3133
.L3132:
	movq	8(%rsi), %rax
	movl	4(%rax), %eax
	movl	%eax, 24(%rsp)
	jmp	.L3133
.L3357:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
	jmp	.L3133
.L3127:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
.L3133:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3138
	movl	$.LC123, %edi
	movl	$0, %eax
	call	error
.L3138:
	movl	24(%rsp), %ebx
	movq	sp(%rip), %rax
	movl	%ebx, %edx
	subl	8(%rax), %edx
	movl	%edx, %ebp
	addl	$1, %ebp
	js	.L3140
	cmpl	%ebp, %ebx
	jge	.L3142
.L3140:
	movl	$.LC124, %edi
	movl	$0, %eax
	call	error
.L3142:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3143
	movl	$.LC125, %edi
	movl	$0, %eax
	call	error
.L3143:
	movq	sp(%rip), %rax
	subl	8(%rax), %ebx
	js	.L3147
	cmpl	%ebx, 24(%rsp)
	jge	.L3145
.L3147:
	movl	$.LC126, %edi
	movl	$0, %eax
	call	error
.L3145:
	movl	%ebx, global_lvalue_range(%rip)
	movl	%ebp, global_lvalue_range+4(%rip)
	movl	24(%rsp), %ecx
	movl	%ecx, global_lvalue_range+8(%rip)
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	$global_lvalue_range_sv, 8(%rax)
	jmp	.L3363
.L1707:
	movq	sp(%rip), %rax
	cmpw	$1, (%rax)
	jne	.L3148
	movq	8(%rax), %rsi
	movq	%rsi, global_lvalue_range+16(%rip)
	movzwl	(%rsi), %eax
	cmpw	$8, %ax
	je	.L3152
	cmpw	$256, %ax
	je	.L3153
	cmpw	$4, %ax
	jne	.L3358
	.p2align 4,,7
	jmp	.L3151
.L3152:
	movq	8(%rsi), %rax
	movzwl	2(%rax), %eax
	movl	%eax, 28(%rsp)
	.p2align 4,,2
	jmp	.L3154
.L3151:
	testb	$1, 2(%rsi)
	.p2align 4,,2
	je	.L3155
	movq	8(%rsi), %rax
	movzwl	-4(%rax), %eax
	movzwl	%ax, %edi
	movl	%edi, 28(%rsp)
	movl	%edi, svalue_strlen_size(%rip)
	cmpw	$-1, %ax
	jne	.L3157
	movq	8(%rsi), %rdx
	addq	$65535, %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	addl	$65534, %ecx
	movl	%ecx, 28(%rsp)
	jmp	.L3157
.L3155:
	movq	8(%rsi), %rdx
	cld
	movq	$-1, %rcx
	movl	$0, %eax
	movq	%rdx, %rdi
	repnz
	scasb
	notq	%rcx
	subl	$1, %ecx
	movl	%ecx, 28(%rsp)
.L3157:
	movq	%rsi, %rdi
	call	unlink_string_svalue
	jmp	.L3154
.L3153:
	movq	8(%rsi), %rax
	movl	4(%rax), %eax
	movl	%eax, 28(%rsp)
	jmp	.L3154
.L3358:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
	jmp	.L3154
.L3148:
	movl	$.LC122, %edi
	movl	$0, %eax
	call	error
.L3154:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3159
	movl	$.LC123, %edi
	movl	$0, %eax
	call	error
.L3159:
	movq	sp(%rip), %rax
	movl	28(%rsp), %edx
	subl	8(%rax), %edx
	movl	%edx, %ebp
	addl	$1, %ebp
	js	.L3161
	cmpl	%ebp, 28(%rsp)
	jge	.L3163
.L3161:
	movl	$.LC124, %edi
	movl	$0, %eax
	call	error
.L3163:
	movq	sp(%rip), %rdx
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	cmpw	$2, -16(%rdx)
	je	.L3164
	movl	$.LC125, %edi
	movl	$0, %eax
	call	error
.L3164:
	movq	sp(%rip), %rax
	movl	8(%rax), %ebx
	testl	%ebx, %ebx
	js	.L3168
	cmpl	%ebx, 28(%rsp)
	jge	.L3166
.L3168:
	movl	$.LC126, %edi
	movl	$0, %eax
	call	error
.L3166:
	movl	%ebx, global_lvalue_range(%rip)
	movl	%ebp, global_lvalue_range+4(%rip)
	movl	28(%rsp), %ecx
	movl	%ecx, global_lvalue_range+8(%rip)
	movq	sp(%rip), %rax
	movw	$1, (%rax)
	movq	sp(%rip), %rax
	movq	$global_lvalue_range_sv, 8(%rax)
	jmp	.L3363
.L1700:
	movl	$0, %edi
	call	f_range
	jmp	.L3363
.L1702:
	movl	$16, %edi
	call	f_range
	.p2align 4,,6
	jmp	.L3363
.L1706:
	movl	$1, %edi
	call	f_range
	.p2align 4,,6
	jmp	.L3363
.L1704:
	movl	$17, %edi
	call	f_range
	.p2align 4,,6
	jmp	.L3363
.L1708:
	movl	$0, %edi
	call	f_extract_range
	.p2align 4,,6
	jmp	.L3363
.L1709:
	movl	$1, %edi
	call	f_extract_range
	.p2align 4,,6
	jmp	.L3363
.L1672:
	movq	csp(%rip), %rax
	testb	$2, (%rax)
	je	.L3169
	movzwl	catch_value(%rip), %eax
	cmpw	$4, %ax
	jne	.L3171
	movq	catch_value+8(%rip), %rdi
	testb	$1, catch_value+2(%rip)
	je	.L3173
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L3175
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L3175
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, catch_value+2(%rip)
	je	.L3178
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L3173
.L3178:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L3173
.L3175:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L3173
.L3171:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L3180
	testb	$32, %dh
	jne	.L3180
	movq	catch_value+8(%rip), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L3173
	movzwl	catch_value(%rip), %eax
	cmpw	$64, %ax
	je	.L3187
	cmpw	$64, %ax
	jg	.L3191
	cmpw	$16, %ax
	je	.L3185
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L3186
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L3173
	.p2align 4,,7
	jmp	.L3184
.L3191:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L3189
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L3190
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L3173
	.p2align 4,,7
	jmp	.L3188
.L3185:
	movl	$.LC52, %esi
	movq	catch_value+8(%rip), %rdi
	call	dealloc_object
	jmp	.L3173
.L3189:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_class
	.p2align 4,,3
	jmp	.L3173
.L3184:
	movq	catch_value+8(%rip), %rdi
	cmpq	$the_null_array, %rdi
	je	.L3173
	call	dealloc_array
	jmp	.L3173
.L3188:
	movq	catch_value+8(%rip), %rdi
	cmpq	$null_buf, %rdi
	je	.L3173
	call	free
	jmp	.L3173
.L3186:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_mapping
	.p2align 4,,3
	jmp	.L3173
.L3187:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_funp
	.p2align 4,,3
	jmp	.L3173
.L3190:
	movq	catch_value+8(%rip), %rdi
	cmpq	$0, 32(%rdi)
	jne	.L3173
	call	kill_ref
	jmp	.L3173
.L3180:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L3173
	.p2align 4,,9
	call	*catch_value+8(%rip)
.L3173:
	movq	const0(%rip), %rax
	movq	%rax, catch_value(%rip)
	movq	const0+8(%rip), %rax
	movq	%rax, catch_value+8(%rip)
	movq	csp(%rip), %rdx
	movzwl	(%rdx), %eax
	testb	$2, %al
	je	.L3196
.L3322:
	call	pop_control_stack
	movq	csp(%rip), %rdx
	movzwl	(%rdx), %eax
	testb	$2, %al
	jne	.L3322
.L3196:
	orl	$16, %eax
	movw	%ax, (%rdx)
.L3169:
	movq	sp(%rip), %rax
	subq	fp(%rip), %rax
	movq	%rax, %rbx
	shrq	$4, %rbx
	cmpl	$-1, %ebx
	je	.L3198
	movl	$-1, %ebp
.L3199:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	subl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L3199
.L3198:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3200
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3200:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0+8(%rip), %rax
	movq	%rax, 8(%rcx)
	call	pop_control_stack
	movq	csp(%rip), %rax
	movswl	80(%rax),%eax
	testb	$24, %al
	je	.L3363
	jmp	.L3310
.L1671:
	movq	csp(%rip), %rax
	testb	$2, (%rax)
	je	.L3203
	movzwl	catch_value(%rip), %eax
	cmpw	$4, %ax
	jne	.L3205
	movq	catch_value+8(%rip), %rdi
	testb	$1, catch_value+2(%rip)
	je	.L3207
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L3209
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L3209
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, catch_value+2(%rip)
	je	.L3212
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L3207
.L3212:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L3207
.L3209:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L3207
.L3205:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L3214
	testb	$32, %dh
	jne	.L3214
	movq	catch_value+8(%rip), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L3207
	movzwl	catch_value(%rip), %eax
	cmpw	$64, %ax
	je	.L3221
	cmpw	$64, %ax
	jg	.L3225
	cmpw	$16, %ax
	je	.L3219
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L3220
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L3207
	.p2align 4,,7
	jmp	.L3218
.L3225:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L3223
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L3224
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L3207
	.p2align 4,,7
	jmp	.L3222
.L3219:
	movl	$.LC52, %esi
	movq	catch_value+8(%rip), %rdi
	call	dealloc_object
	jmp	.L3207
.L3223:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_class
	.p2align 4,,3
	jmp	.L3207
.L3218:
	movq	catch_value+8(%rip), %rdi
	cmpq	$the_null_array, %rdi
	je	.L3207
	call	dealloc_array
	jmp	.L3207
.L3222:
	movq	catch_value+8(%rip), %rdi
	cmpq	$null_buf, %rdi
	je	.L3207
	call	free
	jmp	.L3207
.L3220:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_mapping
	.p2align 4,,3
	jmp	.L3207
.L3221:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_funp
	.p2align 4,,3
	jmp	.L3207
.L3224:
	movq	catch_value+8(%rip), %rdi
	cmpq	$0, 32(%rdi)
	jne	.L3207
	call	kill_ref
	jmp	.L3207
.L3214:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L3207
	.p2align 4,,9
	call	*catch_value+8(%rip)
.L3207:
	movq	const0(%rip), %rax
	movq	%rax, catch_value(%rip)
	movq	const0+8(%rip), %rax
	movq	%rax, catch_value+8(%rip)
	movq	csp(%rip), %rdx
	movzwl	(%rdx), %eax
	testb	$2, %al
	je	.L3230
.L3321:
	call	pop_control_stack
	movq	csp(%rip), %rdx
	movzwl	(%rdx), %eax
	testb	$2, %al
	jne	.L3321
.L3230:
	orl	$16, %eax
	movw	%ax, (%rdx)
.L3203:
	movq	sp(%rip), %rdx
	movq	%rdx, %rax
	subq	fp(%rip), %rax
	addq	$31, %rax
	cmpq	$15, %rax
	jbe	.L3232
	movq	(%rdx), %rax
	movq	%rax, 80(%rsp)
	movq	8(%rdx), %rax
	movq	%rax, 88(%rsp)
	leaq	-16(%rdx), %rax
	movq	%rax, sp(%rip)
	subq	fp(%rip), %rax
	movq	%rax, %rbx
	shrq	$4, %rbx
	cmpl	$-1, %ebx
	je	.L3234
	movl	$-1, %ebp
.L3235:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	subl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L3235
.L3234:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3236
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3236:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	80(%rsp), %rax
	movq	%rax, 16(%rdx)
	movq	88(%rsp), %rax
	movq	%rax, 8(%rcx)
.L3232:
	call	pop_control_stack
	movq	csp(%rip), %rax
	movswl	80(%rax),%eax
	testb	$24, %al
	je	.L3363
	jmp	.L3310
.L1732:
	call	f_rsh
	.p2align 4,,8
	jmp	.L3363
.L1716:
	.p2align 4,,8
	call	f_rsh_eq
	.p2align 4,,8
	jmp	.L3363
.L1738:
	.p2align 4,,8
	call	f_sscanf
	.p2align 4,,8
	jmp	.L3363
.L1638:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	movb	%al, 114(%rsp)
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movzbl	1(%rdx), %eax
	movb	%al, 115(%rsp)
	addq	$2, %rdx
	movq	%rdx, pc(%rip)
	movzwl	114(%rsp), %edx
	movq	current_prog(%rip), %rax
	movq	80(%rax), %rax
	movq	(%rax,%rdx,8), %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3238
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3238:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$4, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	movw	$3, 2(%rax)
	movq	%rbx, %rdi
	call	ref_string
	jmp	.L3363
.L1639:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %ecx
	movq	current_prog(%rip), %rax
	movq	80(%rax), %rax
	movq	(%rax,%rcx,8), %rbx
	addq	$1, %rdx
	movq	%rdx, pc(%rip)
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3240
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3240:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$4, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	sp(%rip), %rax
	movw	$3, 2(%rax)
	movq	%rbx, %rdi
	call	ref_string
	jmp	.L3363
.L1724:
	movq	sp(%rip), %rax
	movswq	(%rax),%rdx
	movq	%rdx, 120(%rsp)
	leaq	-16(%rax), %rbx
	movq	%rbx, sp(%rip)
	movzwl	-16(%rax), %edx
	movswq	%dx,%rax
	orq	120(%rsp), %rax
	cmpq	$8, %rax
	je	.L3244
	cmpq	$8, %rax
	jg	.L3247
	cmpq	$2, %rax
	jne	.L3242
	.p2align 4,,7
	jmp	.L3243
.L3247:
	cmpq	$128, %rax
	.p2align 4,,7
	je	.L3245
	cmpq	$130, %rax
	.p2align 4,,7
	jne	.L3242
	.p2align 4,,7
	jmp	.L3246
.L3243:
	movq	24(%rbx), %rax
	subq	%rax, 8(%rbx)
	.p2align 4,,5
	jmp	.L3363
.L3245:
	movss	8(%rbx), %xmm0
	subss	24(%rbx), %xmm0
	movss	%xmm0, 8(%rbx)
	jmp	.L3363
.L3246:
	cmpw	$128, %dx
	jne	.L3248
	cvtsi2ssq	24(%rbx), %xmm1
	movss	8(%rbx), %xmm0
	subss	%xmm1, %xmm0
	movss	%xmm0, 8(%rbx)
	jmp	.L3363
.L3248:
	movw	$128, (%rbx)
	movq	sp(%rip), %rax
	cvtsi2ssq	8(%rax), %xmm0
	subss	24(%rax), %xmm0
	movss	%xmm0, 8(%rax)
	jmp	.L3363
.L3244:
	movq	24(%rbx), %rsi
	movq	8(%rbx), %rdi
	call	subtract_array
	movq	%rax, 8(%rbx)
	jmp	.L3363
.L3242:
	leaq	16(%rbx), %rax
	movq	%rax, sp(%rip)
	testb	$-118, %dl
	jne	.L3250
	movl	$.LC127, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L3250:
	testb	$-118, (%rax)
	jne	.L3252
	movl	$.LC128, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L3252:
	movl	$.LC129, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1711:
	call	f_sub_eq
	.p2align 4,,8
	jmp	.L3363
.L1737:
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	movb	%dl, 104(%rsp)
	leaq	1(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	1(%rax), %edx
	movb	%dl, 105(%rsp)
	leaq	2(%rax), %rdx
	movq	%rdx, pc(%rip)
	movzbl	2(%rax), %esi
	addl	num_varargs(%rip), %esi
	addq	$3, %rax
	movq	%rax, pc(%rip)
	movl	$0, num_varargs(%rip)
	movzwl	104(%rsp), %edi
	call	call_simul_efun
	jmp	.L3363
.L1668:
	call	f_switch
	.p2align 4,,8
	jmp	.L3363
.L1730:
	.p2align 4,,8
	call	f_xor
	.p2align 4,,8
	jmp	.L3363
.L1714:
	.p2align 4,,8
	call	f_xor_eq
	.p2align 4,,8
	jmp	.L3363
.L1664:
	movq	pc(%rip), %rdi
	movzbl	(%rdi), %eax
	movb	%al, 114(%rsp)
	movzbl	1(%rdi), %eax
	movb	%al, 115(%rsp)
	movzwl	114(%rsp), %eax
	addq	%rdi, %rax
	movq	current_prog(%rip), %rdx
	subw	16(%rdx), %ax
	movw	%ax, 114(%rsp)
	addq	$2, %rdi
	movq	%rdi, pc(%rip)
	movzwl	%ax, %esi
	call	do_catch
	movq	csp(%rip), %rax
	movswl	80(%rax),%eax
	andl	$24, %eax
	cmpl	$24, %eax
	jne	.L3363
	jmp	.L3310
.L1665:
	movzwl	catch_value(%rip), %eax
	cmpw	$4, %ax
	jne	.L3254
	movq	catch_value+8(%rip), %rdi
	testb	$1, catch_value+2(%rip)
	je	.L3256
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L3258
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L3258
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, catch_value+2(%rip)
	je	.L3261
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L3256
.L3261:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L3256
.L3258:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L3256
.L3254:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L3263
	testb	$32, %dh
	jne	.L3263
	movq	catch_value+8(%rip), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L3256
	movzwl	catch_value(%rip), %eax
	cmpw	$64, %ax
	je	.L3270
	cmpw	$64, %ax
	jg	.L3274
	cmpw	$16, %ax
	je	.L3268
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L3269
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L3256
	.p2align 4,,7
	jmp	.L3267
.L3274:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L3272
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L3273
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L3256
	.p2align 4,,7
	jmp	.L3271
.L3268:
	movl	$.LC52, %esi
	movq	catch_value+8(%rip), %rdi
	call	dealloc_object
	jmp	.L3256
.L3272:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_class
	.p2align 4,,3
	jmp	.L3256
.L3267:
	movq	catch_value+8(%rip), %rdi
	cmpq	$the_null_array, %rdi
	je	.L3256
	call	dealloc_array
	jmp	.L3256
.L3271:
	movq	catch_value+8(%rip), %rdi
	cmpq	$null_buf, %rdi
	je	.L3256
	call	free
	jmp	.L3256
.L3269:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_mapping
	.p2align 4,,3
	jmp	.L3256
.L3270:
	movq	catch_value+8(%rip), %rdi
	call	dealloc_funp
	.p2align 4,,3
	jmp	.L3256
.L3273:
	movq	catch_value+8(%rip), %rdi
	cmpq	$0, 32(%rdi)
	jne	.L3256
	call	kill_ref
	jmp	.L3256
.L3263:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L3256
	.p2align 4,,9
	call	*catch_value+8(%rip)
.L3256:
	movq	const0(%rip), %rax
	movq	%rax, catch_value(%rip)
	movq	const0+8(%rip), %rax
	movq	%rax, catch_value+8(%rip)
	call	pop_control_stack
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3279
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3279:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	$0, 8(%rax)
	jmp	.L3310
.L1666:
	leaq	96(%rsp), %rsi
	leaq	104(%rsp), %rdi
	call	get_usec_clock
	movq	104(%rsp), %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3281
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3281:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	movq	96(%rsp), %rbx
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3283
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3283:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L1667:
	leaq	104(%rsp), %rsi
	leaq	96(%rsp), %rdi
	call	get_usec_clock
	movq	sp(%rip), %rcx
	movq	104(%rsp), %rdx
	subq	8(%rcx), %rdx
	movq	96(%rsp), %rax
	subq	-8(%rcx), %rax
	imulq	$1000000, %rax, %rax
	addq	%rax, %rdx
	movq	%rdx, 104(%rsp)
	movq	%rcx, %rax
	subq	$32, %rax
	movq	%rax, sp(%rip)
	movq	%rdx, %rbx
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3285
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3285:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$2, 16(%rdx)
	movq	sp(%rip), %rax
	movw	$0, 2(%rax)
	movq	sp(%rip), %rax
	movq	%rbx, 8(%rax)
	jmp	.L3363
.L1743:
	movq	sp(%rip), %rdi
	movl	8(%rdi), %ebx
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	movq	sp(%rip), %rcx
	movzwl	(%rcx), %edx
	movswl	%dx,%eax
	cmpl	%ebx, %eax
	je	.L3363
	cmpw	$2, %dx
	jne	.L3288
	cmpq	$0, 8(%rcx)
	jne	.L3290
	.p2align 4,,7
	jmp	.L3363
.L3288:
	cmpw	$1, %dx
	.p2align 4,,7
	je	.L3363
.L3290:
	movl	%ebx, %edi
	.p2align 4,,7
	call	type_name
	movq	%rax, %rbx
	movq	sp(%rip), %rax
	movswl	(%rax),%edi
	call	type_name
	movq	%rax, %rsi
	movq	%rbx, %rdx
	movl	$.LC130, %edi
	movl	$0, %eax
	call	error
	jmp	.L3363
.L1628:
	movl	$0, st_num_arg(%rip)
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movslq	%edx,%rdx
	movq	oefun_table.10552(%rip), %rdi
	call	*(%rdi,%rdx,8)
	jmp	.L3363
.L1629:
	movl	$1, st_num_arg(%rip)
	movq	pc(%rip), %rax
	movzbl	(%rax), %edx
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	sp(%rip), %rdi
	movslq	%edx,%rbx
	movq	%rbx, %rax
	salq	$5, %rax
	movq	instrs2.10554(%rip), %rcx
	movzwl	4(%rax,%rcx), %eax
	testw	%ax, (%rdi)
	jne	.L3291
	leal	232(%rdx), %ecx
	movswl	%ax,%esi
	movl	$1, %edx
	call	bad_argument
.L3291:
	movq	oefun_table.10552(%rip), %rdi
	call	*(%rdi,%rbx,8)
	jmp	.L3363
.L1630:
	movl	$2, st_num_arg(%rip)
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebp
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	sp(%rip), %rdi
	subq	$16, %rdi
	movslq	%ebp,%r12
	movq	%r12, %rax
	salq	$5, %rax
	movq	instrs2.10554(%rip), %rbx
	addq	%rax, %rbx
	movzwl	4(%rbx), %eax
	testw	%ax, (%rdi)
	jne	.L3293
	leal	232(%rbp), %ecx
	movswl	%ax,%esi
	movl	$1, %edx
	call	bad_argument
.L3293:
	movq	sp(%rip), %rdi
	movzwl	6(%rbx), %eax
	testw	%ax, (%rdi)
	jne	.L3295
	leal	232(%rbp), %ecx
	movswl	%ax,%esi
	movl	$2, %edx
	call	bad_argument
.L3295:
	movq	oefun_table.10552(%rip), %rax
	call	*(%rax,%r12,8)
	jmp	.L3363
.L1631:
	movl	$3, st_num_arg(%rip)
	movq	pc(%rip), %rax
	movzbl	(%rax), %ebp
	addq	$1, %rax
	movq	%rax, pc(%rip)
	movq	sp(%rip), %rdi
	subq	$32, %rdi
	movslq	%ebp,%r12
	movq	%r12, %rax
	salq	$5, %rax
	movq	instrs2.10554(%rip), %rbx
	addq	%rax, %rbx
	movzwl	4(%rbx), %eax
	testw	%ax, (%rdi)
	jne	.L3297
	leal	232(%rbp), %ecx
	movswl	%ax,%esi
	movl	$1, %edx
	call	bad_argument
.L3297:
	movq	sp(%rip), %rdi
	subq	$16, %rdi
	movzwl	6(%rbx), %eax
	testw	%ax, (%rdi)
	jne	.L3299
	leal	232(%rbp), %ecx
	movswl	%ax,%esi
	movl	$2, %edx
	call	bad_argument
.L3299:
	movq	sp(%rip), %rdi
	movzwl	8(%rbx), %eax
	testw	%ax, (%rdi)
	jne	.L3301
	leal	232(%rbp), %ecx
	movswl	%ax,%esi
	movl	$3, %edx
	call	bad_argument
.L3301:
	movq	oefun_table.10552(%rip), %rdx
	call	*(%rdx,%r12,8)
	jmp	.L3363
.L1632:
	movq	pc(%rip), %rdx
	movzbl	(%rdx), %eax
	addl	num_varargs(%rip), %eax
	movl	%eax, st_num_arg(%rip)
	leaq	1(%rdx), %rax
	movq	%rax, pc(%rip)
	movl	$0, num_varargs(%rip)
	movzbl	1(%rdx), %ecx
	addq	$2, %rdx
	movq	%rdx, pc(%rip)
	movslq	%ecx,%r13
	movq	%r13, %rax
	salq	$5, %rax
	movq	instrs2.10554(%rip), %rdi
	leaq	(%rax,%rdi), %rbp
	movswl	2(%rbp),%eax
	movq	$1, 120(%rsp)
	movslq	%eax,%rbx
	testq	%rbx, %rbx
	jle	.L3303
	movl	$1, %edx
	leal	232(%rcx), %r12d
.L3305:
	movslq	st_num_arg(%rip),%rax
	movq	%rdx, %rcx
	subq	%rax, %rcx
	movq	%rcx, %rax
	salq	$4, %rax
	movq	%rax, %rdi
	addq	sp(%rip), %rdi
	movzwl	2(%rbp,%rdx,2), %eax
	testw	%ax, (%rdi)
	jne	.L3306
	movswl	%ax,%esi
	movl	%r12d, %ecx
	call	bad_argument
.L3306:
	movq	120(%rsp), %rdx
	addq	$1, %rdx
	movq	%rdx, 120(%rsp)
	cmpq	%rbx, %rdx
	jle	.L3305
.L3303:
	movq	oefun_table.10552(%rip), %rdi
	call	*(%rdi,%r13,8)
	jmp	.L3363
.L1625:
	movl	$1, st_num_arg(%rip)
	movq	sp(%rip), %rdi
	movslq	%ebx,%rax
	salq	$5, %rax
	movzwl	instrs+4(%rax), %eax
	testw	%ax, (%rdi)
	jne	.L3308
	movswl	%ax,%esi
	movl	%ebx, %ecx
	movl	$1, %edx
	call	bad_argument
.L3308:
	movslq	%ebx,%rax
	movq	ooefun_table.10553(%rip), %rdx
	call	*(%rdx,%rax,8)
	jmp	.L3363
.L3310:
	addq	$136, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE103:
	.size	eval_instruction, .-eval_instruction
.globl call_direct
	.type	call_direct, @function
call_direct:
.LFB119:
	pushq	%r15
.LCFI192:
	pushq	%r14
.LCFI193:
	pushq	%r13
.LCFI194:
	pushq	%r12
.LCFI195:
	pushq	%rbp
.LCFI196:
	pushq	%rbx
.LCFI197:
	subq	$8, %rsp
.LCFI198:
	movq	%rdi, %r14
	movl	%esi, %ebx
	movl	%edx, %r12d
	movl	%ecx, %r13d
	movq	40(%rdi), %rbp
	movq	current_time(%rip), %rax
	movl	%eax, 32(%rdi)
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L3367
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L3367:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	$4, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movw	%r12w, caller_type(%rip)
	movq	csp(%rip), %rax
	movl	%r13d, 40(%rax)
	movq	%rbp, current_prog(%rip)
	movl	$0, variable_index_offset(%rip)
	movl	$0, function_index_offset(%rip)
	movq	%rbp, %r8
	movq	56(%rbp), %rcx
	movslq	%ebx,%rax
	movzwl	(%rcx,%rax,2), %eax
	movl	%eax, %edx
	andl	$32767, %edx
	cmpw	$-1, %ax
	cmovle	%edx, %ebx
	movslq	%ebx,%rax
	movzwl	(%rcx,%rax,2), %edi
	testb	$1, %dil
	je	.L3371
.L3411:
	movzwl	150(%r8), %eax
	leal	-1(%rax), %esi
	movl	$0, %edx
	testl	%esi, %esi
	jle	.L3375
	movq	104(%r8), %rdi
	movl	$0, %edx
.L3376:
	leal	1(%rsi,%rdx), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movzwl	8(%rax,%rdi), %eax
	cmpl	%eax, %ebx
	jl	.L3377
	movl	%ecx, %edx
	jmp	.L3379
.L3377:
	leal	-1(%rcx), %esi
.L3379:
	cmpl	%esi, %edx
	jl	.L3376
.L3375:
	movslq	%edx,%rdx
	salq	$4, %rdx
	movq	104(%r8), %rax
	movzwl	8(%rax,%rdx), %eax
	subl	%eax, %ebx
	addl	%eax, function_index_offset(%rip)
	movq	current_prog(%rip), %rcx
	movq	104(%rcx), %rax
	movzwl	10(%rdx,%rax), %eax
	addl	%eax, variable_index_offset(%rip)
	movq	104(%rcx), %rax
	movq	(%rdx,%rax), %r8
	movq	%r8, current_prog(%rip)
	movslq	%ebx,%rdx
	movq	56(%r8), %rax
	movzwl	(%rax,%rdx,2), %edi
	testb	$1, %dil
	jne	.L3411
.L3371:
	movzwl	10(%r8), %eax
	movl	%ebx, %edx
	subl	%eax, %edx
	movslq	%edx,%rax
	movq	%rax, %r15
	salq	$4, %r15
	movq	%r15, %rcx
	addq	48(%r8), %rcx
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	testb	$16, %dil
	je	.L3380
	movzbl	10(%rcx), %r13d
	movzbl	11(%rcx), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r13d
	jg	.L3382
	leal	1(%rax), %ebx
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3384
	movl	$-1, %edi
.L3385:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3384
	jmp	.L3385
.L3382:
	movl	%r13d, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3386
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3386:
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3388
	movl	$-1, %edi
.L3389:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3389
.L3388:
	movl	$the_null_array, %ebp
.L3384:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3390
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3390:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$8, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3392
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3392:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L3394
	movl	$-1, %edi
.L3395:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3395
.L3394:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L3396
.L3380:
	movzbl	10(%rcx), %r13d
	movzbl	11(%rcx), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r13d, %ebp
	testl	%ebp, %ebp
	jle	.L3397
	movl	$0, %ebx
.L3401:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L3401
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3402
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3402:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L3404
	movl	$-1, %edi
.L3405:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3404
	jmp	.L3405
.L3397:
	movl	%r12d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3406
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3406:
	testl	%ebx, %ebx
	je	.L3404
	movl	$0, %esi
.L3409:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	addl	$1, %esi
	cmpl	%ebx, %esi
	jne	.L3409
.L3404:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L3396:
	movq	current_prog(%rip), %rdx
	movq	%r15, %rcx
	addq	48(%rdx), %rcx
	movq	current_object(%rip), %rax
	movq	%rax, previous_ob(%rip)
	movq	%r14, current_object(%rip)
	mov	12(%rcx), %edi
	addq	16(%rdx), %rdi
	call	eval_instruction
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE119:
	.size	call_direct, .-call_direct
.globl call___INIT
	.type	call___INIT, @function
call___INIT:
.LFB113:
	pushq	%r15
.LCFI199:
	pushq	%r14
.LCFI200:
	pushq	%r13
.LCFI201:
	pushq	%r12
.LCFI202:
	pushq	%rbp
.LCFI203:
	pushq	%rbx
.LCFI204:
	subq	$8, %rsp
.LCFI205:
	movq	%rdi, %r12
	movl	$0, tracedepth(%rip)
	andw	$-129, 2(%rdi)
	movq	40(%rdi), %rbx
	movzwl	142(%rbx), %eax
	testw	%ax, %ax
	je	.L3468
	movzwl	%ax, %ebp
	movslq	%ebp,%rax
	salq	$4, %rax
	addq	48(%rbx), %rax
	leaq	-16(%rax), %r15
	movq	-16(%rax), %rax
	cmpb	$35, (%rax)
	jne	.L3468
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L3425
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L3425:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	$4, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movq	%rbx, current_prog(%rip)
	leal	-1(%rbp), %edx
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	movw	$1, caller_type(%rip)
	movq	csp(%rip), %rax
	movl	$0, 40(%rax)
	movzwl	10(%rbx), %eax
	leal	-1(%rbp,%rax), %r8d
	movl	$0, variable_index_offset(%rip)
	movl	$0, function_index_offset(%rip)
	movq	current_prog(%rip), %r9
	movq	56(%r9), %rcx
	movslq	%r8d,%rax
	movzwl	(%rcx,%rax,2), %eax
	movl	%eax, %edx
	andl	$32767, %edx
	cmpw	$-1, %ax
	cmovle	%edx, %r8d
	movslq	%r8d,%rax
	movzwl	(%rcx,%rax,2), %ecx
	testb	$1, %cl
	je	.L3429
.L3469:
	movzwl	150(%r9), %eax
	leal	-1(%rax), %esi
	movl	$0, %edx
	testl	%esi, %esi
	jle	.L3433
	movq	104(%r9), %rdi
	movl	$0, %edx
.L3434:
	leal	1(%rsi,%rdx), %eax
	movl	%eax, %ecx
	sarl	%ecx
	movslq	%ecx,%rax
	salq	$4, %rax
	movzwl	8(%rax,%rdi), %eax
	cmpl	%r8d, %eax
	jg	.L3435
	movl	%ecx, %edx
	jmp	.L3437
.L3435:
	leal	-1(%rcx), %esi
.L3437:
	cmpl	%edx, %esi
	jg	.L3434
.L3433:
	movslq	%edx,%rdx
	salq	$4, %rdx
	movq	104(%r9), %rax
	movzwl	8(%rax,%rdx), %eax
	subl	%eax, %r8d
	addl	%eax, function_index_offset(%rip)
	movq	current_prog(%rip), %rcx
	movq	104(%rcx), %rax
	movzwl	10(%rdx,%rax), %eax
	addl	%eax, variable_index_offset(%rip)
	movq	104(%rcx), %rax
	movq	(%rdx,%rax), %r9
	movq	%r9, current_prog(%rip)
	movslq	%r8d,%rdx
	movq	56(%r9), %rax
	movzwl	(%rax,%rdx,2), %ecx
	testb	$1, %cl
	jne	.L3469
.L3429:
	movzwl	10(%r9), %eax
	movl	%r8d, %edx
	subl	%eax, %edx
	movslq	%edx,%rax
	salq	$4, %rax
	movq	%rax, %rsi
	addq	48(%r9), %rsi
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	testb	$16, %cl
	je	.L3438
	movzbl	10(%rsi), %r14d
	movzbl	11(%rsi), %r13d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r14d
	jg	.L3440
	leal	1(%rax), %ebx
	subl	%r14d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3442
	movl	$-1, %edi
.L3443:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3442
	jmp	.L3443
.L3440:
	movl	%r14d, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3444
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3444:
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3446
	movl	$-1, %edi
.L3447:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3447
.L3446:
	movl	$the_null_array, %ebp
.L3442:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3448
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3448:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$8, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r13d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3450
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3450:
	leal	-1(%r13), %esi
	cmpl	$-1, %esi
	je	.L3452
	movl	$-1, %edi
.L3453:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3453
.L3452:
	movq	sp(%rip), %rax
	leal	(%r13,%r14), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L3454
.L3438:
	movzbl	10(%rsi), %r14d
	movzbl	11(%rsi), %r13d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r14d, %ebp
	testl	%ebp, %ebp
	jle	.L3455
	movl	$0, %ebx
.L3459:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L3459
	movslq	%r13d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3460
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3460:
	leal	-1(%r13), %esi
	cmpl	$-1, %esi
	je	.L3462
	movl	$-1, %edi
.L3463:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3462
	jmp	.L3463
.L3455:
	movl	%r13d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3464
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3464:
	testl	%ebx, %ebx
	je	.L3462
	movl	$0, %esi
.L3467:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	addl	$1, %esi
	cmpl	%ebx, %esi
	jne	.L3467
.L3462:
	movq	sp(%rip), %rax
	leal	(%r13,%r14), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L3454:
	movq	current_object(%rip), %rax
	movq	%rax, previous_ob(%rip)
	movq	%r12, current_object(%rip)
	mov	12(%r15), %edi
	movq	current_prog(%rip), %rax
	addq	16(%rax), %rdi
	call	eval_instruction
	subq	$16, sp(%rip)
.L3468:
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE113:
	.size	call___INIT, .-call___INIT
	.section	.rodata.str1.1
.LC131:
	.string	"apply_low() cache [oprogp]"
.LC132:
	.string	"apply_low() cache [progp]"
	.section	.rodata.str1.8
	.align 8
.LC133:
	.string	"apply_low() cache [oprogp miss]"
	.text
.globl apply_low
	.type	apply_low, @function
apply_low:
.LFB111:
	pushq	%r15
.LCFI206:
	pushq	%r14
.LCFI207:
	pushq	%r13
.LCFI208:
	pushq	%r12
.LCFI209:
	pushq	%rbp
.LCFI210:
	pushq	%rbx
.LCFI211:
	subq	$56, %rsp
.LCFI212:
	movq	%rdi, %r14
	movq	%rsi, (%rsp)
	movl	%edx, %ebp
	movl	call_origin(%rip), %r12d
	testl	%r12d, %r12d
	movl	$1, %eax
	cmove	%eax, %r12d
	movl	$0, call_origin(%rip)
	movq	current_time(%rip), %rax
	movl	%eax, 32(%rsi)
	movzwl	2(%rsi), %eax
	testb	$16, %al
	je	.L3482
	testl	%ebp, %ebp
	je	.L3484
	movl	$0, %ebx
.L3486:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	je	.L3484
	jmp	.L3486
.L3482:
	andb	$127, %al
	movq	(%rsp), %rdx
	movw	%ax, 2(%rdx)
	movq	40(%rdx), %r13
	addl	$1, apply_low_call_others(%rip)
	movq	%r13, %rax
	sarq	$16, %rax
	movq	%r13, %rdx
	sarq	$2, %rdx
	xorl	%edx, %eax
	movq	%rdi, %rdx
	sarq	$2, %rdx
	xorl	%edx, %eax
	movq	%rdi, %rdx
	sarq	$16, %rdx
	xorl	%edx, %eax
	andl	cache_mask.11861(%rip), %eax
	cltq
	salq	$5, %rax
	leaq	cache(%rax), %r15
	movq	cache(%rax), %rbx
	cmpq	%r13, %rbx
	jne	.L3487
	cmpq	$0, 8(%r15)
	je	.L3489
	movq	16(%r15), %rax
	movq	(%rax), %rdi
	movq	%r14, %rsi
	call	strcmp
	testl	%eax, %eax
	jne	.L3487
	jmp	.L3491
.L3489:
	movq	16(%r15), %rdi
	movq	%r14, %rsi
	.p2align 4,,5
	call	strcmp
	testl	%eax, %eax
	.p2align 4,,2
	jne	.L3487
.L3491:
	addl	$1, apply_low_cache_hits(%rip)
	movq	8(%r15), %rdx
	testq	%rdx, %rdx
	je	.L3492
	movq	16(%r15), %r14
	movq	%r14, %rax
	subq	48(%rdx), %rax
	movq	%rax, %r13
	shrq	$4, %r13
	movq	(%r15), %rcx
	movzwl	24(%r15), %eax
	movzwl	10(%rdx), %edx
	addl	%edx, %eax
	addl	%r13d, %eax
	cltq
	movq	56(%rcx), %rdx
	movzwl	(%rdx,%rax,2), %ebx
	movl	$256, %edx
	cmpl	$1, %r12d
	je	.L3496
	movq	(%rsp), %rax
	cmpq	%rax, current_object(%rip)
	je	.L3499
	movl	$2048, %edx
	cmpl	$16, %r12d
	jne	.L3496
.L3499:
	movl	$1024, %edx
.L3496:
	movl	%ebx, %eax
	andl	$3840, %eax
	cmpl	%eax, %edx
	jg	.L3492
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L3501
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L3501:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	$4, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movq	8(%r15), %rax
	movq	%rax, current_prog(%rip)
	movw	%r12w, caller_type(%rip)
	movq	csp(%rip), %rax
	movl	%ebp, 40(%rax)
	movzwl	24(%r15), %eax
	movl	%eax, function_index_offset(%rip)
	movzwl	26(%r15), %eax
	movl	%eax, variable_index_offset(%rip)
	movq	csp(%rip), %rax
	movl	%r13d, 8(%rax)
	testb	$16, %bl
	je	.L3503
	movzbl	10(%r14), %r13d
	movzbl	11(%r14), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r13d
	jg	.L3505
	leal	1(%rax), %ebx
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3507
	movl	$-1, %edi
.L3508:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3507
	jmp	.L3508
.L3505:
	movl	%r13d, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3509
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3509:
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3511
	movl	$-1, %edi
.L3512:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3512
.L3511:
	movl	$the_null_array, %ebp
.L3507:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3513
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3513:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$8, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3515
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3515:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L3517
	movl	$-1, %edi
.L3518:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3518
.L3517:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L3519
.L3503:
	movzbl	10(%r14), %r13d
	movzbl	11(%r14), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r13d, %ebp
	testl	%ebp, %ebp
	jle	.L3520
	movl	$0, %ebx
.L3524:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L3524
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3525
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3525:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L3527
	movl	$-1, %edi
.L3528:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3527
	jmp	.L3528
.L3520:
	movl	%r12d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3529
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3529:
	testl	%ebx, %ebx
	je	.L3527
	movl	$0, %esi
.L3532:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	addl	$1, %esi
	cmpl	%ebx, %esi
	jne	.L3532
.L3527:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L3519:
	movq	current_object(%rip), %rax
	movq	%rax, previous_ob(%rip)
	movq	(%rsp), %rax
	movq	%rax, current_object(%rip)
	mov	12(%r14), %edi
	movq	current_prog(%rip), %rax
	addq	16(%rax), %rdi
	call	eval_instruction
	movl	$1, %eax
	jmp	.L3533
.L3487:
	testq	%rbx, %rbx
	je	.L3534
	movq	%rbx, %rdi
	.p2align 4,,5
	call	free_prog
.L3534:
	movq	8(%r15), %rdi
	testq	%rdi, %rdi
	je	.L3536
	call	free_prog
	.p2align 4,,4
	jmp	.L3538
.L3536:
	movq	16(%r15), %rdi
	testq	%rdi, %rdi
	.p2align 4,,4
	je	.L3539
	.p2align 4,,7
	call	free_string
.L3538:
	cmpq	$0, 16(%r15)
	.p2align 4,,2
	jne	.L3541
.L3539:
	addl	$1, apply_low_slots_used(%rip)
	jmp	.L3542
.L3541:
	addl	$1, apply_low_collisions(%rip)
.L3542:
	movq	%r14, %rdi
	call	findstring
	movq	%rax, %rbx
	testq	%rax, %rax
	je	.L3543
	leaq	48(%rsp), %rcx
	leaq	52(%rsp), %rdx
	movq	(%rsp), %rax
	movq	40(%rax), %rdi
	leaq	40(%rsp), %r9
	leaq	44(%rsp), %r8
	movq	%rbx, %rsi
	call	ffbn_recurse2
	movq	%rax, 24(%rsp)
	testq	%rax, %rax
	je	.L3543
	movq	48(%rax), %rdx
	movq	%rdx, 8(%rsp)
	movl	52(%rsp), %eax
	movl	%eax, 20(%rsp)
	movq	(%rsp), %rdx
	movq	40(%rdx), %rax
	movslq	48(%rsp),%rdx
	movq	56(%rax), %rax
	movzwl	(%rax,%rdx,2), %edx
	movl	%edx, 36(%rsp)
	movl	$256, %edx
	cmpl	$1, %r12d
	je	.L3548
	movq	(%rsp), %rax
	cmpq	%rax, current_object(%rip)
	je	.L3551
	movl	$2048, %edx
	cmpl	$16, %r12d
	jne	.L3548
.L3551:
	movl	$1024, %edx
.L3548:
	movl	36(%rsp), %eax
	andl	$3840, %eax
	cmpl	%eax, %edx
	jg	.L3543
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L3553
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L3553:
	movslq	20(%rsp),%rax
	salq	$4, %rax
	movq	8(%rsp), %r14
	addq	%rax, %r14
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	$4, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movq	24(%rsp), %rdx
	movq	%rdx, current_prog(%rip)
	movw	%r12w, caller_type(%rip)
	movq	(%rsp), %rdx
	movq	40(%rdx), %rax
	movq	%rax, (%r15)
	movq	%r14, 16(%r15)
	movl	52(%rsp), %edx
	movq	csp(%rip), %rax
	movl	%edx, 8(%rax)
	movq	csp(%rip), %rax
	movl	%ebp, 40(%rax)
	movl	40(%rsp), %eax
	movl	%eax, variable_index_offset(%rip)
	movw	%ax, 26(%r15)
	movl	44(%rsp), %eax
	movl	%eax, function_index_offset(%rip)
	movw	%ax, 24(%r15)
	testb	$16, 36(%rsp)
	je	.L3555
	movzbl	10(%r14), %r13d
	movzbl	11(%r14), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	cmpl	%eax, %r13d
	jg	.L3557
	leal	1(%rax), %ebx
	subl	%r13d, %ebx
	movl	%ebx, %edi
	call	allocate_empty_array
	movq	%rax, %rbp
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3559
	movl	$-1, %edi
.L3560:
	movq	sp(%rip), %rcx
	movslq	%esi,%rax
	salq	$4, %rax
	movq	(%rcx), %rdx
	movq	%rdx, 8(%rax,%rbp)
	movq	8(%rcx), %rdx
	movq	%rdx, 16(%rax,%rbp)
	subq	$16, %rcx
	movq	%rcx, sp(%rip)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3559
	jmp	.L3560
.L3557:
	movl	%r13d, %edx
	subl	%eax, %edx
	movl	%edx, %eax
	leal	-1(%rax), %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3561
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3561:
	leal	-1(%rbx), %esi
	cmpl	$-1, %esi
	je	.L3563
	movl	$-1, %edi
.L3564:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3564
.L3563:
	movl	$the_null_array, %ebp
.L3559:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3565
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3565:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rax
	movq	%rax, sp(%rip)
	movw	$8, 16(%rdx)
	movq	sp(%rip), %rax
	movq	%rbp, 8(%rax)
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3567
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3567:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L3569
	movl	$-1, %edi
.L3570:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	jne	.L3570
.L3569:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
	jmp	.L3571
.L3555:
	movzbl	10(%r14), %r13d
	movzbl	11(%r14), %r12d
	movq	csp(%rip), %rax
	movl	40(%rax), %eax
	movl	%eax, %ebp
	subl	%r13d, %ebp
	testl	%ebp, %ebp
	jle	.L3572
	movl	$0, %ebx
.L3576:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L3576
	movslq	%r12d,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3577
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3577:
	leal	-1(%r12), %esi
	cmpl	$-1, %esi
	je	.L3579
	movl	$-1, %edi
.L3580:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	subl	$1, %esi
	cmpl	%edi, %esi
	je	.L3579
	jmp	.L3580
.L3572:
	movl	%r12d, %ebx
	subl	%ebp, %ebx
	movslq	%ebx,%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3581
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3581:
	testl	%ebx, %ebx
	je	.L3579
	movl	$0, %esi
.L3584:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	const0u(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rcx)
	addl	$1, %esi
	cmpl	%ebx, %esi
	jne	.L3584
.L3579:
	movq	sp(%rip), %rax
	leal	(%r12,%r13), %edx
	movq	csp(%rip), %rcx
	movl	%edx, 40(%rcx)
	movslq	%edx,%rdx
	salq	$4, %rdx
	subq	%rdx, %rax
	addq	$16, %rax
	movq	%rax, fp(%rip)
.L3571:
	movq	current_prog(%rip), %rax
	movq	%rax, 8(%r15)
	movl	$.LC131, %esi
	movq	(%r15), %rdi
	call	reference_prog
	movq	8(%r15), %rdi
	movl	$.LC132, %esi
	call	reference_prog
	movq	current_object(%rip), %rax
	movq	%rax, previous_ob(%rip)
	movq	(%rsp), %rax
	movq	%rax, current_object(%rip)
	mov	12(%r14), %edi
	movq	current_prog(%rip), %rax
	addq	16(%rax), %rdi
	call	eval_instruction
	movl	$1, %eax
	jmp	.L3533
.L3543:
	movq	%r13, (%r15)
	movl	$.LC133, %esi
	movq	%r13, %rdi
	call	reference_prog
	testq	%rbx, %rbx
	je	.L3585
	movq	%rbx, %rdi
	call	ref_string
	movq	%rbx, 16(%r15)
	.p2align 4,,2
	jmp	.L3587
.L3585:
	movq	%r14, %rdi
	call	make_shared_string
	movq	%rax, 16(%r15)
.L3587:
	movq	$0, 8(%r15)
.L3492:
	testl	%ebp, %ebp
	je	.L3484
	movl	$0, %ebx
.L3589:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	%ebp, %ebx
	jne	.L3589
.L3484:
	movl	$0, %eax
.L3533:
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE111:
	.size	apply_low, .-apply_low
.globl call_all_other
	.type	call_all_other, @function
call_all_other:
.LFB115:
	pushq	%r15
.LCFI213:
	pushq	%r14
.LCFI214:
	pushq	%r13
.LCFI215:
	pushq	%r12
.LCFI216:
	pushq	%rbp
.LCFI217:
	pushq	%rbx
.LCFI218:
	subq	$40, %rsp
.LCFI219:
	movq	%rdi, %r12
	movq	%rsi, 24(%rsp)
	movl	%edx, 20(%rsp)
	movq	sp(%rip), %r15
	leaq	16(%r15), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3608
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3608:
	movq	sp(%rip), %rax
	addq	$16, %rax
	movq	%rax, sp(%rip)
	movw	$8, (%rax)
	movq	sp(%rip), %rbx
	movzwl	2(%r12), %ebp
	movl	%ebp, %edi
	call	allocate_array
	movq	%rax, 32(%rsp)
	movq	%rax, 8(%rbx)
	movslq	20(%rsp),%rax
	salq	$4, %rax
	addq	sp(%rip), %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3610
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3610:
	leal	-1(%rbp), %r13d
	cmpl	$-1, %r13d
	je	.L3612
	movq	32(%rsp), %r14
	addq	$8, %r14
	addq	$24, %r12
	movl	20(%rsp), %eax
	subl	$1, %eax
	cltq
	salq	$4, %rax
	subq	%rax, %r15
	movq	%r15, 8(%rsp)
.L3614:
	movzwl	-16(%r12), %eax
	cmpw	$16, %ax
	jne	.L3615
	movq	-8(%r12), %r15
	jmp	.L3617
.L3615:
	cmpw	$4, %ax
	jne	.L3618
	movq	-8(%r12), %rdi
	.p2align 4,,5
	call	find_object
	movq	%rax, %r15
	testq	%rax, %rax
	je	.L3618
	movq	%rax, %rdi
	call	object_visible
	testl	%eax, %eax
	je	.L3618
.L3617:
	testb	$16, 2(%r15)
	.p2align 4,,2
	jne	.L3618
	movq	8(%rsp), %rbx
	addq	$2, %rbx
	movl	20(%rsp), %ebp
	jmp	.L3622
.L3623:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3624
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3624:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	movq	%rbx, %rcx
	cmpw	$16, -2(%rbx)
	jne	.L3626
	movq	6(%rbx), %rax
	testq	%rax, %rax
	je	.L3628
	testb	$16, 2(%rax)
	je	.L3626
.L3628:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3630
.L3626:
	movq	-2(%rbx), %rax
	movq	%rax, (%rdx)
	movq	6(%rbx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L3631
	cmpw	$8192, %ax
	je	.L3631
	andb	$223, %ah
	movw	%ax, (%rdx)
.L3631:
	movzwl	-2(%rcx), %eax
	cmpw	$4, %ax
	jne	.L3634
	testb	$1, (%rcx)
	je	.L3630
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L3637
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L3637:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3630
.L3634:
	testl	$17272, %eax
	je	.L3630
	movq	6(%rbx), %rax
	addw	$1, (%rax)
.L3630:
	addq	$16, %rbx
.L3622:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L3623
	movl	$4, call_origin(%rip)
	movl	20(%rsp), %edx
	movq	%r15, %rsi
	movq	24(%rsp), %rdi
	call	apply_low
	testl	%eax, %eax
	je	.L3618
	movq	sp(%rip), %rax
	movq	(%rax), %rdx
	movq	%rdx, (%r14)
	movq	8(%rax), %rdx
	movq	%rdx, 8(%r14)
	subq	$16, %rax
	movq	%rax, sp(%rip)
.L3618:
	subl	$1, %r13d
	addq	$16, %r12
	cmpl	$-1, %r13d
	je	.L3612
	addq	$16, %r14
	jmp	.L3614
.L3612:
	subq	$16, sp(%rip)
	cmpl	$0, 20(%rsp)
	je	.L3643
	movl	$0, %ebx
.L3645:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	20(%rsp), %ebx
	jne	.L3645
.L3643:
	movq	32(%rsp), %rax
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.LFE115:
	.size	call_all_other, .-call_all_other
.globl apply
	.type	apply, @function
apply:
.LFB112:
	subq	$8, %rsp
.LCFI220:
	movl	$0, tracedepth(%rip)
	movl	%ecx, call_origin(%rip)
	call	apply_low
	movl	$0, %edx
	testl	%eax, %eax
	je	.L3651
	movzwl	apply_ret_value(%rip), %eax
	cmpw	$4, %ax
	jne	.L3652
	movq	apply_ret_value+8(%rip), %rdi
	testb	$1, apply_ret_value+2(%rip)
	je	.L3654
	leaq	-4(%rdi), %rcx
	movzwl	-4(%rdi), %edx
	movzwl	-2(%rdi), %eax
	testw	%ax, %ax
	je	.L3656
	subl	$1, %eax
	movw	%ax, -2(%rdi)
	testw	%ax, %ax
	jne	.L3656
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	testb	$2, apply_ret_value+2(%rip)
	je	.L3659
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$16, overhead_bytes(%rip)
	call	deallocate_string
	jmp	.L3654
.L3659:
	subl	$1, num_distinct_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, bytes_distinct_strings(%rip)
	subl	$4, overhead_bytes(%rip)
	movq	%rcx, %rdi
	call	free
	jmp	.L3654
.L3656:
	subl	$1, allocd_strings(%rip)
	movl	%edx, %eax
	notl	%eax
	addl	%eax, allocd_bytes(%rip)
	jmp	.L3654
.L3652:
	movswl	%ax,%edx
	testl	$17272, %edx
	je	.L3661
	testb	$32, %dh
	jne	.L3661
	movq	apply_ret_value+8(%rip), %rax
	movzwl	(%rax), %edx
	subl	$1, %edx
	movw	%dx, (%rax)
	testw	%dx, %dx
	jne	.L3654
	movzwl	apply_ret_value(%rip), %eax
	cmpw	$64, %ax
	je	.L3668
	cmpw	$64, %ax
	jg	.L3672
	cmpw	$16, %ax
	je	.L3666
	cmpw	$32, %ax
	.p2align 4,,5
	je	.L3667
	cmpw	$8, %ax
	.p2align 4,,5
	jne	.L3654
	.p2align 4,,7
	jmp	.L3665
.L3672:
	cmpw	$512, %ax
	.p2align 4,,7
	je	.L3670
	cmpw	$16384, %ax
	.p2align 4,,7
	je	.L3671
	cmpw	$256, %ax
	.p2align 4,,5
	jne	.L3654
	.p2align 4,,7
	jmp	.L3669
.L3666:
	movl	$.LC52, %esi
	movq	apply_ret_value+8(%rip), %rdi
	call	dealloc_object
	jmp	.L3654
.L3670:
	movq	apply_ret_value+8(%rip), %rdi
	call	dealloc_class
	.p2align 4,,3
	jmp	.L3654
.L3665:
	movq	apply_ret_value+8(%rip), %rdi
	cmpq	$the_null_array, %rdi
	je	.L3654
	call	dealloc_array
	jmp	.L3654
.L3669:
	movq	apply_ret_value+8(%rip), %rdi
	cmpq	$null_buf, %rdi
	je	.L3654
	call	free
	jmp	.L3654
.L3667:
	movq	apply_ret_value+8(%rip), %rdi
	call	dealloc_mapping
	.p2align 4,,3
	jmp	.L3654
.L3668:
	movq	apply_ret_value+8(%rip), %rdi
	call	dealloc_funp
	.p2align 4,,3
	jmp	.L3654
.L3671:
	movq	apply_ret_value+8(%rip), %rdi
	cmpq	$0, 32(%rdi)
	jne	.L3654
	call	kill_ref
	jmp	.L3654
.L3661:
	cmpw	$4096, %ax
	.p2align 4,,6
	jne	.L3654
	.p2align 4,,9
	call	*apply_ret_value+8(%rip)
.L3654:
	movq	sp(%rip), %rax
	movq	(%rax), %rdx
	movq	%rdx, apply_ret_value(%rip)
	movq	8(%rax), %rdx
	movq	%rdx, apply_ret_value+8(%rip)
	subq	$16, %rax
	movq	%rax, sp(%rip)
	movl	$apply_ret_value, %edx
.L3651:
	movq	%rdx, %rax
	addq	$8, %rsp
	ret
.LFE112:
	.size	apply, .-apply
.globl safe_apply
	.type	safe_apply, @function
safe_apply:
.LFB114:
	pushq	%rbx
.LCFI221:
	subq	$272, %rsp
.LCFI222:
	movq	%rdi, 24(%rsp)
	movq	%rsi, 16(%rsp)
	movl	%edx, 12(%rsp)
	movl	%ecx, 8(%rsp)
	leaq	32(%rsp), %rdi
	call	save_context
	movl	$0, %ebx
	testl	%eax, %eax
	je	.L3681
	leaq	32(%rsp), %rdi
	call	_setjmp
	testl	%eax, %eax
	jne	.L3682
	movq	16(%rsp), %rax
	testb	$16, 2(%rax)
	jne	.L3684
	movl	8(%rsp), %ecx
	movl	12(%rsp), %edx
	movq	%rax, %rsi
	movq	24(%rsp), %rdi
	call	apply
	movq	%rax, %rbx
	jmp	.L3686
.L3682:
	leaq	32(%rsp), %rdi
	call	restore_context
	cmpl	$0, 12(%rsp)
	je	.L3684
	movl	$0, %ebx
.L3688:
	movq	sp(%rip), %rdi
	leaq	-16(%rdi), %rax
	movq	%rax, sp(%rip)
	call	int_free_svalue
	addl	$1, %ebx
	cmpl	12(%rsp), %ebx
	jne	.L3688
.L3684:
	movl	$0, %ebx
.L3686:
	leaq	32(%rsp), %rdi
	call	pop_context
.L3681:
	movq	%rbx, %rax
	addq	$272, %rsp
	popq	%rbx
	ret
.LFE114:
	.size	safe_apply, .-safe_apply
	.section	.rodata.str1.8
	.align 8
.LC134:
	.string	"Object destructed during efun callback.\n"
	.text
.globl call_efun_callback
	.type	call_efun_callback, @function
call_efun_callback:
.LFB63:
	pushq	%r13
.LCFI223:
	pushq	%r12
.LCFI224:
	pushq	%rbp
.LCFI225:
	pushq	%rbx
.LCFI226:
	subq	$8, %rsp
.LCFI227:
	movq	%rdi, %r12
	movl	%esi, %r13d
	movl	16(%rdi), %ebp
	testl	%ebp, %ebp
	je	.L3692
	movq	24(%rdi), %rbx
	addq	$16, %rbx
	jmp	.L3694
.L3695:
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3696
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3696:
	movq	sp(%rip), %rdx
	addq	$16, %rdx
	movq	%rdx, sp(%rip)
	movq	%rbx, %rcx
	cmpw	$16, -16(%rbx)
	jne	.L3698
	movq	-8(%rbx), %rax
	testq	%rax, %rax
	je	.L3700
	testb	$16, 2(%rax)
	je	.L3698
.L3700:
	movq	const0u(%rip), %rax
	movq	%rax, (%rdx)
	movq	const0u+8(%rip), %rax
	movq	%rax, 8(%rdx)
	jmp	.L3702
.L3698:
	movq	-16(%rbx), %rax
	movq	%rax, (%rdx)
	movq	-8(%rbx), %rax
	movq	%rax, 8(%rdx)
	movzwl	(%rdx), %eax
	testb	$32, %ah
	je	.L3703
	cmpw	$8192, %ax
	je	.L3703
	andb	$223, %ah
	movw	%ax, (%rdx)
.L3703:
	movzwl	-16(%rcx), %eax
	cmpw	$4, %ax
	jne	.L3706
	testb	$1, -14(%rcx)
	je	.L3702
	movq	8(%rdx), %rcx
	subq	$4, %rcx
	movzwl	2(%rcx), %eax
	testw	%ax, %ax
	je	.L3709
	addl	$1, %eax
	movw	%ax, 2(%rcx)
.L3709:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	8(%rdx), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3702
.L3706:
	testl	$17272, %eax
	je	.L3702
	movq	-8(%rbx), %rax
	addw	$1, (%rax)
.L3702:
	addq	$16, %rbx
.L3694:
	subl	$1, %ebp
	cmpl	$-1, %ebp
	jne	.L3695
.L3692:
	movq	(%r12), %rax
	testq	%rax, %rax
	je	.L3712
	testb	$16, 2(%rax)
	je	.L3714
	movl	$.LC134, %edi
	movl	$0, %eax
	call	error
.L3714:
	movl	%r13d, %edx
	addl	16(%r12), %edx
	movq	8(%r12), %rdi
	movl	$32, %ecx
	movq	(%r12), %rsi
	call	apply
	jmp	.L3716
.L3712:
	movl	%r13d, %esi
	addl	16(%r12), %esi
	movq	8(%r12), %rdi
	call	call_function_pointer
.L3716:
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	ret
.LFE63:
	.size	call_efun_callback, .-call_efun_callback
	.section	.rodata.str1.8
	.align 8
.LC135:
	.string	"Can't catch too deep recursion error.\n"
	.align 8
.LC136:
	.string	"Can't catch eval cost too big error.\n"
	.text
	.type	do_catch, @function
do_catch:
.LFB104:
	pushq	%rbx
.LCFI228:
	subq	$256, %rsp
.LCFI229:
	movq	%rdi, 8(%rsp)
	movl	%esi, %ebx
	leaq	16(%rsp), %rdi
	call	save_context
	testl	%eax, %eax
	jne	.L3719
	movl	$.LC135, %edi
	call	error
.L3719:
	cmpq	$control_stack+11920, csp(%rip)
	jne	.L3721
	movl	$1, too_deep_error(%rip)
	movl	$.LC14, %edi
	movl	$0, %eax
	call	error
.L3721:
	movq	csp(%rip), %rdx
	addq	$80, %rdx
	movq	%rdx, csp(%rip)
	movzwl	caller_type(%rip), %eax
	movw	%ax, 72(%rdx)
	movq	current_object(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 16(%rax)
	movq	csp(%rip), %rax
	movw	$2, (%rax)
	movq	previous_ob(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 24(%rax)
	movq	fp(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 56(%rax)
	movq	current_prog(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 32(%rax)
	movq	pc(%rip), %rdx
	movq	csp(%rip), %rax
	movq	%rdx, 48(%rax)
	movl	function_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 64(%rax)
	movl	variable_index_offset(%rip), %edx
	movq	csp(%rip), %rax
	movl	%edx, 68(%rax)
	movzwl	%bx, %eax
	movq	current_prog(%rip), %rdx
	addq	16(%rdx), %rax
	movq	csp(%rip), %rdx
	movq	%rax, 48(%rdx)
	movq	csp(%rip), %rdx
	movl	-40(%rdx), %eax
	movl	%eax, 40(%rdx)
	leaq	16(%rsp), %rdi
	call	_setjmp
	testl	%eax, %eax
	je	.L3723
	leaq	16(%rsp), %rdi
	call	restore_context
	movq	sp(%rip), %rax
	addq	$16, %rax
	cmpq	end_of_stack(%rip), %rax
	jb	.L3725
	movl	$1, too_deep_error(%rip)
	movl	$.LC15, %edi
	movl	$0, %eax
	call	error
.L3725:
	movq	sp(%rip), %rdx
	leaq	16(%rdx), %rcx
	movq	%rcx, sp(%rip)
	movq	catch_value(%rip), %rax
	movq	%rax, 16(%rdx)
	movq	catch_value+8(%rip), %rax
	movq	%rax, 8(%rcx)
	movq	const1(%rip), %rax
	movq	%rax, catch_value(%rip)
	movq	const1+8(%rip), %rax
	movq	%rax, catch_value+8(%rip)
	cmpl	$0, max_eval_error(%rip)
	je	.L3727
	leaq	16(%rsp), %rdi
	call	pop_context
	movl	$.LC136, %edi
	movl	$0, %eax
	call	error
.L3727:
	cmpl	$0, too_deep_error(%rip)
	je	.L3729
	leaq	16(%rsp), %rdi
	call	pop_context
	movl	$.LC135, %edi
	movl	$0, %eax
	call	error
	jmp	.L3729
.L3723:
	movl	$catch_value, %edi
	call	int_free_svalue
	movzwl	const1(%rip), %edx
	cmpw	$16, %dx
	jne	.L3731
	movq	const1+8(%rip), %rax
	testq	%rax, %rax
	je	.L3733
	testb	$16, 2(%rax)
	je	.L3731
.L3733:
	movq	const0u(%rip), %rax
	movq	%rax, catch_value(%rip)
	movq	const0u+8(%rip), %rax
	movq	%rax, catch_value+8(%rip)
	jmp	.L3735
.L3731:
	movq	const1(%rip), %rax
	movq	%rax, catch_value(%rip)
	movq	const1+8(%rip), %rax
	movq	%rax, catch_value+8(%rip)
	movzwl	catch_value(%rip), %eax
	testb	$32, %ah
	je	.L3736
	cmpw	$8192, %ax
	je	.L3736
	andb	$223, %ah
	movw	%ax, catch_value(%rip)
.L3736:
	cmpw	$4, %dx
	jne	.L3739
	testb	$1, const1+2(%rip)
	je	.L3735
	movq	catch_value+8(%rip), %rdx
	subq	$4, %rdx
	movzwl	2(%rdx), %eax
	testw	%ax, %ax
	je	.L3742
	addl	$1, %eax
	movw	%ax, 2(%rdx)
.L3742:
	addl	$1, allocd_strings(%rip)
	movl	allocd_bytes(%rip), %eax
	addl	$1, %eax
	movq	catch_value+8(%rip), %rdx
	movzwl	-4(%rdx), %edx
	addl	%edx, %eax
	movl	%eax, allocd_bytes(%rip)
	jmp	.L3735
.L3739:
	testl	$17272, %edx
	je	.L3735
	movq	const1+8(%rip), %rax
	addw	$1, (%rax)
.L3735:
	movq	8(%rsp), %rdi
	call	eval_instruction
.L3729:
	leaq	16(%rsp), %rdi
	call	pop_context
	addq	$256, %rsp
	popq	%rbx
	ret
.LFE104:
	.size	do_catch, .-do_catch
.globl efun_table
	.data
	.align 32
	.type	efun_table, @object
	.size	efun_table, 1968
efun_table:
	.quad	f__to_int
	.quad	f__to_float
	.quad	f_this_player
	.quad	f_previous_object
	.quad	f_call_stack
	.quad	f_sizeof
	.quad	f_strwidth
	.quad	f_destruct
	.quad	f_file_name
	.quad	f_capitalize
	.quad	f_random
	.quad	f_all_inventory
	.quad	f_deep_inventory
	.quad	f_first_inventory
	.quad	f_next_inventory
	.quad	f_move_object
	.quad	f_set_this_player
	.quad	f_lower_case
	.quad	f_save_variable
	.quad	f_restore_variable
	.quad	f_write
	.quad	f_shout
	.quad	f_receive
	.quad	f_find_call_out
	.quad	f_values
	.quad	f_keys
	.quad	f_clonep
	.quad	f_intp
	.quad	f_undefinedp
	.quad	f_floatp
	.quad	f_stringp
	.quad	f_virtualp
	.quad	f_functionp
	.quad	f_pointerp
	.quad	f_objectp
	.quad	f_classp
	.quad	f_typeof
	.quad	f_bufferp
	.quad	f_allocate_buffer
	.quad	f_replace_program
	.quad	f_crc32
	.quad	f_file_size
	.quad	f_mkdir
	.quad	f_rm
	.quad	f_rmdir
	.quad	f_localtime
	.quad	f_query_idle
	.quad	f_query_snoop
	.quad	f_query_snooping
	.quad	f_set_heart_beat
	.quad	f_query_heart_beat
	.quad	f_set_hide
	.quad	f_throw
	.quad	f_deep_inherit_list
	.quad	f_shallow_inherit_list
	.quad	f_mapp
	.quad	f_interactive
	.quad	f_has_mxp
	.quad	f_in_edit
	.quad	f_in_input
	.quad	f_userp
	.quad	f_get_config
	.quad	f_children
	.quad	f_reload_object
	.quad	f_error
	.quad	f_mud_status
	.quad	f_set_eval_limit
	.quad	f_set_encoding
	.quad	f_str_to_arr
	.quad	f_arr_to_str
	.quad	f_export_uid
	.quad	f_geteuid
	.quad	f_getuid
	.quad	f_seteuid
	.quad	f_compress
	.quad	f_uncompress
	.quad	f_copy
	.quad	f_pluralize
	.quad	f_file_length
	.quad	f_upper_case
	.quad	f_fetch_variable
	.quad	f_remove_interactive
	.quad	f_debug_message
	.quad	f_function_owner
	.quad	f_compressedp
	.quad	f_base_name
	.quad	f_db_close
	.quad	f_db_commit
	.quad	f_db_rollback
	.quad	f_shuffle
	.quad	f_element_of
	.quad	f_abs
	.quad	f_add_a
	.quad	f_vowel
	.quad	f_num_classes
	.quad	f_assemble_class
	.quad	f_disassemble_class
	.quad	f_socket_close
	.quad	f_socket_error
	.quad	f_refs
	.quad	f_cos
	.quad	f_sin
	.quad	f_tan
	.quad	f_asin
	.quad	f_acos
	.quad	f_atan
	.quad	f_sqrt
	.quad	f_log
	.quad	f_log10
	.quad	f_exp
	.quad	f_floor
	.quad	f_ceil
	.quad	f__call_other
	.quad	f__evaluate
	.quad	f__this_object
	.quad	f__new
	.quad	f_bind
	.quad	f_explode
	.quad	f_implode
	.quad	f_call_out
	.quad	f_member_array
	.quad	f_input_to
	.quad	f_environment
	.quad	f_say
	.quad	f_tell_room
	.quad	f_present
	.quad	f_replace_string
	.quad	f_restore_object
	.quad	f_save_object
	.quad	f_users
	.quad	f_get_dir
	.quad	f_strsrch
	.quad	f_tell_object
	.quad	f_message
	.quad	f_find_object
	.quad	f_allocate_mapping
	.quad	f_map_delete
	.quad	f_match_path
	.quad	f_inherits
	.quad	f_regexp
	.quad	f_reg_assoc
	.quad	f_allocate
	.quad	f_call_out_info
	.quad	f_read_buffer
	.quad	f_write_buffer
	.quad	f_write_file
	.quad	f_rename
	.quad	f_write_bytes
	.quad	f_read_bytes
	.quad	f_read_file
	.quad	f_cp
	.quad	f_link
	.quad	f_clear_bit
	.quad	f_test_bit
	.quad	f_set_bit
	.quad	f_next_bit
	.quad	f_crypt
	.quad	f_oldcrypt
	.quad	f_ctime
	.quad	f_exec
	.quad	f_function_exists
	.quad	f_objects
	.quad	f_query_host_name
	.quad	f_query_ip_name
	.quad	f_query_ip_number
	.quad	f_snoop
	.quad	f_remove_call_out
	.quad	f_set_reset
	.quad	f_sort_array
	.quad	f_time
	.quad	f_unique_array
	.quad	f_unique_mapping
	.quad	f_printf
	.quad	f_sprintf
	.quad	f_stat
	.quad	f_master
	.quad	f_memory_info
	.quad	f_get_char
	.quad	f_uptime
	.quad	f_strcmp
	.quad	f_rusage
	.quad	f_flush_messages
	.quad	f_ed
	.quad	f_cache_stats
	.quad	f_filter
	.quad	f_map
	.quad	f_malloc_status
	.quad	f_dumpallobj
	.quad	f_dump_file_descriptors
	.quad	f_query_load_average
	.quad	f_origin
	.quad	f_reclaim_objects
	.quad	f_resolve
	.quad	f_to_utf8
	.quad	f_utf8_to
	.quad	f_act_mxp
	.quad	f_request_term_type
	.quad	f_start_request_term_type
	.quad	f_request_term_size
	.quad	f_shutdown
	.quad	f_external_start
	.quad	f_compress_file
	.quad	f_uncompress_file
	.quad	f_functions
	.quad	f_variables
	.quad	f_heart_beats
	.quad	f_terminal_colour
	.quad	f_replaceable
	.quad	f_program_info
	.quad	f_store_variable
	.quad	f_query_ip_port
	.quad	f_zonetime
	.quad	f_is_daylight_savings_time
	.quad	f_repeat_string
	.quad	f_memory_summary
	.quad	f_query_replaced_program
	.quad	f_network_stats
	.quad	f_real_time
	.quad	f_event
	.quad	f_query_num
	.quad	f_get_garbage
	.quad	f_db_connect
	.quad	f_db_exec
	.quad	f_db_fetch
	.quad	f_db_status
	.quad	f_query_multiple_short
	.quad	f_reference_allowed
	.quad	f_max
	.quad	f_min
	.quad	f_roll_MdN
	.quad	f_fetch_class_member
	.quad	f_store_class_member
	.quad	f_replace
	.quad	f_socket_create
	.quad	f_socket_bind
	.quad	f_socket_listen
	.quad	f_socket_accept
	.quad	f_socket_connect
	.quad	f_socket_write
	.quad	f_socket_release
	.quad	f_socket_acquire
	.quad	f_socket_address
	.quad	f_socket_status
	.quad	f_debug_info
	.quad	f_dump_prog
	.quad	f_pow
.globl end_of_stack
	.align 8
	.type	end_of_stack, @object
	.size	end_of_stack, 8
end_of_stack:
	.quad	start_of_stack+48000000
.globl catch_value
	.align 16
	.type	catch_value, @object
	.size	catch_value, 16
catch_value:
	.value	2
	.zero	14
.globl apply_ret_value
	.align 16
	.type	apply_ret_value, @object
	.size	apply_ret_value, 16
apply_ret_value:
	.value	2
	.zero	14
.globl too_deep_error
	.bss
	.align 4
	.type	too_deep_error, @object
	.size	too_deep_error, 4
too_deep_error:
	.zero	4
.globl max_eval_error
	.align 4
	.type	max_eval_error, @object
	.size	max_eval_error, 4
max_eval_error:
	.zero	4
.globl global_ref_list
	.align 8
	.type	global_ref_list, @object
	.size	global_ref_list, 8
global_ref_list:
	.zero	8
.globl global_lvalue_byte
	.data
	.align 16
	.type	global_lvalue_byte, @object
	.size	global_lvalue_byte, 16
global_lvalue_byte:
	.value	1024
	.zero	14
.globl fake_prog
	.section	.rodata.str1.1
.LC137:
	.string	"<driver>"
	.data
	.align 32
	.type	fake_prog, @object
	.size	fake_prog, 152
fake_prog:
	.quad	.LC137
	.zero	144
.globl fake_program
	.type	fake_program, @object
	.size	fake_program, 1
fake_program:
	.byte	46
.globl apply_low_call_others
	.bss
	.align 4
	.type	apply_low_call_others, @object
	.size	apply_low_call_others, 4
apply_low_call_others:
	.zero	4
.globl apply_low_cache_hits
	.align 4
	.type	apply_low_cache_hits, @object
	.size	apply_low_cache_hits, 4
apply_low_cache_hits:
	.zero	4
.globl apply_low_slots_used
	.align 4
	.type	apply_low_slots_used, @object
	.size	apply_low_slots_used, 4
apply_low_slots_used:
	.zero	4
.globl apply_low_collisions
	.align 4
	.type	apply_low_collisions, @object
	.size	apply_low_collisions, 4
apply_low_collisions:
	.zero	4
	.local	buff.12957
	.comm	buff.12957,10,1
	.local	buf.12319
	.comm	buf.12319,256,32
	.section	.rodata
	.align 4
	.type	cache_mask.11861, @object
	.size	cache_mask.11861, 4
cache_mask.11861:
	.long	16383
	.align 8
	.type	instrs2.10554, @object
	.size	instrs2.10554, 8
instrs2.10554:
	.quad	instrs+7424
	.align 8
	.type	ooefun_table.10553, @object
	.size	ooefun_table.10553, 8
ooefun_table.10553:
	.quad	efun_table-960
	.align 8
	.type	oefun_table.10552, @object
	.size	oefun_table.10552, 8
oefun_table.10552:
	.quad	efun_table+896
	.data
	.align 16
	.type	global_lvalue_range_sv, @object
	.size	global_lvalue_range_sv, 16
global_lvalue_range_sv:
	.value	2048
	.zero	14
	.section	.rodata.str1.1
.LC138:
	.string	"int"
.LC139:
	.string	"string"
.LC140:
	.string	"array"
.LC141:
	.string	"mapping"
.LC142:
	.string	"float"
.LC143:
	.string	"buffer"
.LC144:
	.string	"class"
	.section	.rodata
	.align 32
	.type	type_names, @object
	.size	type_names, 72
type_names:
	.quad	.LC138
	.quad	.LC139
	.quad	.LC140
	.quad	.LC46
	.quad	.LC141
	.quad	.LC42
	.quad	.LC142
	.quad	.LC143
	.quad	.LC144
	.local	tracedepth
	.comm	tracedepth,4,4
	.local	start_of_stack
	.comm	start_of_stack,48000160,32
	.local	global_lvalue_range
	.comm	global_lvalue_range,24,16
	.local	previous_instruction
	.comm	previous_instruction,240,32
	.local	stack_size
	.comm	stack_size,240,32
	.local	previous_pc
	.comm	previous_pc,480,32
	.local	last
	.comm	last,4,4
	.local	cache
	.comm	cache,524288,32
	.comm	current_prog,8,8
	.comm	caller_type,2,2
	.comm	pc,8,8
	.comm	sp,8,8
	.comm	fp,8,8
	.comm	control_stack,12400,32
	.comm	csp,8,8
	.comm	function_index_offset,4,4
	.comm	variable_index_offset,4,4
	.comm	num_varargs,4,4
	.comm	st_num_arg,4,4
	.comm	const0,16,16
	.comm	const1,16,16
	.comm	const0u,16,16
	.comm	lv_owner_type,4,4
	.comm	lv_owner,8,8
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	"zR"
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.uleb128 0x1
	.byte	0x3
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.long	.LFB52
	.long	.LFE52-.LFB52
	.uleb128 0x0
	.align 8
.LEFDE1:
.LSFDE3:
	.long	.LEFDE3-.LASFDE3
.LASFDE3:
	.long	.LASFDE3-.Lframe1
	.long	.LFB66
	.long	.LFE66-.LFB66
	.uleb128 0x0
	.align 8
.LEFDE3:
.LSFDE5:
	.long	.LEFDE5-.LASFDE5
.LASFDE5:
	.long	.LASFDE5-.Lframe1
	.long	.LFB82
	.long	.LFE82-.LFB82
	.uleb128 0x0
	.align 8
.LEFDE5:
.LSFDE7:
	.long	.LEFDE7-.LASFDE7
.LASFDE7:
	.long	.LASFDE7-.Lframe1
	.long	.LFB99
	.long	.LFE99-.LFB99
	.uleb128 0x0
	.align 8
.LEFDE7:
.LSFDE9:
	.long	.LEFDE9-.LASFDE9
.LASFDE9:
	.long	.LASFDE9-.Lframe1
	.long	.LFB105
	.long	.LFE105-.LFB105
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI0-.LFB105
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI1-.LCFI0
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI2-.LCFI1
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI3-.LCFI2
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI4-.LCFI3
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI5-.LCFI4
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI6-.LCFI5
	.byte	0xe
	.uleb128 0x40
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE9:
.LSFDE11:
	.long	.LEFDE11-.LASFDE11
.LASFDE11:
	.long	.LASFDE11-.Lframe1
	.long	.LFB106
	.long	.LFE106-.LFB106
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI7-.LFB106
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI8-.LCFI7
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI9-.LCFI8
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI10-.LCFI9
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI11-.LCFI10
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI12-.LCFI11
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI13-.LCFI12
	.byte	0xe
	.uleb128 0x50
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE11:
.LSFDE13:
	.long	.LEFDE13-.LASFDE13
.LASFDE13:
	.long	.LASFDE13-.Lframe1
	.long	.LFB110
	.long	.LFE110-.LFB110
	.uleb128 0x0
	.align 8
.LEFDE13:
.LSFDE15:
	.long	.LEFDE15-.LASFDE15
.LASFDE15:
	.long	.LASFDE15-.Lframe1
	.long	.LFB116
	.long	.LFE116-.LFB116
	.uleb128 0x0
	.align 8
.LEFDE15:
.LSFDE17:
	.long	.LEFDE17-.LASFDE17
.LASFDE17:
	.long	.LASFDE17-.Lframe1
	.long	.LFB117
	.long	.LFE117-.LFB117
	.uleb128 0x0
	.align 8
.LEFDE17:
.LSFDE19:
	.long	.LEFDE19-.LASFDE19
.LASFDE19:
	.long	.LASFDE19-.Lframe1
	.long	.LFB120
	.long	.LFE120-.LFB120
	.uleb128 0x0
	.align 8
.LEFDE19:
.LSFDE21:
	.long	.LEFDE21-.LASFDE21
.LASFDE21:
	.long	.LASFDE21-.Lframe1
	.long	.LFB121
	.long	.LFE121-.LFB121
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI14-.LFB121
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI15-.LCFI14
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI16-.LCFI15
	.byte	0xe
	.uleb128 0x28
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE21:
.LSFDE23:
	.long	.LEFDE23-.LASFDE23
.LASFDE23:
	.long	.LASFDE23-.Lframe1
	.long	.LFB123
	.long	.LFE123-.LFB123
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI17-.LFB123
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE23:
.LSFDE25:
	.long	.LEFDE25-.LASFDE25
.LASFDE25:
	.long	.LASFDE25-.Lframe1
	.long	.LFB134
	.long	.LFE134-.LFB134
	.uleb128 0x0
	.align 8
.LEFDE25:
.LSFDE27:
	.long	.LEFDE27-.LASFDE27
.LASFDE27:
	.long	.LASFDE27-.Lframe1
	.long	.LFB136
	.long	.LFE136-.LFB136
	.uleb128 0x0
	.align 8
.LEFDE27:
.LSFDE29:
	.long	.LEFDE29-.LASFDE29
.LASFDE29:
	.long	.LASFDE29-.Lframe1
	.long	.LFB137
	.long	.LFE137-.LFB137
	.uleb128 0x0
	.align 8
.LEFDE29:
.LSFDE31:
	.long	.LEFDE31-.LASFDE31
.LASFDE31:
	.long	.LASFDE31-.Lframe1
	.long	.LFB133
	.long	.LFE133-.LFB133
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI18-.LFB133
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI19-.LCFI18
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI20-.LCFI19
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE31:
.LSFDE33:
	.long	.LEFDE33-.LASFDE33
.LASFDE33:
	.long	.LASFDE33-.Lframe1
	.long	.LFB132
	.long	.LFE132-.LFB132
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI21-.LFB132
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI22-.LCFI21
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI23-.LCFI22
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI24-.LCFI23
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI25-.LCFI24
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI26-.LCFI25
	.byte	0xe
	.uleb128 0x40
	.byte	0x83
	.uleb128 0x6
	.byte	0x86
	.uleb128 0x5
	.byte	0x8c
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.align 8
.LEFDE33:
.LSFDE35:
	.long	.LEFDE35-.LASFDE35
.LASFDE35:
	.long	.LASFDE35-.Lframe1
	.long	.LFB50
	.long	.LFE50-.LFB50
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI27-.LFB50
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE35:
.LSFDE37:
	.long	.LEFDE37-.LASFDE37
.LASFDE37:
	.long	.LASFDE37-.Lframe1
	.long	.LFB98
	.long	.LFE98-.LFB98
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI28-.LFB98
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE37:
.LSFDE39:
	.long	.LEFDE39-.LASFDE39
.LASFDE39:
	.long	.LASFDE39-.Lframe1
	.long	.LFB93
	.long	.LFE93-.LFB93
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI29-.LFB93
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE39:
.LSFDE41:
	.long	.LEFDE41-.LASFDE41
.LASFDE41:
	.long	.LASFDE41-.Lframe1
	.long	.LFB91
	.long	.LFE91-.LFB91
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI30-.LFB91
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE41:
.LSFDE43:
	.long	.LEFDE43-.LASFDE43
.LASFDE43:
	.long	.LASFDE43-.Lframe1
	.long	.LFB90
	.long	.LFE90-.LFB90
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI31-.LFB90
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE43:
.LSFDE45:
	.long	.LEFDE45-.LASFDE45
.LASFDE45:
	.long	.LASFDE45-.Lframe1
	.long	.LFB89
	.long	.LFE89-.LFB89
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI32-.LFB89
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE45:
.LSFDE47:
	.long	.LEFDE47-.LASFDE47
.LASFDE47:
	.long	.LASFDE47-.Lframe1
	.long	.LFB88
	.long	.LFE88-.LFB88
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI33-.LFB88
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE47:
.LSFDE49:
	.long	.LEFDE49-.LASFDE49
.LASFDE49:
	.long	.LASFDE49-.Lframe1
	.long	.LFB87
	.long	.LFE87-.LFB87
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI34-.LFB87
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE49:
.LSFDE51:
	.long	.LEFDE51-.LASFDE51
.LASFDE51:
	.long	.LASFDE51-.Lframe1
	.long	.LFB86
	.long	.LFE86-.LFB86
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI35-.LFB86
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE51:
.LSFDE53:
	.long	.LEFDE53-.LASFDE53
.LASFDE53:
	.long	.LASFDE53-.Lframe1
	.long	.LFB85
	.long	.LFE85-.LFB85
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI36-.LFB85
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE53:
.LSFDE55:
	.long	.LEFDE55-.LASFDE55
.LASFDE55:
	.long	.LASFDE55-.Lframe1
	.long	.LFB84
	.long	.LFE84-.LFB84
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI37-.LFB84
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE55:
.LSFDE57:
	.long	.LEFDE57-.LASFDE57
.LASFDE57:
	.long	.LASFDE57-.Lframe1
	.long	.LFB83
	.long	.LFE83-.LFB83
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI38-.LFB83
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE57:
.LSFDE59:
	.long	.LEFDE59-.LASFDE59
.LASFDE59:
	.long	.LASFDE59-.Lframe1
	.long	.LFB81
	.long	.LFE81-.LFB81
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI39-.LFB81
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE59:
.LSFDE61:
	.long	.LEFDE61-.LASFDE61
.LASFDE61:
	.long	.LASFDE61-.Lframe1
	.long	.LFB79
	.long	.LFE79-.LFB79
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI40-.LFB79
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE61:
.LSFDE63:
	.long	.LEFDE63-.LASFDE63
.LASFDE63:
	.long	.LASFDE63-.Lframe1
	.long	.LFB55
	.long	.LFE55-.LFB55
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI41-.LFB55
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE63:
.LSFDE65:
	.long	.LEFDE65-.LASFDE65
.LASFDE65:
	.long	.LASFDE65-.Lframe1
	.long	.LFB54
	.long	.LFE54-.LFB54
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI42-.LFB54
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI43-.LCFI42
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE65:
.LSFDE67:
	.long	.LEFDE67-.LASFDE67
.LASFDE67:
	.long	.LASFDE67-.Lframe1
	.long	.LFB53
	.long	.LFE53-.LFB53
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI44-.LFB53
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE67:
.LSFDE69:
	.long	.LEFDE69-.LASFDE69
.LASFDE69:
	.long	.LASFDE69-.Lframe1
	.long	.LFB51
	.long	.LFE51-.LFB51
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI45-.LFB51
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE69:
.LSFDE71:
	.long	.LEFDE71-.LASFDE71
.LASFDE71:
	.long	.LASFDE71-.Lframe1
	.long	.LFB57
	.long	.LFE57-.LFB57
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI48-.LFB57
	.byte	0xe
	.uleb128 0x20
	.byte	0x86
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x3
	.align 8
.LEFDE71:
.LSFDE73:
	.long	.LEFDE73-.LASFDE73
.LASFDE73:
	.long	.LASFDE73-.Lframe1
	.long	.LFB49
	.long	.LFE49-.LFB49
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI49-.LFB49
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE73:
.LSFDE75:
	.long	.LEFDE75-.LASFDE75
.LASFDE75:
	.long	.LASFDE75-.Lframe1
	.long	.LFB70
	.long	.LFE70-.LFB70
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI53-.LFB70
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE75:
.LSFDE77:
	.long	.LEFDE77-.LASFDE77
.LASFDE77:
	.long	.LASFDE77-.Lframe1
	.long	.LFB135
	.long	.LFE135-.LFB135
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI54-.LFB135
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI55-.LCFI54
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI56-.LCFI55
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE77:
.LSFDE79:
	.long	.LEFDE79-.LASFDE79
.LASFDE79:
	.long	.LASFDE79-.Lframe1
	.long	.LFB125
	.long	.LFE125-.LFB125
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI57-.LFB125
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI58-.LCFI57
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI59-.LCFI58
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI60-.LCFI59
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI61-.LCFI60
	.byte	0xe
	.uleb128 0x130
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.byte	0x8d
	.uleb128 0x2
	.align 8
.LEFDE79:
.LSFDE81:
	.long	.LEFDE81-.LASFDE81
.LASFDE81:
	.long	.LASFDE81-.Lframe1
	.long	.LFB124
	.long	.LFE124-.LFB124
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI62-.LFB124
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI63-.LCFI62
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE81:
.LSFDE83:
	.long	.LEFDE83-.LASFDE83
.LASFDE83:
	.long	.LASFDE83-.Lframe1
	.long	.LFB128
	.long	.LFE128-.LFB128
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI64-.LFB128
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE83:
.LSFDE85:
	.long	.LEFDE85-.LASFDE85
.LASFDE85:
	.long	.LASFDE85-.Lframe1
	.long	.LFB92
	.long	.LFE92-.LFB92
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI65-.LFB92
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE85:
.LSFDE87:
	.long	.LEFDE87-.LASFDE87
.LASFDE87:
	.long	.LASFDE87-.Lframe1
	.long	.LFB58
	.long	.LFE58-.LFB58
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI68-.LFB58
	.byte	0xe
	.uleb128 0x20
	.byte	0x86
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x3
	.align 8
.LEFDE87:
.LSFDE89:
	.long	.LEFDE89-.LASFDE89
.LASFDE89:
	.long	.LASFDE89-.Lframe1
	.long	.LFB107
	.long	.LFE107-.LFB107
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI72-.LFB107
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE89:
.LSFDE91:
	.long	.LEFDE91-.LASFDE91
.LASFDE91:
	.long	.LASFDE91-.Lframe1
	.long	.LFB109
	.long	.LFE109-.LFB109
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI73-.LFB109
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI74-.LCFI73
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI75-.LCFI74
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI76-.LCFI75
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI77-.LCFI76
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI78-.LCFI77
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI79-.LCFI78
	.byte	0xe
	.uleb128 0x470
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE91:
.LSFDE93:
	.long	.LEFDE93-.LASFDE93
.LASFDE93:
	.long	.LASFDE93-.Lframe1
	.long	.LFB65
	.long	.LFE65-.LFB65
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI80-.LFB65
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI81-.LCFI80
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI82-.LCFI81
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE93:
.LSFDE95:
	.long	.LEFDE95-.LASFDE95
.LASFDE95:
	.long	.LASFDE95-.Lframe1
	.long	.LFB80
	.long	.LFE80-.LFB80
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI83-.LFB80
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI84-.LCFI83
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI85-.LCFI84
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI86-.LCFI85
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI87-.LCFI86
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI88-.LCFI87
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI89-.LCFI88
	.byte	0xe
	.uleb128 0x50
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE95:
.LSFDE97:
	.long	.LEFDE97-.LASFDE97
.LASFDE97:
	.long	.LASFDE97-.Lframe1
	.long	.LFB62
	.long	.LFE62-.LFB62
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI96-.LFB62
	.byte	0xe
	.uleb128 0x40
	.byte	0x8f
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x6
	.byte	0x83
	.uleb128 0x7
	.align 8
.LEFDE97:
.LSFDE99:
	.long	.LEFDE99-.LASFDE99
.LASFDE99:
	.long	.LASFDE99-.Lframe1
	.long	.LFB59
	.long	.LFE59-.LFB59
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI97-.LFB59
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE99:
.LSFDE101:
	.long	.LEFDE101-.LASFDE101
.LASFDE101:
	.long	.LASFDE101-.Lframe1
	.long	.LFB60
	.long	.LFE60-.LFB60
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI101-.LFB60
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE101:
.LSFDE103:
	.long	.LEFDE103-.LASFDE103
.LASFDE103:
	.long	.LASFDE103-.Lframe1
	.long	.LFB129
	.long	.LFE129-.LFB129
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI102-.LFB129
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI103-.LCFI102
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI104-.LCFI103
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI105-.LCFI104
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI106-.LCFI105
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI107-.LCFI106
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI108-.LCFI107
	.byte	0xe
	.uleb128 0x80
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE103:
.LSFDE105:
	.long	.LEFDE105-.LASFDE105
.LASFDE105:
	.long	.LASFDE105-.Lframe1
	.long	.LFB118
	.long	.LFE118-.LFB118
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI111-.LFB118
	.byte	0xe
	.uleb128 0x30
	.byte	0x86
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x3
	.align 8
.LEFDE105:
.LSFDE107:
	.long	.LEFDE107-.LASFDE107
.LASFDE107:
	.long	.LASFDE107-.Lframe1
	.long	.LFB127
	.long	.LFE127-.LFB127
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI112-.LFB127
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI113-.LCFI112
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI114-.LCFI113
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI115-.LCFI114
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI116-.LCFI115
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI117-.LCFI116
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI118-.LCFI117
	.byte	0xe
	.uleb128 0x90
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE107:
.LSFDE109:
	.long	.LEFDE109-.LASFDE109
.LASFDE109:
	.long	.LASFDE109-.Lframe1
	.long	.LFB69
	.long	.LFE69-.LFB69
	.uleb128 0x0
	.align 8
.LEFDE109:
.LSFDE111:
	.long	.LEFDE111-.LASFDE111
.LASFDE111:
	.long	.LASFDE111-.Lframe1
	.long	.LFB68
	.long	.LFE68-.LFB68
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI119-.LFB68
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI120-.LCFI119
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI121-.LCFI120
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE111:
.LSFDE113:
	.long	.LEFDE113-.LASFDE113
.LASFDE113:
	.long	.LASFDE113-.Lframe1
	.long	.LFB48
	.long	.LFE48-.LFB48
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI122-.LFB48
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE113:
.LSFDE115:
	.long	.LEFDE115-.LASFDE115
.LASFDE115:
	.long	.LASFDE115-.Lframe1
	.long	.LFB61
	.long	.LFE61-.LFB61
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI123-.LFB61
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE115:
.LSFDE117:
	.long	.LEFDE117-.LASFDE117
.LASFDE117:
	.long	.LASFDE117-.Lframe1
	.long	.LFB76
	.long	.LFE76-.LFB76
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI124-.LFB76
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI125-.LCFI124
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI126-.LCFI125
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE117:
.LSFDE119:
	.long	.LEFDE119-.LASFDE119
.LASFDE119:
	.long	.LASFDE119-.Lframe1
	.long	.LFB138
	.long	.LFE138-.LFB138
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI127-.LFB138
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI128-.LCFI127
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI129-.LCFI128
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE119:
.LSFDE121:
	.long	.LEFDE121-.LASFDE121
.LASFDE121:
	.long	.LASFDE121-.Lframe1
	.long	.LFB126
	.long	.LFE126-.LFB126
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI130-.LFB126
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI131-.LCFI130
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI132-.LCFI131
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI133-.LCFI132
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI134-.LCFI133
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI135-.LCFI134
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI136-.LCFI135
	.byte	0xe
	.uleb128 0x180
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE121:
.LSFDE123:
	.long	.LEFDE123-.LASFDE123
.LASFDE123:
	.long	.LASFDE123-.Lframe1
	.long	.LFB130
	.long	.LFE130-.LFB130
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI137-.LFB130
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI138-.LCFI137
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI139-.LCFI138
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE123:
.LSFDE125:
	.long	.LEFDE125-.LASFDE125
.LASFDE125:
	.long	.LASFDE125-.Lframe1
	.long	.LFB94
	.long	.LFE94-.LFB94
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI140-.LFB94
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI141-.LCFI140
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI142-.LCFI141
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI143-.LCFI142
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI144-.LCFI143
	.byte	0xe
	.uleb128 0x30
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.byte	0x8d
	.uleb128 0x2
	.align 8
.LEFDE125:
.LSFDE127:
	.long	.LEFDE127-.LASFDE127
.LASFDE127:
	.long	.LASFDE127-.Lframe1
	.long	.LFB96
	.long	.LFE96-.LFB96
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI145-.LFB96
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI146-.LCFI145
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI147-.LCFI146
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI148-.LCFI147
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI149-.LCFI148
	.byte	0xe
	.uleb128 0x30
	.byte	0x83
	.uleb128 0x6
	.byte	0x86
	.uleb128 0x5
	.byte	0x8c
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.align 8
.LEFDE127:
.LSFDE129:
	.long	.LEFDE129-.LASFDE129
.LASFDE129:
	.long	.LASFDE129-.Lframe1
	.long	.LFB97
	.long	.LFE97-.LFB97
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI150-.LFB97
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI151-.LCFI150
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI152-.LCFI151
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI153-.LCFI152
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI154-.LCFI153
	.byte	0xe
	.uleb128 0x30
	.byte	0x83
	.uleb128 0x6
	.byte	0x86
	.uleb128 0x5
	.byte	0x8c
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8e
	.uleb128 0x2
	.align 8
.LEFDE129:
.LSFDE131:
	.long	.LEFDE131-.LASFDE131
.LASFDE131:
	.long	.LASFDE131-.Lframe1
	.long	.LFB75
	.long	.LFE75-.LFB75
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI155-.LFB75
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI156-.LCFI155
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI157-.LCFI156
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI158-.LCFI157
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI159-.LCFI158
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI160-.LCFI159
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI161-.LCFI160
	.byte	0xe
	.uleb128 0x70
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE131:
.LSFDE133:
	.long	.LEFDE133-.LASFDE133
.LASFDE133:
	.long	.LASFDE133-.Lframe1
	.long	.LFB100
	.long	.LFE100-.LFB100
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI162-.LFB100
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI163-.LCFI162
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI164-.LCFI163
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x4
	.byte	0x86
	.uleb128 0x3
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE133:
.LSFDE135:
	.long	.LEFDE135-.LASFDE135
.LASFDE135:
	.long	.LASFDE135-.Lframe1
	.long	.LFB71
	.long	.LFE71-.LFB71
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI165-.LFB71
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE135:
.LSFDE137:
	.long	.LEFDE137-.LASFDE137
.LASFDE137:
	.long	.LASFDE137-.Lframe1
	.long	.LFB67
	.long	.LFE67-.LFB67
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI166-.LFB67
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI167-.LCFI166
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI168-.LCFI167
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x2
	.align 8
.LEFDE137:
.LSFDE139:
	.long	.LEFDE139-.LASFDE139
.LASFDE139:
	.long	.LASFDE139-.Lframe1
	.long	.LFB64
	.long	.LFE64-.LFB64
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI169-.LFB64
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI170-.LCFI169
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI171-.LCFI170
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x4
	.byte	0x86
	.uleb128 0x3
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE139:
.LSFDE141:
	.long	.LEFDE141-.LASFDE141
.LASFDE141:
	.long	.LASFDE141-.Lframe1
	.long	.LFB78
	.long	.LFE78-.LFB78
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI172-.LFB78
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE141:
.LSFDE143:
	.long	.LEFDE143-.LASFDE143
.LASFDE143:
	.long	.LASFDE143-.Lframe1
	.long	.LFB77
	.long	.LFE77-.LFB77
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI173-.LFB77
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE143:
.LSFDE145:
	.long	.LEFDE145-.LASFDE145
.LASFDE145:
	.long	.LASFDE145-.Lframe1
	.long	.LFB74
	.long	.LFE74-.LFB74
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI174-.LFB74
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI175-.LCFI174
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI176-.LCFI175
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI177-.LCFI176
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI178-.LCFI177
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI179-.LCFI178
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI180-.LCFI179
	.byte	0xe
	.uleb128 0x70
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE145:
.LSFDE147:
	.long	.LEFDE147-.LASFDE147
.LASFDE147:
	.long	.LASFDE147-.Lframe1
	.long	.LFB72
	.long	.LFE72-.LFB72
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI184-.LFB72
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE147:
.LSFDE149:
	.long	.LEFDE149-.LASFDE149
.LASFDE149:
	.long	.LASFDE149-.Lframe1
	.long	.LFB103
	.long	.LFE103-.LFB103
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI185-.LFB103
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI186-.LCFI185
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI187-.LCFI186
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI188-.LCFI187
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI189-.LCFI188
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI190-.LCFI189
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI191-.LCFI190
	.byte	0xe
	.uleb128 0xc0
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE149:
.LSFDE151:
	.long	.LEFDE151-.LASFDE151
.LASFDE151:
	.long	.LASFDE151-.Lframe1
	.long	.LFB119
	.long	.LFE119-.LFB119
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI192-.LFB119
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI193-.LCFI192
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI194-.LCFI193
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI195-.LCFI194
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI196-.LCFI195
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI197-.LCFI196
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI198-.LCFI197
	.byte	0xe
	.uleb128 0x40
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE151:
.LSFDE153:
	.long	.LEFDE153-.LASFDE153
.LASFDE153:
	.long	.LASFDE153-.Lframe1
	.long	.LFB113
	.long	.LFE113-.LFB113
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI199-.LFB113
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI200-.LCFI199
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI201-.LCFI200
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI202-.LCFI201
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI203-.LCFI202
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI204-.LCFI203
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI205-.LCFI204
	.byte	0xe
	.uleb128 0x40
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE153:
.LSFDE155:
	.long	.LEFDE155-.LASFDE155
.LASFDE155:
	.long	.LASFDE155-.Lframe1
	.long	.LFB111
	.long	.LFE111-.LFB111
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI206-.LFB111
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI207-.LCFI206
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI208-.LCFI207
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI209-.LCFI208
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI210-.LCFI209
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI211-.LCFI210
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI212-.LCFI211
	.byte	0xe
	.uleb128 0x70
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE155:
.LSFDE157:
	.long	.LEFDE157-.LASFDE157
.LASFDE157:
	.long	.LASFDE157-.Lframe1
	.long	.LFB115
	.long	.LFE115-.LFB115
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI213-.LFB115
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI214-.LCFI213
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI215-.LCFI214
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI216-.LCFI215
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI217-.LCFI216
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI218-.LCFI217
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI219-.LCFI218
	.byte	0xe
	.uleb128 0x60
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE157:
.LSFDE159:
	.long	.LEFDE159-.LASFDE159
.LASFDE159:
	.long	.LASFDE159-.Lframe1
	.long	.LFB112
	.long	.LFE112-.LFB112
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI220-.LFB112
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE159:
.LSFDE161:
	.long	.LEFDE161-.LASFDE161
.LASFDE161:
	.long	.LASFDE161-.Lframe1
	.long	.LFB114
	.long	.LFE114-.LFB114
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI221-.LFB114
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI222-.LCFI221
	.byte	0xe
	.uleb128 0x120
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE161:
.LSFDE163:
	.long	.LEFDE163-.LASFDE163
.LASFDE163:
	.long	.LASFDE163-.Lframe1
	.long	.LFB63
	.long	.LFE63-.LFB63
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI223-.LFB63
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI224-.LCFI223
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI225-.LCFI224
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI226-.LCFI225
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI227-.LCFI226
	.byte	0xe
	.uleb128 0x30
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.byte	0x8d
	.uleb128 0x2
	.align 8
.LEFDE163:
.LSFDE165:
	.long	.LEFDE165-.LASFDE165
.LASFDE165:
	.long	.LASFDE165-.Lframe1
	.long	.LFB104
	.long	.LFE104-.LFB104
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI228-.LFB104
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI229-.LCFI228
	.byte	0xe
	.uleb128 0x110
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE165:
	.ident	"GCC: (GNU) 4.1.1 20070105 (Red Hat 4.1.1-51)"
	.section	.note.GNU-stack,"",@progbits
