#pragma once
#define CURL_STATICLIB
#define BUILDING_LIBCURL

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "curl/curl.h"
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>
#include <Windows.h>

#pragma comment (lib,"libcurl_a_debug.lib")
#pragma comment (lib,"wldap32.lib")
#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"Crypt32.lib")

class ChatBot {
private:
    std::u8string model = u8"llama3";
    std::string prompt = "";
    std::u8string API_URL = u8"http://localhost:11434/api/chat";
    std::u8string readBuffer;
    std::u8string messages;

    static size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up) {
        ChatBot* chatBot = static_cast<ChatBot*>(up);
        for (int c = 0; c < size * nmemb; c++) {
            chatBot->readBuffer.push_back(buf[c]);
        }
        return size * nmemb;
    }

    std::u8string parseJsonResponse(const std::u8string& jsonStr) {
        std::u8string response;
        rapidjson::Document doc;
        doc.Parse(reinterpret_cast<const char*>(jsonStr.c_str()));
        if (!doc.HasParseError() && doc.HasMember("message")) {
            const rapidjson::Value& message = doc["message"];
            if (message.HasMember("content")) {
                response = reinterpret_cast<const char8_t*>(message["content"].GetString());
            }
        }
        return response;
    }

    std::u8string generateMessageJson(const std::u8string& role, const std::u8string& input, std::u8string& messageJson) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartObject();
        writer.Key("role");
        writer.String(reinterpret_cast<const char*>(role.c_str()));
        writer.Key("content");
        writer.String(reinterpret_cast<const char*>(input.c_str()));
        writer.EndObject();

        if (!messageJson.empty()) {
            messageJson += u8",";
        }
        messageJson += reinterpret_cast<const char8_t*>(buffer.GetString());

        return messageJson;
    }

    std::u8string generateRequestJson(const std::u8string& model, const std::u8string& messageJson) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartObject();
        writer.Key("model");
        writer.String(reinterpret_cast<const char*>(model.c_str()));
        writer.Key("messages");
        writer.StartArray();
        writer.RawValue(reinterpret_cast<const char*>(messageJson.c_str()), messageJson.length(), rapidjson::kObjectType);
        writer.EndArray();
        writer.Key("stream");
        writer.Bool(false);
        writer.EndObject();

        return reinterpret_cast<const char8_t*>(buffer.GetString());
    }

    

