// Copyright © 2026 RediAI. Out-of-tree consumer for installed carbon-parser.
#include <ccpparser.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

using namespace CcpParser;

static bool eval_expr(const char* expr, float& out)
{
	Program program;
	ParseResult pr = Parse(expr, {}, program);
	if (!pr)
	{
		std::printf("parse_fail type=%d expr=%s\n", static_cast<int>(pr.type), expr);
		return false;
	}
	auto arena = std::make_unique<uint8_t[]>(program.GetTempArenaSize());
	out = program.Eval(nullptr, arena.get());
	return true;
}

static bool eval_with_x(const char* expr, float x, float& out)
{
	std::vector<Variable> vars = { { "x", 0, 0 } };
	std::vector<float> values = { x };
	VariableView varViews[] = { vars };
	Externals ext{};
	ext.variables = varViews;
	Program program;
	ParseResult pr = Parse(expr, ext, program);
	if (!pr)
	{
		std::printf("parse_fail type=%d expr=%s\n", static_cast<int>(pr.type), expr);
		return false;
	}
	auto arena = std::make_unique<uint8_t[]>(program.GetTempArenaSize());
	void* ext_buf[] = { values.data() };
	out = program.Eval(ext_buf, arena.get());
	return true;
}

int main()
{
	float v = 0.f;
	if (!eval_expr("3 + 4", v) || std::fabs(v - 7.f) > 1e-5f)
	{
		std::printf("PARSER_CONSUMER add_fail v=%f\n", v);
		return 1;
	}
	if (!eval_expr("2 * 3 + 4", v) || std::fabs(v - 10.f) > 1e-5f)
	{
		std::printf("PARSER_CONSUMER precedence_fail v=%f\n", v);
		return 1;
	}
	if (!eval_expr("1 ? 2 : 3", v) || std::fabs(v - 2.f) > 1e-5f)
	{
		std::printf("PARSER_CONSUMER cond_fail v=%f\n", v);
		return 1;
	}
	ParseResult bad = Parse("1 +", {}, Program{});
	if (bad.type != ParseResult::SYNTAX_ERROR)
	{
		std::printf("PARSER_CONSUMER invalid_expected_syntax got=%d\n", static_cast<int>(bad.type));
		return 1;
	}
	if (!eval_with_x("x + 10", 123.f, v) || std::fabs(v - 133.f) > 1e-5f)
	{
		std::printf("PARSER_CONSUMER variable_fail v=%f\n", v);
		return 1;
	}
	std::printf("PARSER_CONSUMER add=7 precedence=10 cond=2 invalid=SYNTAX_ERROR var=133\n");
	return 0;
}