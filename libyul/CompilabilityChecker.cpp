/*(
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
 * Component that checks whether all variables are reachable on the stack.
 */

#include <libyul/CompilabilityChecker.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/NirvanaAssembly.h>

using namespace std;
using namespace yul;

int CompilabilityChecker::run(FunctionDefinition const& /*_function*/)
{
	if (m_dialect.flavour == AsmFlavour::Yul)
		return 0;

//	EVMDialect const& evmDialect = dynamic_cast<EVMDialect const&>(m_dialect);
//	NirvanaAssembly assembly;
//	CodeTransform(assembly, analysisInfo, block, evmDialect, _optimize);

	return 0;
}

int CompilabilityChecker::run(Block const& /*_ast*/)
{
	// TODO
	return 0;
}
