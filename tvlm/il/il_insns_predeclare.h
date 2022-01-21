/** Loads ImmValue, which can be type of float or int
 *  represents: constant value
 */
class LoadImm;

/** Allocates memory in global scope, of size @size
 * represents: address of allocated memory
 */
class AllocG;

/** Allocates memory in local scope, of size @size
 * represents: address of allocated memory
 */
class AllocL;


/** Counts address on stack of argument specified at index @index
 * represents: address of argument
 */
class ArgAddr;
/**
 * Performs binary operation @op between @lhs and @rhs
 * represents: result of binary operation
 */
class BinOp;

/**
 * Performs unary operation @op on @operand
 * represents: result of unary operation
 */
class UnOp;

/**
 * Performs return from function call initiated with Call instruction
 */
class Return;

/**
 * Performs conditional jump according specified condition @condition
 *  if @condiditon results false, jump to 0th element in @targets
 *  otherwise jump to 1st element in @targets
 *
 *  @targets are created by calling 'addTarget' function
 */
class CondJump;

/**
 * Performs jump to BasicBLock specified by @target
 */
class Jump;

/**
 * Ends the program
 */
class Halt;

/**
 * Interacts with the Memory, loads memory cell at 'address' differing by type specified by argument 'type'
 */
 class Load;

/**
 * Interacts with the Memory, stores register value into cell at 'address'
 */
class Store;

/**
 * Copies value from src instruction
 */
class Copy;

/** Utilizes I/O operation
 *  @src has to carry ASCII value of type integer
 */
class PutChar;

/** Utilizes I/O operation
 * stops execution until a input is given by user
 * represents: value supplied by user
 */
class GetChar;

/**
 * Instruction by which is possible to carry information about regAllocation
 * it is used to specify single register for multiple assignments,
 * which would normally consume as many regs as there is assignments
 * represents unification among registers (so there is no need to carry it threw memory)
 */
class Phi;

/**
 * Represents a call to a function that is not known. 'indirection by function pointers'
 */
class Call;

/**
 * Represents a call to a statically known function. 'main(args...)'
 */
class CallStatic;

/**
 */
class Extend;

/**
 */
class Truncate;

/**
 *
 */
class ElemAddrIndex;

/**
 *
 */
class ElemAddrOffset;

/**
 *
 */
class StructAssign;
