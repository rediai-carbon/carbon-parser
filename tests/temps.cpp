// Copyright © 2022 CCP ehf.

#include "pch.h"
#include "../include/ccpparser.h"

#include <string>
#include <vector>

using namespace CcpParser;

namespace
{
float Add8( float a, float b, float c, float d, float e, float f, float g, float h )
{
	return a + b + c + d + e + f + g + h;
}

size_t ArenaSize( const char* expression, const std::vector<Variable>& variables, const std::vector<Function>& functions )
{
	Externals externals;
	VariableView variablesView[] = { variables };
	FunctionView functionsView[] = { functions };
	externals.variables = variablesView;
	externals.functions = functionsView;
	Program program;
	const auto parsed = Parse( expression, externals, program );
	EXPECT_TRUE( parsed ) << expression;
	return program.GetTempArenaSize();
}

std::string NestedAdds( int depth )
{
	if( depth == 0 )
	{
		return "0";
	}
	std::string expression = "wide(a+1,a+1,a+1,a+1,a+1,a+1,a+1,";
	expression += NestedAdds( depth - 1 );
	expression += ")";
	return expression;
}
}

TEST( Temps, NonzeroMasksUseDistinctSlots )
{
	const std::vector<Variable> variables = { { "a", 0, 0 } };
	// Nine or more simultaneous temps are visible. GetTempArenaSize reports 0
	// for an arena of 32 bytes or less because Eval keeps that buffer itself.
	ASSERT_EQ( 15 * sizeof( float ), ArenaSize( NestedAdds( 2 ).c_str(), variables, { Function( "wide", &Add8 ) } ) );
}

TEST( Temps, ZeroMaskReusesTheLowestSlot )
{
	const std::vector<Variable> variables = { { "a", 0, 0 } };
	const auto functions = std::vector<Function>{ Function( "wide", &Add8 ) };
	ASSERT_EQ( 64 * sizeof( float ), ArenaSize( NestedAdds( 10 ).c_str(), variables, functions ) );
}
