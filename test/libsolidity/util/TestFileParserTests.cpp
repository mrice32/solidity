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
 * Unit tests for Solidity's test expectation parser.
 */

#include <functional>
#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <liblangutil/Exceptions.h>
#include <test/libsolidity/SolidityExecutionFramework.h>

#include <test/libsolidity/util/TestFileParser.h>

using namespace std;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

vector<FunctionCall> parse(string const& _source)
{
	istringstream stream{_source, ios_base::out};
	TestFileParser parser{stream};
	return parser.parseFunctionCalls();
}

BOOST_AUTO_TEST_SUITE(TestFileParserTest)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* source = R"()";
	BOOST_CHECK_EQUAL(parse(source).size(), 0);
}

BOOST_AUTO_TEST_CASE(simple_call_succees)
{
	char const* source = R"(
		// f()
		// -> 1
	)";
	auto const& calls = parse(source);
	BOOST_CHECK_EQUAL(calls.size(), 1);

	auto const& call = calls.at(0);
	BOOST_CHECK_EQUAL(call.signature, "f()");
	BOOST_CHECK_EQUAL(call.expectations.output, "-> 1");
	BOOST_CHECK_EQUAL(call.expectations.raw, "1");
}

BOOST_AUTO_TEST_CASE(non_existent_call_revert)
{
	char const* source = R"(
		// i_am_not_there()
		// REVERT
	)";
	auto const& calls = parse(source);
	BOOST_CHECK_EQUAL(calls.size(), 1);

	auto const& call = calls.at(0);
	BOOST_CHECK_EQUAL(call.signature, "i_am_not_there()");
	BOOST_CHECK_EQUAL(call.expectations.output, "REVERT");
	BOOST_CHECK_EQUAL(call.expectations.raw, "");
}

BOOST_AUTO_TEST_CASE(call_comments)
{
	char const* source = R"(
		// f() # This is a comment
		// -> 1 # This is another comment
	)";
	auto const& calls = parse(source);
	BOOST_CHECK_EQUAL(calls.size(), 1);

	auto const& call = calls.at(0);
	BOOST_CHECK_EQUAL(call.signature, "f()");
	BOOST_CHECK_EQUAL(call.arguments.comment, "This is a comment");
	BOOST_CHECK_EQUAL(call.expectations.output, "-> 1");
	BOOST_CHECK_EQUAL(call.expectations.raw, "1");
	BOOST_CHECK_EQUAL(call.expectations.comment, "This is another comment");
}

BOOST_AUTO_TEST_CASE(call_arguments)
{
	char const* source = R"(
		// f(uint256): 5
		// : 314 # optional ether value
		// -> -4
	)";
	auto const& calls = parse(source);
	BOOST_CHECK_EQUAL(calls.size(), 1);

	auto const& call = calls.at(0);
	BOOST_CHECK_EQUAL(call.signature, "f(uint256)");
	BOOST_CHECK_EQUAL(call.value, u256{314});
	BOOST_CHECK_EQUAL(call.arguments.raw, "5");
	BOOST_CHECK_EQUAL(call.expectations.output, "-> -4");
	BOOST_CHECK_EQUAL(call.expectations.raw, "-4");
	ABI_CHECK(call.arguments.rawBytes, toBigEndian(u256{5}));
	ABI_CHECK(call.expectations.rawBytes, toBigEndian(u256{-4}));
}

BOOST_AUTO_TEST_CASE(call_expectations_missing)
{
	char const* source = R"(
		// f())";
	BOOST_CHECK_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_ether_value_expectations_missing)
{
	char const* source = R"(
		// f()
		// : 0)";
	BOOST_CHECK_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_invalid)
{
	char const* source = R"(
		// f(uint256): abc
		// -> 1
	)";
	BOOST_CHECK_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_ether_value_invalid)
{
	char const* source = R"(
		// f(uint256): 1
		// : abc
		// -> 1
	)";
	BOOST_CHECK_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_mismatch)
{
	char const* source = R"(
		// f(uint256, uint256): 1 # This only throws at runtime
		// -> 1
	)";
	auto const& calls = parse(source);
	BOOST_CHECK_EQUAL(calls.size(), 1);

	auto const& call = calls.at(0);
	BOOST_CHECK_EQUAL(call.signature, "f(uint256, uint256)");
	BOOST_CHECK_EQUAL(call.arguments.raw, "1");
}

BOOST_AUTO_TEST_CASE(call_multiple_arguments)
{
	char const* source = R"(
		// f(uint256, uint256): 1, 2
		// -> 1, 1
	)";
	auto const& calls = parse(source);
	BOOST_CHECK_EQUAL(calls.size(), 1);

	auto const& call = calls.at(0);
	BOOST_CHECK_EQUAL(call.signature, "f(uint256, uint256)");
	BOOST_CHECK_EQUAL(call.arguments.raw, "1, 2");
	ABI_CHECK(call.arguments.rawBytes, toBigEndian(u256{1}) + toBigEndian(u256{2}));
}

BOOST_AUTO_TEST_CASE(call_multiple_arguments_mixed_format)
{
	char const* source = R"(
		// f(uint256, uint256): -1, 2
		// -> 1, -2
	)";
	auto const& calls = parse(source);
	BOOST_CHECK_EQUAL(calls.size(), 1);

	auto const& call = calls.at(0);
	BOOST_CHECK_EQUAL(call.signature, "f(uint256, uint256)");
	BOOST_CHECK_EQUAL(call.arguments.raw, "-1, 2");
	ABI_CHECK(call.arguments.rawBytes, toBigEndian(u256{-1}) + toBigEndian(u256{2}));
	ABI_CHECK(call.expectations.rawBytes, toBigEndian(u256{1}) + toBigEndian(u256{-2}));
}

BOOST_AUTO_TEST_CASE(parser_convert_string_to_bytes)
{
	string raw{"1, -1"};
	auto bytesFormat = TestFileParser::formattedStringToBytes(raw);
	ABI_CHECK(bytesFormat.first, toBigEndian(u256{1}) + toBigEndian(u256{-1}));
	BOOST_CHECK_EQUAL(bytesFormat.second.size(), 2);
	BOOST_CHECK_EQUAL(bytesFormat.second.at(0).type, ByteFormat::UnsignedDec);
	BOOST_CHECK_EQUAL(bytesFormat.second.at(1).type, ByteFormat::SignedDec);
}

BOOST_AUTO_TEST_CASE(parser_convert_bytes_to_string)
{
	bytes rawBytes = toBigEndian(u256{1}) + toBigEndian(u256{-1});
	vector<ByteFormat> formats
	{
		ByteFormat{ByteFormat::UnsignedDec},
		ByteFormat{ByteFormat::SignedDec}
	};
	string formatted = TestFileParser::bytesToFormattedString(rawBytes, formats);
	BOOST_CHECK_EQUAL(formatted, "1,-1");
}


BOOST_AUTO_TEST_SUITE_END()

}
}
}
