#include <gtest/gtest.h>
#include <config.hpp>
#include <json.hpp>
#include <cstdlib>

using namespace granada;

class ConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Config::instance().clear();
    }
};

TEST_F(ConfigTest, JsonProviderStringValue)
{
    auto json = loadJson(R"({"database": "localhost"})");
    auto provider = std::make_shared<JsonConfigProvider>(json);
    Config::instance().addProvider(provider);

    EXPECT_EQ(Config::instance().getString("database"), "localhost");
}

TEST_F(ConfigTest, JsonProviderNestedValue)
{
    auto json = loadJson(R"({"database": {"host": "127.0.0.1", "port": 5432}})");
    auto provider = std::make_shared<JsonConfigProvider>(json);
    Config::instance().addProvider(provider);

    EXPECT_EQ(Config::instance().getString("database.host"), "127.0.0.1");
    EXPECT_EQ(Config::instance().getInt("database.port"), 5432);
}

TEST_F(ConfigTest, JsonProviderArrayValue)
{
    auto json = loadJson(R"({"items": [1, 2, 3]})");
    auto provider = std::make_shared<JsonConfigProvider>(json);
    Config::instance().addProvider(provider);

    auto values = Config::instance().getArray("items");
    EXPECT_EQ(values.size(), 3);
}

TEST_F(ConfigTest, EnvProviderStringValue)
{
    setenv("TEST_APP_NAME", "myapp", 1);

    auto provider = std::make_shared<EnvConfigProvider>("TEST_");
    Config::instance().addProvider(provider);

    EXPECT_EQ(Config::instance().getString("app.name"), "myapp");

    unsetenv("TEST_APP_NAME");
}

TEST_F(ConfigTest, MultipleProvidersOverride)
{
    auto json1 = loadJson(R"({"key": "first"})");
    auto provider1 = std::make_shared<JsonConfigProvider>(json1);

    auto json2 = loadJson(R"({"key": "second"})");
    auto provider2 = std::make_shared<JsonConfigProvider>(json2);

    Config::instance().addProvider(provider1);
    Config::instance().addProvider(provider2);

    // Later provider overrides earlier one
    EXPECT_EQ(Config::instance().getString("key"), "second");
}

TEST_F(ConfigTest, DefaultValueFallback)
{
    auto provider = std::make_shared<JsonConfigProvider>(std::make_shared<Json>());
    Config::instance().addProvider(provider);

    EXPECT_EQ(Config::instance().getString("nonexistent", "default"), "default");
    EXPECT_EQ(Config::instance().getInt("nonexistent", 42), 42);
    EXPECT_DOUBLE_EQ(Config::instance().getDouble("nonexistent", 3.14), 3.14);
    EXPECT_TRUE(Config::instance().getBool("nonexistent", true));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
