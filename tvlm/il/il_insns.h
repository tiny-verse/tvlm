#ifndef INS
#define INS(...)
#endif
#ifndef INSTYPE
#define INSTYPE(...)
#endif

/** Loads ImmValue, which can be type of float or int
 *  represents: constant value
 */
INS(LoadImm, ImmValue)

/** Allocates memory in global scope, of size @size
 * represents: address of allocated memory
 */
INS(AllocG, ImmSize)

/** Allocates memory in local scope, of size @size
 * represents: address of allocated memory
 */
INS(AllocL, ImmSize)


/** Counts address on stack of argument specified at index @index
 * represents: address of argument
 */
INS(ArgAddr, ImmIndex)
/**
 * Performs binary operation @op between @lhs and @rhs
 * represents: result of binary operation
 */
INS(BinOp, BinaryOperator)

/**
 * Performs unary operation @op on @operand
 * represents: result of unary operation
 */
INS(UnOp, UnaryOperator)

/**
 * Performs return from function call initiated with Call instruction
 */
INS(Return, Returnator)

/**
 * Performs conditional jump according specified condition @condition
 *  if @condiditon results false, jump to 0th element in @targets
 *  otherwise jump to 1st element in @targets
 *
 *  @targets are created by calling 'addTarget' function
 */
INS(CondJump, Terminator2)

/**
 * Performs jump to BasicBLock specified by @target
 */
INS(Jump, Terminator1)

/**
 * Ends the program
 */
INS(Halt, Terminator0)

/**
 * Interacts with the Memory, loads memory cell at 'address' differing by type specified by argument 'type'
 */
 INS(Load, LoadAddress)

/**
 * Interacts with the Memory, stores register value into cell at 'address'
 */
INS(Store, StoreAddress)

/**
 * Copies value from src instruction
 */
INSTYPE(Copy, SrcInstruction, src->resultType())

/** Utilizes I/O operation
 *  @src has to carry ASCII value of type integer
 */
INSTYPE(PutChar, SrcInstruction, ResultType::Void)

/** Utilizes I/O operation
 * stops execution until a input is given by user
 * represents: value supplied by user
 */
INSTYPE(GetChar,VoidInstruction, ResultType::Integer)

/**
 * Instruction by which is possible to carry information about regAllocation
 * it is used to specify single register for multiple assignments,
 * which would normally consume as many regs as there is assignments
 * represents unification among registers (so there is no need to carry it threw memory)
 */
INS(Phi, PhiInstruction)

/**
 * Represents a call to a function that is not known. 'indirection by function pointers'
 */
INS(Call,IndirectCallInstruction)

/**
 * Represents a call to a statically known function. 'main(args...)'
 */
INS(CallStatic,DirectCallInstruction)

/**
 */
INSTYPE(Extend, SrcInstruction, ResultType::Double)

/**
 */
INSTYPE(Truncate, SrcInstruction, ResultType::Integer)

/**
 *
 */
INS(ElemAddrIndex, ElemIndexInstruction )

/**
 *
 */
INS(ElemAddrOffset, ElemOffsetInstruction )

/**
 *
 */
INS(StructAssign, StructAssignInstruction )



#undef INS

#undef INSTYPE

#ifdef ImmSize
#undef ImmSize
#endif
#ifdef ImmIndex
#undef ImmIndex
#endif
#ifdef ImmValue
#undef ImmValue
#endif
#ifdef BinaryOperator
#undef BinaryOperator
#endif
#ifdef UnaryOperator
#undef UnaryOperator
#endif
#ifdef Terminator0
#undef Terminator0
#endif
#ifdef Terminator1
#undef Terminator1
#endif
#ifdef Terminator2
#undef Terminator2
#endif
#ifdef LoadAddress
#undef LoadAddress
#endif
#ifdef StoreAddress
#undef StoreAddress
#endif
#ifdef SrcInstruction
#undef SrcInstruction
#endif
#ifdef VoidInstruction
#undef VoidInstruction
#endif
#ifdef PhiInstruction
#undef PhiInstruction
#endif
#ifdef IndirectCallInstruction
#undef IndirectCallInstruction
#endif
#ifdef DirectCallInstruction
#undef DirectCallInstruction
#endif
#ifdef Returnator
#undef Returnator
#endif
#ifdef ElemAddrIndexInstruction
#undef ElemAddrIndexInstruction
#endif
#ifdef ElemAddrOffsetInstruction
#undef ElemAddrOffsetInstruction
#endif
#ifdef StructAssignInstruction
#undef StructAssignInstruction
#endif

//INS(Add, BinaryOperator)
//INS(Sub, BinaryOperator)
//INS(Mul, BinaryOperator)
//INS(Div, BinaryOperator)
//INS(Mod, BinaryOperator)
//INS(Shr, BinaryOperator)
//INS(Shl, BinaryOperator)
//INS(And, BinaryOperator)
//INS(Or, BinaryOperator)
//INS(Xor, BinaryOperator)
//INS(Not, UnaryOperator)
