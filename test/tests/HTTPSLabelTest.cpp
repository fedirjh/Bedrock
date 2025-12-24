#include <BedrockCommand.h>
#include <libstuff/SHTTPSManager.h>
#include <test/lib/tpunit++.hpp>

struct HTTPSLabelTest : tpunit::TestFixture {
    HTTPSLabelTest()
        : tpunit::TestFixture("HTTPSLabel",
                              TEST(HTTPSLabelTest::serializeDeserializePreservesLabels)) {}

    void serializeDeserializePreservesLabels()
    {
        // Create a command with two HTTPS transactions and give them labels.
        BedrockCommand cmdWithRequests(SQLiteCommand(SData("LabelTestCmd")), nullptr);
        SStandaloneHTTPSManager manager;

        auto* first = new SStandaloneHTTPSManager::Transaction(manager, "req1", "firstLabel");
        first->response = 200;
        first->finished = STimeNow();
        first->fullRequest.methodLine = "GET /first HTTP/1.1";
        first->fullResponse.methodLine = "HTTP/1.1 200 OK";
        first->fullResponse.content = "body1";

        auto* second = new SStandaloneHTTPSManager::Transaction(manager, "req2");
        second->response = 201;
        second->finished = STimeNow();
        second->fullRequest.methodLine = "GET /second HTTP/1.1";
        second->fullResponse.methodLine = "HTTP/1.1 201 Created";
        second->fullResponse.content = "body2";
        second->label = "secondLabel";

        cmdWithRequests.httpsRequests.push_back(first);
        cmdWithRequests.httpsRequests.push_back(second);

        // Serialize and deserialize into a new command and verify labels round-trip.
        const string serialized = cmdWithRequests.serializeHTTPSRequests();

        BedrockCommand deserializedCmd(SQLiteCommand(SData("LabelTestCmdDeser")), nullptr);
        deserializedCmd.deserializeHTTPSRequests(serialized);

        auto it = deserializedCmd.httpsRequests.begin();
        ASSERT_TRUE(it != deserializedCmd.httpsRequests.end());
        ASSERT_EQUAL(string("firstLabel"), (*it)->label);
        ++it;
        ASSERT_TRUE(it != deserializedCmd.httpsRequests.end());
        ASSERT_EQUAL(string("secondLabel"), (*it)->label);
        ++it;
        ASSERT_TRUE(it == deserializedCmd.httpsRequests.end());
    }
} __HTTPSLabelTest;
