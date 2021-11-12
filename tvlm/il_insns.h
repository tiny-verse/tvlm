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

/** Performs binary operation @op between @lhs and @rhs
 * represents: result of binary operation
 */
INS(BinOp, BinaryOperator)

/** Performs unary operation @op on @operand
 * represents: result of unary operation
 */
INS(UnOp, UnaryOperator)

/** Performs return from function call initiated with Call instruction
 *
 */
INS(Return, Returnator)

/**
 * Performs conditional jump according specified condition @condition
 *  if @condiditon results false, jump to 0th element in @targets
 *  otherwise jump to 1st emelent in @targets
 *
 *  @targets are created by calling 'addTarget' function
 */
INS(CondJump, TerminatorN)

/**
 * Performs jump to BasicBLock specified by @target
 */
INS(Jump, Terminator1)


INS(Halt, Terminator0)

/**
 *
 */
INS(Load, LoadAddress)

/**
 *
 */
INS(Store, StoreAddress)

/**
 *
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
INSTYPE(Getchar,VoidInstruction, ResultType::Integer)


INS(Phi, PhiInstruction)


INS(Call,IndirectCallInstruction)
INS(CallStatic,DirectCallInstruction)

INSTYPE(Extend, SrcInstruction, ResultType::Double)
INSTYPE(Truncate, SrcInstruction, ResultType::Integer)

/** Performs computation of address of a subelement of an aggregate data structure. Such as struct or array
 * represents: Address of member in a structure
 * indexes of a member are added by function `addIndex` // TODO
 */
INSTYPE(ElemAddr,VoidInstruction , ResultType::Integer)





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
#ifdef TerminatorN
#undef TerminatorN
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
