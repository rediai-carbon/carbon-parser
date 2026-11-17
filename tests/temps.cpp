// Copyright © 2022 CCP ehf.

#include "pch.h"
#include "../include/ccpparser.h"
#include "helpers.h"

#include <vector>

using namespace CcpParser;

namespace
{
float Add8( float a, float b, float c, float d, float e, float f, float g, float h )
{
	return a + b + c + d + e + f + g + h;
}

float Sub2( float x, float y )
{
	return x - y;
}

size_t ArenaSize( const char* expression, const std::vector<Function>& functions )
{
	Externals externals;
	FunctionView funcs[] = { functions };
	externals.functions = funcs;
	Program program;
	auto parsed = Parse( expression, externals, program );
	EXPECT_TRUE( parsed );
	return program.GetTempArenaSize();
}

std::string NestedAdds( int depth )
{
	if( depth == 0 )
	{
		return "0";
	}
	std::string expression = "wide(1+1,1+1,1+1,1+1,1+1,1+1,1+1,";
	expression += NestedAdds( depth - 1 );
	expression += ")";
	return expression;
}
}

TEST( Temps, NonzeroMasksUseDistinctSlots )
{
	ASSERT_EQ( sizeof( float ), ArenaSize( "1+2", {} ) );
	ASSERT_EQ( 3 * sizeof( float ), ArenaSize( "gap(1+2, 3+4)", { Function( "gap", &Sub2 ) } ) );
}

TEST( Temps, ZeroMaskReusesTheLowestSlot )
{
	const auto functions = std::vector<Function>{ Function( "wide", &Add8 ) };
	ASSERT_EQ( 64 * sizeof( float ), ArenaSize( NestedAdds( 10 ).c_str(), functions ) );
}
