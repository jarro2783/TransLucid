#include <tl/set_lazy_evaluator.hpp>
#include <boost/assign/list_of.hpp>

namespace TransLucid {

namespace SetLazyEvaluator {

TaggedValue AtAbsolute::operator()(const Tuple& context) {
}

TaggedValue AtRelative::operator()(const Tuple& context) {
}

TaggedValue Boolean::operator()(const Tuple& context) {
   //return boost::assign::list_of(ValueContext(
   //   TypedValue(TransLucid::Boolean(m_value), i.typeRegistry().indexBoolean()), Tuple()));
}

TaggedValue BuildTuple::operator()(const Tuple& context) {
}

TaggedValue Constant::operator()(const Tuple& context) {
}

TaggedValue Convert::operator()(const Tuple& context) {
}

TaggedValue Dimension::operator()(const Tuple& context) {
}

TaggedValue Hash::operator()(const Tuple& context) {
}

TaggedValue Ident::operator()(const Tuple& context) {
}

TaggedValue If::operator()(const Tuple& context) {
}

TaggedValue Integer::operator()(const Tuple& context) {
}

TaggedValue IsSpecial::operator()(const Tuple& context) {
}

TaggedValue IsType::operator()(const Tuple& context) {
}

TaggedValue Pair::operator()(const Tuple& context) {
}

TaggedValue UnaryOp::operator()(const Tuple& context) {
}

} //namespace SetLazyEvaluator

} //namespace TransLucid
