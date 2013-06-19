#!/usr/bin/python

# generates yet another tree traversal
# run with the name of the class as an argument and the class declaration and
# definitions are printed to stdout.

import sys

NUMARGS = 2

TreeNodes = [
  "Tree::nil",
  "bool",
  "Special",
  "mpz_class",
  "char32_t",
  "u32string",
  "Tree::LiteralExpr",
  "Tree::DimensionExpr",
  "Tree::IdentExpr",
  "Tree::HashSymbol",
  "Tree::HostOpExpr",
  "Tree::ParenExpr",
  "Tree::UnaryOpExpr",
  "Tree::BinaryOpExpr",
  "Tree::MakeIntenExpr",
  "Tree::EvalIntenExpr",
  "Tree::IfExpr",
  "Tree::HashExpr",
  "Tree::RegionExpr",
  "Tree::TupleExpr",
  "Tree::AtExpr",
  "Tree::LambdaExpr",
  "Tree::PhiExpr",
  "Tree::BaseAbstractionExpr",
  "Tree::BangAppExpr",
  "Tree::LambdaAppExpr",
  "Tree::PhiAppExpr",
  "Tree::WhereExpr",
  "Tree::ConditionalBestfitExpr"
]

if len(sys.argv) < NUMARGS:
  print "Usage:", sys.argv[0], "class [default_result]"
  exit(1)

default_return = ""

if len(sys.argv) > NUMARGS:
  default_return = "  return {};".format(sys.argv[2])

# generate the header

print "class", sys.argv[1]
print "{"
for i in range(0, len(TreeNodes)):
  print "  result_type\n  operator()(const {}&);\n".format(TreeNodes[i])
print "};\n"

# generate the definitions

for i in range(0, len(TreeNodes)):
  print """{0}::result_type\n{0}::operator()(const {1}& e)
{{\n{2}\n}}\n""".format(
    sys.argv[1], TreeNodes[i], default_return)
