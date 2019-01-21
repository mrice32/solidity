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

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <unordered_map>
#include <string>
#include <boost/optional.hpp>

namespace yul
{

class VarNameCleaner: public ASTModifier
{
public:
	using ASTModifier::operator();
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(Identifier&) override;

private:
	/// @returns suffix-stripped name, if a suffix was detected, none otherwise.
	static boost::optional<std::string> stripSuffix(std::string const& _name);

	/// @returns some name if it could uniquely clean the name, none otherwise.
	boost::optional<std::string> makeCleanName(std::string const& name);

	/// Retruens the new name, if one was mapped, or none.
	boost::optional<std::string> getCleanName(std::string const& _name) const;

	/// map on old name to new name
	std::unordered_map<std::string, std::string> m_usedNames;
};

}
