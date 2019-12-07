#include "TestJson.h"
#include "MessageHelper.h"
#include "helpers.h"
#include <any>
#include "json/json.h"

void CTestJson::SetUp()
{
}

void CTestJson::TearDown()
{
}

// Test simple map serialization std::any to Json
TEST(CTestJson, TestMapSerialize)
{
    std::map<std::string, std::any> map1, map2;
    map1["state"] = true;
    map1["sensor"] = "motion";

    Json::Value json;
    CMessage::SerializeHeaderMapToJson(map1, json);
    MessageHelper::LoadJsonIntoMap(json, map2);

    ASSERT_TRUE(map1.size() == map2.size()) << "CTestJson, TestMapSerialize failed test.";
}

// Test map with recursive calls serialization std::any to Json
TEST(CTestJson, TestMapSerializeSubmaps)
{
    std::map<std::string, std::any> subMap;
    subMap["state"] = true;
    subMap["sensor"] = "motion";

    std::map<std::string, std::any> map1, map2;
    map1["timestamp"] = 1589283928;
    map1["name"] = "unittest";
    map1["sub_items"] = subMap;

    Json::Value json;
    CMessage::SerializeHeaderMapToJson(map1, json);
    MessageHelper::LoadJsonIntoMap(json, map2);

    ASSERT_TRUE(map1.size() == map2.size()) << "CTestJson, TestMapSerialize failed test.";
}

// Test serialization from map to byte[] buffer
TEST(CTestJson, TestWriteJsonBufferMatches)
{
    std::map<std::string, std::any> values;
    values["state"] = true;
    values["sensor"] = "motion";
    CMessage message1;
    message1.CreateMessageFromJson("unitTest", CMessage::MotionSensor, values);

    std::vector<unsigned char> data;
    message1.SerializeMessageToBuffer(data);
    CMessage message2;
    message2.DeserializeBufferToMessage(data);

    ASSERT_TRUE(message1 == message2) << "CTestJson, TestWriteJsonBuffer failed == operator test.";

    // Add custom header data
    message1.SetHeaderMapValue("is_key_frame", true);
    message1.SerializeMessageToBuffer(data);
    message2.DeserializeBufferToMessage(data);
    ASSERT_TRUE(message1 == message2) << "CTestJson, TestWriteJsonBuffer failed == operator test.";
}

// Test message overloaded operators
TEST(CTestJson, TestOperatorOverload)
{
    std::map<std::string, std::any> values;
    values["state"] = false;
    values["sensor"] = "motion";
    CMessage message1;
    message1.CreateMessageFromJson("unitTest", CMessage::MotionSensor, values);

    // Add custom header data
    message1.SetHeaderMapValue("is_key_frame", true);

    // Add data buffer
    std::vector<unsigned char> buffer = {'a', 'b', 'c'};
    message1.SetData(buffer);

    CMessage message2;
    message2 = message1;
    ASSERT_TRUE(message1 == message2) << "CTestJson, TestOperatorOverload failed == operator test.";

    // Make sure json data size is correct
    std::any size;
    message2.GetHeaderMapValue("data_size", size);
    ASSERT_TRUE(std::any_cast<uint64_t>(size) == 3);

    std::vector<unsigned char> data;
    message2.GetData(data);
    data[1] = 'a';
    message2.SetData(data);

    // Detect that data buffers do not match
    ASSERT_FALSE(message1 == message2) << "CTestJson, TestOperatorOverload failed == operator test.";
}

// Test json config file
TEST(CTestJson, TestJsonConfigFile)
{
    // Load settings map from json config file
    std::string path = Helpers::AppendToRunPath("message_developer.json");
    auto settingsMap = MessageHelper::LoadSettingsFromConfig(path);

    std::map<std::string, std::any> publisher;
    CASTANY(settingsMap["Publisher"], publisher);
    std::vector<std::any> uris;
    CASTANY(publisher["Endpoints"], uris);

    std::string uri;
    CASTANY(uris[0], uri);
    ASSERT_TRUE(uris.size() == 1 && uri == "tcp://*:5563") << "CTestJson, TestJsonConfigFile failed publisher test.";

    std::map<std::string, std::any> subscriber;
    CASTANY(settingsMap["Subscriber"], subscriber);
    CASTANY(subscriber["Endpoints"], uris);
    CASTANY(uris[0], uri);
    ASSERT_TRUE(uris.size() == 1 && uri == "tcp://127.0.0.1:5563") << "CTestJson, TestJsonConfigFile failed publisher test.";
}