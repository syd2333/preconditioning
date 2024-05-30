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
        // ���û�����ת��Ϊwstring
        int wideLength = MultiByteToWideChar(CP_ACP, 0, userInput.c_str(), -1, nullptr, 0);
        std::wstring wideUserInput(wideLength, L'\0');
        MultiByteToWideChar(CP_ACP, 0, userInput.c_str(), -1, &wideUserInput[0], wideLength);

        // ��wstringת��ΪUTF-8�����string
        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wideUserInput.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string utf8UserInput(utf8Length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wideUserInput.c_str(), -1, &utf8UserInput[0], utf8Length, nullptr, nullptr);

        // ��UTF-8�����stringת��Ϊu8string
        return std::u8string(reinterpret_cast<const char8_t*>(utf8UserInput.c_str()));
    }

    std::string convertFromUtf8(const std::u8string& utf8Input) {
        // ��UTF-8�����u8stringת��Ϊwstring
        int wideLength = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf8Input.c_str()), -1, nullptr, 0);
        std::wstring wideString(wideLength, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf8Input.c_str()), -1, &wideString[0], wideLength);

        // ��wstringת��ΪANSI�����string
        int ansiLength = WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string ansiString(ansiLength, '\0');
        WideCharToMultiByte(CP_ACP, 0, wideString.c_str(), -1, &ansiString[0], ansiLength, nullptr, nullptr);

        return ansiString;
    }

    void initBot(const std::string& systemMessage) {
        messages = u8"";
        prompt = systemMessage;
        messages = generateMessageJson(u8"system", convertToUtf8(prompt), messages);

        // �ֶ�������ֶԻ���¼
        std::string videoInfo1 = R"({
        "bv_id": "BV1A44y1N73e",
        "title": "����������ǵķ�����Ǿ���",
        "desc": "-",
        "view": 10814,
        "danmaku": 0,
        "reply": 23,
        "favorite": 342,
        "coin": 47,
        "share": 343,
        "like": 305,
        "tags": "�����޶˵Ľ��ǰ�,��̸,����,����֢,�Ҹ�,����,����ս"
    })";
        std::string relevancy1 = "relevancy:0.85;";
        messages = generateMessageJson(u8"user", convertToUtf8(videoInfo1), messages);
        messages = generateMessageJson(u8"assistant", convertToUtf8(relevancy1), messages);

        std::string videoInfo2 = R"({
        "bv_id": "BV1A64y127Ge",
        "title": "����ס��24���ˣ�����"����"�ɷ���Ƶ�������˸�ת�����ڼ���֮�䡭",
        "desc": "https://youtu.be/VwpjlimZIZE",
        "view": 621798,
        "danmaku": 621,
        "reply": 327,
        "favorite": 3853,
        "coin": 252,
        "share": 1195,
        "like": 7325,
        "tags": "����,�����˸�,����,�˸����,����,24������,DID,����,�ؼ�����,����������"
    })";
        std::string relevancy2 = "relevancy:0.92;";
        messages = generateMessageJson(u8"user", convertToUtf8(videoInfo2), messages);
        messages = generateMessageJson(u8"assistant", convertToUtf8(relevancy2), messages);

        std::string videoInfo3 = R"({
        "bv_id": "BV1ic411V75x",
        "title": "��־��Ƶ��Ƭ�������ഺ����������������",
        "desc": "��־��Ƶ��Ƭ�������ഺ����������������",
        "view": 23084,
        "danmaku": 2,
        "reply": 7,
        "favorite": 347,
        "coin": 6,
        "share": 247,
        "like": 98,
        "tags": "Villanelle,΢��Ӱ,�ഺ,��Ƶ,������,����,��Ƭ,��־,��־��Ƶ"
    })";
        std::string relevancy3 = "relevancy:0.12;";
        messages = generateMessageJson(u8"user", convertToUtf8(videoInfo3), messages);
        messages = generateMessageJson(u8"assistant", convertToUtf8(relevancy3), messages);
    }

    std::string chatWithUserInput(const std::string& userInput) {
        CURL* curl = curl_easy_init();
        if (curl) {
            // ����API�����URL
            curl_easy_setopt(curl, CURLOPT_URL, reinterpret_cast<const char*>(API_URL.c_str()));

            // �������󷽷�ΪPOST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // ����һ����ʱ��messages����,���ڴ洢��ǰ�Ի�����Ϣ��¼
            std::u8string tempMessages = messages;

            // ����user message JSON,��׷�ӵ�tempMessages��
            tempMessages = generateMessageJson(u8"user", convertToUtf8(userInput), tempMessages);

            // ��������������JSON
            std::u8string requestJson = generateRequestJson(model, tempMessages);

            // �������������
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(requestJson.c_str()));

            // ���readBuffer
            readBuffer.clear();

            // ���ý�����Ӧ�Ļص�����
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

            // �������󲢽�����Ӧ
            CURLcode res = curl_easy_perform(curl);

            // ��������Ƿ�ɹ�
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                return "";
            }

            // ����JSON��Ӧ,��ȡassistant�Ļظ�
            std::u8string response = parseJsonResponse(readBuffer);

            // ����curl��Դ
            curl_easy_cleanup(curl);

            // ��assistant�Ļظ�ת��Ϊstring������
            return convertFromUtf8(response);
        }

        // ���curl��ʼ��ʧ��,���ؿ��ַ���
        return "";
    }

    void chatLoop(const std::string& modelStr, const std::string& systemMessageStr) {
        // ��ģ���ַ�����system message�ַ���ת��Ϊu8string
        std::u8string model = convertToUtf8(modelStr);
        std::string systemStr = systemMessageStr;
        std::u8string systemMessage = convertToUtf8(systemStr);

        CURL* curl = curl_easy_init();
        if (curl) {
            // ����API�����URL
            curl_easy_setopt(curl, CURLOPT_URL, reinterpret_cast<const char*>(API_URL.c_str()));

            // �������󷽷�ΪPOST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // ��ʼ��messageJsonΪ���ַ���
            std::u8string messageJson;

            // ����system message JSON,��׷�ӵ�messageJson��
            messageJson = generateMessageJson(u8"system", systemMessage, messageJson);

            // ѭ�����жԻ�
            while (true) {
                // ��ȡ�û�����
                std::string input;
                std::cout << ">User: ";
                std::getline(std::cin, input);

                // ����û�����Ϊquit,���˳�ѭ��
                if (input == "quit" || input == "QUIT") {
                    break;
                }

                // ����user message JSON,��׷�ӵ�messageJson��
                messageJson = generateMessageJson(u8"user", convertToUtf8(input), messageJson);

                // ��������������JSON
                std::u8string requestJson = generateRequestJson(model, messageJson);

                // �������������
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(requestJson.c_str()));

                // ���readBuffer
                readBuffer.clear();

                // ���ý�����Ӧ�Ļص�����
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

                // �������󲢽�����Ӧ
                CURLcode res = curl_easy_perform(curl);

                // ��������Ƿ�ɹ�
                if (res != CURLE_OK) {
                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                    break;
                }

                // ����JSON��Ӧ,��ȡassistant�Ļظ�
                std::u8string response = parseJsonResponse(readBuffer);
                std::cout << "\n>Assistant: " << convertFromUtf8(response) << std::endl;
                std::cout << std::endl;

                // ����assistant message JSON,��׷�ӵ�messageJson��
                messageJson = generateMessageJson(u8"assistant", response, messageJson);
            }

            // ����curl��Դ
            curl_easy_cleanup(curl);
        }
    }

    std::string getMessages() {
        return convertFromUtf8(messages);
    }
};