public:
    ChatBot() {}

    std::u8string convertToUtf8(const std::string& userInput) {
        // 将用户输入转换为wstring
        int wideLength = MultiByteToWideChar(CP_ACP, 0, userInput.c_str(), -1, nullptr, 0);
        std::wstring wideUserInput(wideLength, L'\0');
        MultiByteToWideChar(CP_ACP, 0, userInput.c_str(), -1, &wideUserInput[0], wideLength);

        // 将wstring转换为UTF-8编码的string
        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wideUserInput.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string utf8UserInput(utf8Length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wideUserInput.c_str(), -1, &utf8UserInput[0], utf8Length, nullptr, nullptr);

        // 将UTF-8编码的string转换为u8string
        return std::u8string(reinterpret_cast<const char8_t*>(utf8UserInput.c_str()));
    }

    std::string convertFromUtf8(const std::u8string& utf8Input) {
        // 将UTF-8编码的u8string转换为wstring
        int wideLength = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf8Input.c_str()), -1, nullptr, 0);
        std::wstring wideString(wideLength, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf8Input.c_str()), -1, &wideString[0], wideLength);

        // 将wstring转换为ANSI编码的string
        int ansiLength = WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string ansiString(ansiLength, '\0');
        WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, &ansiString[0], ansiLength, nullptr, nullptr);

        return ansiString;
    }

    void initBot(const std::string& systemMessage) {
        messages = u8"";
        prompt = systemMessage;
        messages = generateMessageJson(u8"system", convertToUtf8(prompt), messages);

        // 手动添加三轮对话记录
        std::string videoInfo1 = R"({
        "bv_id": "BV1A44y1N73e",
        "title": "周轶君：焦虑的反义词是具体",
        "desc": "-",
        "view": 10814,
        "danmaku": 0,
        "reply": 23,
        "favorite": 342,
        "coin": 47,
        "share": 343,
        "like": 305,
        "tags": "摆脱无端的焦虑吧,访谈,焦虑,焦虑症,幸福,情绪,打卡挑战"
    })";
        std::string relevancy1 = "relevancy:0.85;";
        messages = generateMessageJson(u8"user", convertToUtf8(videoInfo1), messages);
        messages = generateMessageJson(u8"assistant", convertToUtf8(relevancy1), messages);

        std::string videoInfo2 = R"({
        "bv_id": "BV1A64y127Ge",
        "title": "体内住着24个人！重现"比利"采访视频，多重人格转换就在几秒之间…",
        "desc": "https://youtu.be/VwpjlimZIZE",
        "view": 621798,
        "danmaku": 621,
        "reply": 327,
        "favorite": 3853,
        "coin": 252,
        "share": 1195,
        "like": 7325,
        "tags": "犯罪,多重人格,比利,人格分裂,心理,24个比利,DID,案件,必剪创作,比利米利根"
    })";
        std::string relevancy2 = "relevancy:0.92;";
        messages = generateMessageJson(u8"user", convertToUtf8(videoInfo2), messages);
        messages = generateMessageJson(u8"assistant", convertToUtf8(relevancy2), messages);

        std::string videoInfo3 = R"({
        "bv_id": "BV1ic411V75x",
        "title": "励志视频短片《不负青春》——班主任自用",
        "desc": "励志视频短片《不负青春》——班主任自用",
        "view": 23084,
        "danmaku": 2,
        "reply": 7,
        "favorite": 347,
        "coin": 6,
        "share": 247,
        "like": 98,
        "tags": "Villanelle,微电影,青春,视频,正能量,梦想,短片,励志,励志视频"
    })";
        std::string relevancy3 = "relevancy:0.12;";
        messages = generateMessageJson(u8"user", convertToUtf8(videoInfo3), messages);
        messages = generateMessageJson(u8"assistant", convertToUtf8(relevancy3), messages);
    }

    std::string chatWithUserInput(const std::string& userInput) {
        CURL* curl = curl_easy_init();
        if (curl) {
            // 设置API请求的URL
            curl_easy_setopt(curl, CURLOPT_URL, reinterpret_cast<const char*>(API_URL.c_str()));

            // 设置请求方法为POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // 创建一个临时的messages变量,用于存储当前对话的消息记录
            std::u8string tempMessages = messages;

            // 生成user message JSON,并追加到tempMessages中
            tempMessages = generateMessageJson(u8"user", convertToUtf8(userInput), tempMessages);

            // 生成完整的请求JSON
            std::u8string requestJson = generateRequestJson(model, tempMessages);

            // 设置请求的数据
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(requestJson.c_str()));

            // 清空readBuffer
            readBuffer.clear();

            // 设置接收响应的回调函数
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

            // 发送请求并接收响应
            CURLcode res = curl_easy_perform(curl);

            // 检查请求是否成功
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                return "";
            }

            // 解析JSON响应,获取assistant的回复
            std::u8string response = parseJsonResponse(readBuffer);

            // 清理curl资源
            curl_easy_cleanup(curl);

            // 将assistant的回复转换为string并返回
            return convertFromUtf8(response);
        }

        // 如果curl初始化失败,返回空字符串
        return "";
    }

    void chatLoop(const std::string& modelStr, const std::string& systemMessageStr) {
        // 将模型字符串和system message字符串转换为u8string
        std::u8string model = convertToUtf8(modelStr);
        std::string systemStr = systemMessageStr;
        std::u8string systemMessage = convertToUtf8(systemStr);

        CURL* curl = curl_easy_init();
        if (curl) {
            // 设置API请求的URL
            curl_easy_setopt(curl, CURLOPT_URL, reinterpret_cast<const char*>(API_URL.c_str()));

            // 设置请求方法为POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // 初始化messageJson为空字符串
            std::u8string messageJson;

            // 生成system message JSON,并追加到messageJson中
            messageJson = generateMessageJson(u8"system", systemMessage, messageJson);

            // 循环进行对话
            while (true) {
                // 获取用户输入
                std::string input;
                std::cout << ">User: ";
                std::getline(std::cin, input);

                // 如果用户输入为quit,则退出循环
                if (input == "quit" || input == "QUIT") {
                    break;
                }

                // 生成user message JSON,并追加到messageJson中
                messageJson = generateMessageJson(u8"user", convertToUtf8(input), messageJson);

                // 生成完整的请求JSON
                std::u8string requestJson = generateRequestJson(model, messageJson);

                // 设置请求的数据
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(requestJson.c_str()));

                // 清空readBuffer
                readBuffer.clear();

                // 设置接收响应的回调函数
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

                // 发送请求并接收响应
                CURLcode res = curl_easy_perform(curl);

                // 检查请求是否成功
                if (res != CURLE_OK) {
                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                    break;
                }

                // 解析JSON响应,获取assistant的回复
                std::u8string response = parseJsonResponse(readBuffer);
                std::cout << "\n>Assistant: " << convertFromUtf8(response) << std::endl;
                std::cout << std::endl;

                // 生成assistant message JSON,并追加到messageJson中
                messageJson = generateMessageJson(u8"assistant", response, messageJson);
            }

            // 清理curl资源
            curl_easy_cleanup(curl);
        }
    }

    std::string getMessages() {
        return convertFromUtf8(messages);
    }
};
