#include "serialization/not_null.h"
#include "serialization/ptr.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"

#include <boost/test/unit_test.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <map>
#include <memory>
#include <string>

namespace serialization::tests
{
struct PtrContext
{
  std::map<int, std::unique_ptr<int>> objects;

  int save(int* ptr) const
  {
    for(const auto& [id, obj] : objects)
    {
      if(obj.get() == ptr)
        return id;
    }
    return -1;
  }

  int* load(const int id) const
  {
    if(const auto it = objects.find(id); it != objects.end())
      return it->second.get();
    return nullptr;
  }
};
} // namespace serialization::tests

namespace serialization
{
inline int ptrSave(int* const& ptr, const Serializer<tests::PtrContext>& ser)
{
  return ser.context->save(ptr);
}

inline int* ptrLoad(const TypeId<int*>&, const int id, const Deserializer<tests::PtrContext>& ser)
{
  return ser.context->load(id);
}
} // namespace serialization

namespace serialization::tests
{
BOOST_AUTO_TEST_SUITE(ptr_serialization_tests)

BOOST_AUTO_TEST_CASE(test_ptr_roundtrip)
{
  PtrContext ctx;
  ctx.objects[1] = std::make_unique<int>(42);
  ctx.objects[2] = std::make_unique<int>(100);

  const std::string yaml = "config:\n  ptr: 1";
  YAMLDocument<true> doc{yaml};

  int* result = nullptr;
  struct Cfg
  {
    int*& ref;
    void deserialize(const Deserializer<PtrContext>& ser)
    {
      ser("ptr", ref);
    }
  } cfg{result};

  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result, ctx.objects[1].get());
  BOOST_CHECK_EQUAL(*result, 42);
}

BOOST_AUTO_TEST_CASE(test_not_null_ptr_roundtrip)
{
  PtrContext ctx;
  ctx.objects[1] = std::make_unique<int>(42);

  const std::string yaml = "config:\n  ptr: 1";
  YAMLDocument<true> doc{yaml};

  int dummy = 0;
  gsl_lite::not_null<int*> result = gsl_lite::make_not_null(&dummy);

  struct Cfg
  {
    gsl_lite::not_null<int*>& ref;
    void deserialize(const Deserializer<PtrContext>& ser)
    {
      ser("ptr", ref);
    }
  } cfg{result};

  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.get(), ctx.objects[1].get());
  BOOST_CHECK_EQUAL(*result, 42);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace serialization::tests
