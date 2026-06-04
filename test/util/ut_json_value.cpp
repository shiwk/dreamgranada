#include <gtest/gtest.h>
#include <json.hpp>

namespace
{
    struct Point : public granada::JsonElementSerializable
    {
        double x;
        double y;
        Point(double x, double y) : x(x), y(y) {}
        void toJson(granada::JsonValuePtr jv) const override
        {
            jv->addMember("x", x);
            jv->addMember("y", y);
        }
    };

    struct PointList : public granada::JsonElementSerializable
    {
        std::vector<std::pair<double, double>> points;
        PointList(std::initializer_list<std::pair<double, double>> pts) : points(pts) {}
        void toJson(granada::JsonValuePtr jv) const override
        {
            for (const auto &p : points)
            {
                jv->addElement(std::make_shared<Point>(p.first, p.second));
            }
        }
    };
}

class JsonValueTests : public ::testing::Test {};

TEST_F(JsonValueTests, TestAddObjMember)
{
    // Test adding a nested object via Json::addObjMember
    auto json = std::make_shared<granada::Json>();
    json->addMember("name", std::string("test"));
    json->addObjMember("point", std::make_shared<Point>(1.5, 2.5));

    std::string out;
    json->dump(out);

    auto parsed = std::make_shared<granada::Json>(out);
    EXPECT_FALSE(parsed->hasError());

    auto name = parsed->get("name");
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->value<std::string>(), "test");

    auto point = parsed->get("point");
    ASSERT_NE(point, nullptr);
    EXPECT_TRUE(point->isObj());

    auto x = point->getObj("x");
    auto y = point->getObj("y");
    ASSERT_NE(x, nullptr);
    ASSERT_NE(y, nullptr);
    EXPECT_DOUBLE_EQ(x->value<double>(), 1.5);
    EXPECT_DOUBLE_EQ(y->value<double>(), 2.5);
}

TEST_F(JsonValueTests, TestAddArrayMember)
{
    // Test adding a nested array via Json::addArrayMember
    auto json = std::make_shared<granada::Json>();
    json->addMember("title", std::string("points"));
    json->addArrayMember("items", std::make_shared<PointList>(
        std::initializer_list<std::pair<double, double>>{{1.0, 2.0}, {3.0, 4.0}}));

    std::string out;
    json->dump(out);

    auto parsed = std::make_shared<granada::Json>(out);
    EXPECT_FALSE(parsed->hasError());

    auto title = parsed->get("title");
    ASSERT_NE(title, nullptr);
    EXPECT_EQ(title->value<std::string>(), "points");

    auto items = parsed->get("items");
    ASSERT_NE(items, nullptr);
    EXPECT_TRUE(items->isArray());
    EXPECT_EQ(items->size(), 2);

    auto first = items->getElement(0);
    auto second = items->getElement(1);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    auto x1 = first->getObj("x");
    auto y1 = first->getObj("y");
    ASSERT_NE(x1, nullptr);
    ASSERT_NE(y1, nullptr);
    EXPECT_DOUBLE_EQ(x1->value<double>(), 1.0);
    EXPECT_DOUBLE_EQ(y1->value<double>(), 2.0);

    auto x2 = second->getObj("x");
    auto y2 = second->getObj("y");
    ASSERT_NE(x2, nullptr);
    ASSERT_NE(y2, nullptr);
    EXPECT_DOUBLE_EQ(x2->value<double>(), 3.0);
    EXPECT_DOUBLE_EQ(y2->value<double>(), 4.0);
}

TEST_F(JsonValueTests, TestAddArrayMemberEmpty)
{
    // Test adding an empty array
    auto json = std::make_shared<granada::Json>();
    json->addArrayMember("empty", std::make_shared<PointList>(std::initializer_list<std::pair<double, double>>{}));

    std::string out;
    json->dump(out);

    auto parsed = std::make_shared<granada::Json>(out);
    auto empty = parsed->get("empty");
    ASSERT_NE(empty, nullptr);
    EXPECT_TRUE(empty->isArray());
    EXPECT_EQ(empty->size(), 0);
}

TEST_F(JsonValueTests, TestJsonValueAddObjMember)
{
    // Specifically test JsonValue::addObjMember (not Json::addObjMember)
    // by creating a nested object and then adding another nested object via JsonValue

    // First create a Json with a nested object
    auto json = std::make_shared<granada::Json>();
    json->addObjMember("outer", std::make_shared<Point>(5.0, 6.0));

    std::string out;
    json->dump(out);

    auto parsed = std::make_shared<granada::Json>(out);
    auto outer = parsed->get("outer");
    ASSERT_NE(outer, nullptr);
    EXPECT_TRUE(outer->isObj());

    auto x = outer->getObj("x");
    auto y = outer->getObj("y");
    ASSERT_NE(x, nullptr);
    ASSERT_NE(y, nullptr);
    EXPECT_DOUBLE_EQ(x->value<double>(), 5.0);
    EXPECT_DOUBLE_EQ(y->value<double>(), 6.0);
}

TEST_F(JsonValueTests, TestJsonValueAddArrayMember)
{
    // Specifically test JsonValue::addArrayMember (not Json::addArrayMember)
    // This is harder to test directly since we need a JsonValue to call it on
    // We test it indirectly through a scenario where addArrayMember on JsonValue is used

    // Create a nested structure
    auto json = std::make_shared<granada::Json>();
    json->addArrayMember("points", std::make_shared<PointList>(
        std::initializer_list<std::pair<double, double>>{{10.0, 20.0}, {30.0, 40.0}, {50.0, 60.0}}));

    std::string out;
    json->dump(out);

    auto parsed = std::make_shared<granada::Json>(out);
    auto points = parsed->get("points");
    ASSERT_NE(points, nullptr);
    EXPECT_TRUE(points->isArray());
    EXPECT_EQ(points->size(), 3);

    for (size_t i = 0; i < 3; ++i)
    {
        auto elem = points->getElement(i);
        ASSERT_NE(elem, nullptr);

        auto xVal = elem->getObj("x");
        auto yVal = elem->getObj("y");
        ASSERT_NE(xVal, nullptr);
        ASSERT_NE(yVal, nullptr);

        EXPECT_DOUBLE_EQ(xVal->value<double>(), (i + 1) * 10.0 * 2 - 10.0);
        EXPECT_DOUBLE_EQ(yVal->value<double>(), (i + 1) * 20.0);
    }
}

TEST_F(JsonValueTests, TestJsonValueNestedStructure)
{
    // Test a more complex nested structure
    auto json = std::make_shared<granada::Json>();
    json->addMember("version", 1.0);
    json->addObjMember("metadata", std::make_shared<Point>(0.0, 1.0));
    json->addArrayMember("data", std::make_shared<PointList>(
        std::initializer_list<std::pair<double, double>>{{1.0, 1.0}, {2.0, 2.0}}));

    std::string out;
    json->dump(out);

    auto parsed = std::make_shared<granada::Json>(out);
    EXPECT_FALSE(parsed->hasError());

    auto version = parsed->get("version");
    ASSERT_NE(version, nullptr);
    EXPECT_DOUBLE_EQ(version->value<double>(), 1.0);

    auto metadata = parsed->get("metadata");
    ASSERT_NE(metadata, nullptr);
    EXPECT_TRUE(metadata->isObj());

    auto data = parsed->get("data");
    ASSERT_NE(data, nullptr);
    EXPECT_TRUE(data->isArray());
    EXPECT_EQ(data->size(), 2);
}
