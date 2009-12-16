#include <tl/set_lazy_evaluator.hpp>
#include <boost/assign/list_of.hpp>

namespace TransLucid {

namespace SetLazyEvaluator {

SetResult AtAbsolute::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult AtRelative::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Boolean::evaluate(const TupleSet& context, Interpreter& i) {
   //return boost::assign::list_of(ValueContext(
   //   TypedValue(TransLucid::Boolean(m_value), i.typeRegistry().indexBoolean()), Tuple()));
}

SetResult BuildTuple::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Constant::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Convert::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Dimension::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Hash::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Ident::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult If::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Integer::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult IsSpecial::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult IsType::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult Pair::evaluate(const TupleSet& context, Interpreter& i) {
}

SetResult UnaryOp::evaluate(const TupleSet& context, Interpreter& i) {
}

} //namespace SetLazyEvaluator

} //namespace TransLucid
