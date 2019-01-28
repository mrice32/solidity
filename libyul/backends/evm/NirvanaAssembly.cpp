/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Assembly interface that ignores everything. Can be used as a backend for a compilation dry-run.
 */

#include <libyul/backends/evm/NirvanaAssembly.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/Exceptions.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;


void NirvanaAssembly::appendInstruction(solidity::Instruction _instr)
{
	m_stackHeight += solidity::instructionInfo(_instr).ret - solidity::instructionInfo(_instr).args;
}

void NirvanaAssembly::appendConstant(u256 const&)
{
	appendInstruction(solidity::pushInstruction(1));
}

void NirvanaAssembly::appendLabel(LabelID)
{
	appendInstruction(solidity::Instruction::JUMPDEST);
}

void NirvanaAssembly::appendLabelReference(LabelID)
{
	solAssert(!m_evm15, "Cannot use plain label references in EMV1.5 mode.");
	appendInstruction(solidity::pushInstruction(1));
}

NirvanaAssembly::LabelID NirvanaAssembly::newLabelId()
{
	return 1;
}

AbstractAssembly::LabelID NirvanaAssembly::namedLabel(string const&)
{
	return 1;
}

void NirvanaAssembly::appendLinkerSymbol(string const&)
{
	solAssert(false, "Linker symbols not yet implemented.");
}

void NirvanaAssembly::appendJump(int _stackDiffAfter)
{
	solAssert(!m_evm15, "Plain JUMP used for EVM 1.5");
	appendInstruction(solidity::Instruction::JUMP);
	m_stackHeight += _stackDiffAfter;
}

void NirvanaAssembly::appendJumpTo(LabelID _labelId, int _stackDiffAfter)
{
	if (m_evm15)
		m_stackHeight += _stackDiffAfter;
	else
	{
		appendLabelReference(_labelId);
		appendJump(_stackDiffAfter);
	}
}

void NirvanaAssembly::appendJumpToIf(LabelID _labelId)
{
	if (m_evm15)
		m_stackHeight--;
	else
	{
		appendLabelReference(_labelId);
		appendInstruction(solidity::Instruction::JUMPI);
	}
}

void NirvanaAssembly::appendBeginsub(LabelID, int _arguments)
{
	solAssert(m_evm15, "BEGINSUB used for EVM 1.0");
	solAssert(_arguments >= 0, "");
	m_stackHeight += _arguments;
}

void NirvanaAssembly::appendJumpsub(LabelID, int _arguments, int _returns)
{
	solAssert(m_evm15, "JUMPSUB used for EVM 1.0");
	solAssert(_arguments >= 0 && _returns >= 0, "");
	m_stackHeight += _returns - _arguments;
}

void NirvanaAssembly::appendReturnsub(int _returns, int _stackDiffAfter)
{
	solAssert(m_evm15, "RETURNSUB used for EVM 1.0");
	solAssert(_returns >= 0, "");
	m_stackHeight += _stackDiffAfter - _returns;
}

void NirvanaAssembly::appendAssemblySize()
{
	appendInstruction(solidity::Instruction::PUSH1);
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> NirvanaAssembly::createSubAssembly()
{
	solAssert(false, "Sub assemblies not implemented.");
	return {};
}

void NirvanaAssembly::appendDataOffset(AbstractAssembly::SubID)
{
	appendInstruction(solidity::Instruction::PUSH1);
}

void NirvanaAssembly::appendDataSize(AbstractAssembly::SubID)
{
	appendInstruction(solidity::Instruction::PUSH1);
}

AbstractAssembly::SubID NirvanaAssembly::appendData(bytes const&)
{
	return 1;
}
